/* tmv_platform_io.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) utility to read/write a file using OS-specific APIs.

Supports:
 - Windows (Win32 API)
 - Linux / macOS (POSIX)
 - BSDs (FreeBSD, NetBSD, OpenBSD, Haiku)

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef TMV_PLATFORM_IO_H
#define TMV_PLATFORM_IO_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define TMV_PLATFORM_INLINE inline
#define TMV_PLATFORM_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define TMV_PLATFORM_INLINE __inline__
#define TMV_PLATFORM_API static
#elif defined(_MSC_VER)
#define TMV_PLATFORM_INLINE __inline
#define TMV_PLATFORM_API static
#else
#define TMV_PLATFORM_INLINE
#define TMV_PLATFORM_API static
#endif

#ifdef _WIN32
#define TMV_PLATFORM_WIN32_INVALID_HANDLE ((void *)-1)
#define TMV_PLATFORM_WIN32_GENERIC_WRITE (0x40000000L)
#define TMV_PLATFORM_WIN32_CREATE_ALWAYS 2
#define TMV_PLATFORM_WIN32_FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define TMV_PLATFORM_WIN32_FILE_ATTRIBUTE_NORMAL 0x00000080

/* IO read */
#define TMV_PLATFORM_WIN32_INVALID_FILE_SIZE ((unsigned long)0xffffffff)
#define TMV_PLATFORM_WIN32_GENERIC_READ (0x80000000L)
#define TMV_PLATFORM_WIN32_FILE_SHARE_READ 0x00000001
#define TMV_PLATFORM_WIN32_OPEN_EXISTING 3

/* IO Find file */
#define TMV_PLATFORM_WIN32_MAX_PATH 260

typedef struct TMV_PLATFORM_WIN32_FILETIME
{
    unsigned long dwLowDateTime;
    unsigned long dwHighDateTime;

} TMV_PLATFORM_WIN32_FILETIME;

typedef struct TMV_PLATFORM_WIN32_FIND_DATAA
{
    unsigned long dwFileAttributes;
    TMV_PLATFORM_WIN32_FILETIME ftCreationTime;
    TMV_PLATFORM_WIN32_FILETIME ftLastAccessTime;
    TMV_PLATFORM_WIN32_FILETIME ftLastWriteTime;
    unsigned long nFileSizeHigh;
    unsigned long nFileSizeLow;
    unsigned long dwReserved0;
    unsigned long dwReserved1;
    char cFileName[TMV_PLATFORM_WIN32_MAX_PATH];
    char cAlternateFileName[14];

} TMV_PLATFORM_WIN32_FIND_DATAA;

#ifndef _WINDOWS_
#define TMV_PLATFORM_WIN32_API(r) __declspec(dllimport) r __stdcall

TMV_PLATFORM_WIN32_API(int)
CloseHandle(void *hObject);

TMV_PLATFORM_WIN32_API(void *)
CreateFileA(
    const char *lpFileName,
    unsigned long dwDesiredAccess,
    unsigned long dwShareMode,
    void *,
    unsigned long dwCreationDisposition,
    unsigned long dwFlagsAndAttributes,
    void *hTemplateFile);

TMV_PLATFORM_WIN32_API(int)
WriteFile(
    void *hFile,
    const void *lpBuffer,
    unsigned long nNumberOfBytesToWrite,
    unsigned long *lpNumberOfBytesWritten,
    void *lpOverlapped);

/* IO read */
TMV_PLATFORM_WIN32_API(unsigned long)
GetFileSize(
    void *hFile,
    unsigned long *lpFileSizeHigh);

TMV_PLATFORM_WIN32_API(int)
ReadFile(
    void *hFile,
    void *lpBuffer,
    unsigned long nNumberOfBytesToRead,
    unsigned long *lpNumberOfBytesRead,
    void *lpOverlapped);

/* IO Find file */
TMV_PLATFORM_WIN32_API(void *)
FindFirstFileA(const char *lpFileName, TMV_PLATFORM_WIN32_FIND_DATAA *lpFindFileData);

TMV_PLATFORM_WIN32_API(int)
FindNextFileA(void *hFindFile, TMV_PLATFORM_WIN32_FIND_DATAA *lpFindFileData);

TMV_PLATFORM_WIN32_API(int)
FindClose(void *hFindFile);

#endif /* _WINDOWS_ */

TMV_PLATFORM_API TMV_PLATFORM_INLINE int tmv_platform_write(char *filename, unsigned char *buffer, unsigned long size)
{
    void *hFile;
    unsigned long bytes_written;
    int success;

    hFile = CreateFileA(filename, TMV_PLATFORM_WIN32_GENERIC_WRITE, 0, 0, TMV_PLATFORM_WIN32_CREATE_ALWAYS, TMV_PLATFORM_WIN32_FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == TMV_PLATFORM_WIN32_INVALID_HANDLE)
    {
        return 0;
    }

    success = WriteFile(hFile, buffer, size, &bytes_written, 0);
    success = CloseHandle(hFile);

    return (success && (bytes_written == size));
}

TMV_PLATFORM_API TMV_PLATFORM_INLINE int tmv_platform_read(char *filename, unsigned char *file_buffer, unsigned long file_buffer_capacity, unsigned long *file_buffer_size)
{
    void *hFile;
    unsigned long fileSize;
    unsigned long bytesRead;

    hFile = CreateFileA(filename, TMV_PLATFORM_WIN32_GENERIC_READ, TMV_PLATFORM_WIN32_FILE_SHARE_READ, 0, TMV_PLATFORM_WIN32_OPEN_EXISTING, TMV_PLATFORM_WIN32_FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == TMV_PLATFORM_WIN32_INVALID_HANDLE)
    {
        return 0;
    }

    fileSize = GetFileSize(hFile, 0);

    if (fileSize == TMV_PLATFORM_WIN32_INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        return 0;
    }

    /* +1 for null terminator */
    if (file_buffer_capacity < fileSize + 1)
    {
        CloseHandle(hFile);
        return 0;
    }

    if (!ReadFile(hFile, file_buffer, fileSize, &bytesRead, 0) || bytesRead != fileSize)
    {
        CloseHandle(hFile);
        return 0;
    }

    file_buffer[fileSize] = '\0';
    *file_buffer_size = fileSize;

    CloseHandle(hFile);

    return 1;
}

#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__HAIKU__)

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

TMV_PLATFORM_API TMV_PLATFORM_INLINE int tmv_platform_write(char *filename, unsigned char *buffer, unsigned long size)
{
    int fd;
    ssize_t written;

    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        return 0;
    }

    written = write(fd, buffer, size);
    close(fd);

    return (written == (ssize_t)size);
}

TMV_PLATFORM_API TMV_PLATFORM_INLINE int tmv_platform_read(char *filename, unsigned char *file_buffer, unsigned long file_buffer_capacity, unsigned long *file_buffer_size)
{
    int fd;
    struct stat st;
    ssize_t bytes_read;

    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        return 0;
    }

    if (fstat(fd, &st) != 0)
    {
        close(fd);
        return 0;
    }

    if ((unsigned long)st.st_size + 1 > file_buffer_capacity)
    {
        close(fd);
        return 0;
    }

    bytes_read = read(fd, file_buffer, st.st_size);
    if (bytes_read != st.st_size)
    {
        close(fd);
        return 0;
    }

    file_buffer[st.st_size] = '\0'; /* Optional: null-terminate */
    *file_buffer_size = st.st_size;

    close(fd);
    return 1;
}

#else
#error "tmv_platform_io: unsupported operating system. please provide your own write binary file implementation"
#endif

#endif /* TMV_PLATFORM_IO_H */

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
