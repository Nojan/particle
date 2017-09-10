pushd %~dp0
@rem do not call emsdk_env.bat if it has already been done
@if "%EM_CONFIG%"=="" call "%EMSCRIPTEN%\..\..\emsdk_env.bat"
python build_emscripten.py
