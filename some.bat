@echo off
set CommonCompilerFlags=-Od -MTd -nologo -fp:fast -Gm -GR- -EHa- -Zi -Oi- -WX -W4 -D_CRT_SECURE_NO_WARNINGS -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1
set CommonLinkerFlags=/incremental /debug user32.lib gdi32.lib winmm.lib opengl32.lib

IF NOT EXIST build mkdir build
pushd build

cl %CommonCompilerFlags% ../ray.cpp /link %CommonLinkerFlags%
