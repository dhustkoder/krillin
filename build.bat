
@echo off

set SDL2_ROOT=D:\sdks\SDL2-2.0.14

set INCLUDE_DIRS=/I./src^
	/I%SDL2_ROOT%/include^
	/I"C:\Program Files\OpenSSL-Win64\include"

set LIBRARY_DIRS=%SDL2_ROOT%\lib\x64

set CFLAGS_DEBUG=^
	/DDEBUG=1^
	/DEBUG:FULL^
	/Od^
	/Zi

set CFLAGS=%INCLUDE_DIRS% %CFLAGS_DEBUG% /DSDL_MAIN_HANDLED

set LINKER_FLAGS=/LIBPATH:%LIBRARY_DIRS%^
	user32.lib^
	Ws2_32.lib^
	sdl2.lib^
	sdl2main.lib

set CSRC=src\cunit.c

	
del krillin.exe
@echo on
clang-cl -fsanitize=address %CFLAGS% %CSRC% /Fekrillin.exe /link %LINKER_FLAGS%
@echo off
del *.obj

