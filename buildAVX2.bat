@echo off

set CommonCompilerFlags= -arch:AVX2 -O2 -MTd -nologo -fp:fast -GR- -EHa- -Zi -Oi -WX -W4
set CommonCompilerFlags= -D_CRT_SECURE_NO_WARNINGS -DRAY_WIN32=1 %CommonCompilerFlags%
set CommonLinkerFlags=-incremental -debug user32.lib gdi32.lib winmm.lib opengl32.lib

IF NOT EXIST build mkdir build
pushd build

cl.exe %CommonCompilerFlags% ../ray.cpp /link %CommonLinkerFlags%
set LastError=%ERRORLEVEL%
popd

echo %ERRORLEVEL%
IF NOT %ERRORLEVEL%==0 GOTO :end

pushd data
..\build\ray.exe
start test.bmp
popd


