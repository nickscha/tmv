@echo off
REM Compiles the program without the C standard library


REM -ftime-report    /* To see compile/link time statistic */

set DEF_COMPILER_FLAGS=-mconsole -march=native -mtune=native -std=c89 -pedantic -nodefaultlibs -nostdlib -mno-stack-arg-probe -Xlinker /STACK:0x100000,0x100000 ^
-fno-builtin -ffreestanding -fno-asynchronous-unwind-tables -fuse-ld=lld ^
-Wall -Wextra -Werror -Wvla -Wconversion -Wdouble-promotion -Wmissing-field-initializers -Wno-uninitialized -Winit-self -Wunused -Wunused-macros -Wunused-local-typedefs

set DEF_FLAGS_LINKER=-lkernel32

set SOURCE_NAME=tmv_win32_nostdlib

cc -s -O2 %DEF_COMPILER_FLAGS% %SOURCE_NAME%.c -o %SOURCE_NAME%.exe %DEF_FLAGS_LINKER%
%SOURCE_NAME%.exe
