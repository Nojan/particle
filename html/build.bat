pushd %~dp0
@rem do not call emsdk_env.bat if it has already been done
@if "%EM_CONFIG%"=="" call "..\..\emsdk\emsdk_env.bat"

set MAKE="%EMSDK%/mingw/7.1.0_64bit/bin/mingw32-make.exe"

call emcmake cmake -DCMAKE_BUILD_TYPE=Release -D"CMAKE_MAKE_PROGRAM:PATH=%MAKE%" .. -G "MinGW Makefiles"
call emmake %MAKE%
