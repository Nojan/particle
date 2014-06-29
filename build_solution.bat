pushd %~dp0
rmdir /s /q build
rmdir /s /q bin
premake4 --file=premake4.lua --os=windows --platform=x32 vs2010