@echo off
if exist "build/" (
	cd build
	
	if [%1] == [] (	
		echo Building with Release Config.
		cmake --build . --config release
	) else (
		echo Building with %1 Config.
		cmake --build . --config %1
	)
	
	echo Libraries builded and should be located in "/build/bin/ " folder.
) else (
	echo "build" folder does not exist. Call "configure.bat" to configure it.
)