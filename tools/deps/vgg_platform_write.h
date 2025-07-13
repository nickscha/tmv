/* vgg_platform_write.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) utility to write a file using OS-specific APIs.

Supports:
 - Windows (Win32 API)
 - Linux / macOS (POSIX)
 - BSDs (FreeBSD, NetBSD, OpenBSD, Haiku)

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef VGG_PLATFORM_WRITE_H
#define VGG_PLATFORM_WRITE_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define VGG_PLATFORM_INLINE inline
#define VGG_PLATFORM_API extern
#elif defined(__GNUC__) || defined(__clang__)
#define VGG_PLATFORM_INLINE __inline__
#define VGG_PLATFORM_API static
#elif defined(_MSC_VER)
#define VGG_PLATFORM_INLINE __inline
#define VGG_PLATFORM_API static
#else
#define VGG_PLATFORM_INLINE
#define VGG_PLATFORM_API static
#endif

#ifdef _WIN32
#define VGG_WIN32_GENERIC_WRITE (0x40000000L)
#define VGG_WIN32_CREATE_ALWAYS 2
#define VGG_WIN32_FILE_ATTRIBUTE_NORMAL 0x00000080

#ifndef _WINDOWS_
#define VGG_WIN32_API(r) __declspec(dllimport) r __stdcall

VGG_WIN32_API(int)
CloseHandle(void *hObject);

VGG_WIN32_API(void *)
CreateFileA(
    const char *lpFileName,
    unsigned long dwDesiredAccess,
    unsigned long dwShareMode,
    void *,
    unsigned long dwCreationDisposition,
    unsigned long dwFlagsAndAttributes,
    void *hTemplateFile);

VGG_WIN32_API(int)
WriteFile(
    void *hFile,
    const void *lpBuffer,
    unsigned long nNumberOfBytesToWrite,
    unsigned long *lpNumberOfBytesWritten,
    void *lpOverlapped);

#endif /* _WINDOWS_   */

VGG_PLATFORM_API VGG_PLATFORM_INLINE int vgg_platform_write(char *filename, unsigned char *buffer, unsigned long size)
{
    void *hFile;
    unsigned long bytes_written;
    int success;

    hFile = CreateFileA(filename, VGG_WIN32_GENERIC_WRITE, 0, 0, VGG_WIN32_CREATE_ALWAYS, VGG_WIN32_FILE_ATTRIBUTE_NORMAL, 0);
    success = WriteFile(hFile, buffer, size, &bytes_written, 0);
    success = CloseHandle(hFile);

    return (success && (bytes_written == size));
}

#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__HAIKU__)

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

VGG_PLATFORM_API VGG_PLATFORM_INLINE int vgg_platform_write(char *filename, unsigned char *buffer, unsigned long size)
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

#else
#error "vgg_platform_write: unsupported operating system. please provide your own write binary file implementation"
#endif

#endif /* VGG_PLATFORM_WRITE_H */

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
