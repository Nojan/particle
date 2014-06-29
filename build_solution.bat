pushd %~dp0
rmdir /s /q build
rmdir /s /q bin
premake4 --file=premake.lua --os=windows --platform=x32 vs2010