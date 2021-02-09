
@echo off

set SDL2_ROOT=D:\sdks\SDL2-2.0.14
set SDL_MIXER_ROOT=D:\sdks\SDL2_mixer-2.0.4

set INCLUDE_DIRS=/I.\src^
	/I"%SDL2_ROOT%\include"^
	/I"%SDL_MIXER_ROOT%\include"^
	/I"C:\Program Files\OpenSSL-Win64\include"

set LIBRARY_DIRS=/LIBPATH:%SDL2_ROOT%\lib\x64 /LIBPATH:%SDL_MIXER_ROOT%\lib\x64

set CFLAGS_DEBUG=^
	/DDEBUG=1^
	/DEBUG:FULL^
	/Od^
	/Zi
	
set CFLAGS_RELEASE=^
	/DNDEBUG
	/Ox

set CFLAGS=^
	%INCLUDE_DIRS%^
	%CFLAGS_DEBUG%^
	/DSDL_MAIN_HANDLED ^
	/D_CRT_SECURE_NO_WARNINGS ^
	/D_WINSOCK_DEPRECATED_NO_WARNINGS ^
	/wd4028 ^
	/wd4214 ^
	/wd4204 ^
	/wd4244 ^
	/W4

set LINKER_FLAGS=^
	%LIBRARY_DIRS%^
	user32.lib^
	Ws2_32.lib^
	sdl2.lib^
	sdl2main.lib^
	sdl2_mixer.lib

set CSRC=src\cunit.c

set CC_CLANG_CL=clang-cl -fsanitize=address
set CC_CL=cl
set CC=%CC_CLANG_CL%

	
del krillin.exe
@echo on
%CC%  %CFLAGS% %CSRC% /Fekrillin.exe /link %LINKER_FLAGS%
@echo off
del *.obj

