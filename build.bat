@echo off
set FASM=c:\Programs\fasm\FASM.EXE
rem /analyze- /GS- /O1 /nologo /W3 /Zc:wchar_t- /Gm- /O1 /nologo /Zp1
set CL_OPTS=/Zl /GS- /Zp1 /nologo
if not exist %~dp0\build\obj mkdir %~dp0\build\obj
PUSHD %~dp0\build\obj
del *.obj
%FASM% %~dp0\entry.asm ..\entry.obj && ^
%FASM% %~dp0\src\loader.asm loader.bin && ^
%FASM% %~dp0\src\ISR.asm ISR.obj && ^
cl /I %~dp0\include %CL_OPTS% /c %~dp0\src\*.c && ^
php -f %~dp0\linker.php > %~dp0\build\linker.log && ^
echo "Done!"
POPD