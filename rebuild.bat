set CFLAGS=/Zi /EHsc /nologo
cd build
cl %CFLAGS% /Fe.\_opsgen.exe ..\_opsgen.c
cd ..
build\_opsgen
cd build
cl %CFLAGS% /Fe.\main.exe ..\main.c
cl %CFLAGS% /Fe.\sas.exe ..\sas.c