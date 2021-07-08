@echo off

ctime\ctime.exe -begin d3d12_renderer.ctm

set CommonCompilerFlags=-std:c++17 -diagnostics:column -WL -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4702 -FC -Z7
set CommonLinkerFlags=-subsystem:windows -incremental:no -opt:ref user32.lib gdi32.lib

IF NOT EXIST build mdkir build
pushd build

REM 32-bit build
REM cl %CommonCompilerFlags% ..\src\main.cpp /link -subsystem:windows %CommonLinkerFlags%

REM 64-bit build
del *.pdb > NUL 2> NUL
REM Optimization switches /O2
cl %CommonCompilerFlags% ..\src\main.cpp /link %CommonLinkerFlags%
popd

ctime\ctime.exe -end d3d12_renderer.ctm %LastError%