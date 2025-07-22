@echo off

set DEF_FLAGS_COMPILER=-std=c89 -pedantic -Wall -Wextra -Werror -Wconversion -Wvla -Wdouble-promotion -Wsign-conversion -Wuninitialized -Winit-self -Wunused -Wunused-macros -Wunused-local-typedefs
set DEF_FLAGS_LINKER=
set SOURCE_NAME=tmv_tools

cc -s -O2 %DEF_FLAGS_COMPILER% -o %SOURCE_NAME%.exe %SOURCE_NAME%.c %DEF_FLAGS_LINKER%
%SOURCE_NAME%.exe --cmd=files_to_tmv --input=..                   --output=test.tmv
%SOURCE_NAME%.exe --cmd=tmv_to_svg   --input=test.tmv             --output=test.svg
%SOURCE_NAME%.exe --cmd=tmv_to_svg   --input=tmv_tools_binary.tmv --output=tmv_tools_binary.svg
