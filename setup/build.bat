@echo off

rem -- set vsvars32
if not defined MSVCDir (
	echo -- setting msdev paths
	call vsvars32.bat
	echo.
)

rem -- note: makensis.exe should be present in path

rem -- build
echo -- building
devenv ..\winguide.sln /rebuild Release
echo.

rem -- package
echo -- packaging
makensis guide-setup.nsi
echo.

echo -- done

