@echo off
if exist "build/" (
	cd build
	cmake --build . --config %1
	echo Libraries builded and should be located in "/build/bin/" folder.
) else (
	echo "build" folder does not exist. Call "configure.bat" to configure it.
)