@ECHO OFF

if "%1" EQU "cc" (
    set CFLAGS=-g
    cd build
    cc %CFLAGS% -o _opsgen.exe ..\_opsgen.c
    cd ..
    build\_opsgen
    cd build
    cc %CFLAGS% -o main.exe ..\main.c
    cc %CFLAGS% -o sas.exe ..\sas.c
    cd ..
    set OK=1
)
if "%1" EQU "cl" (
    set CFLAGS=/Zi /EHsc /nologo
    cd build
    cl %CFLAGS% /Fe.\_opsgen.exe ..\_opsgen.c
    cd ..
    build\_opsgen
    cd build
    cl %CFLAGS% /Fe.\main.exe ..\main.c
    cl %CFLAGS% /Fe.\sas.exe ..\sas.c
    cd ..
    set OK=1
)

if not defined OK (
    echo "Usage: %0 cc  # use cc (posix)"
    echo "       %0 cl  # use cl (MSVC)""
    exit 1
)