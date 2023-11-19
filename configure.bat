@echo off
md build

if exist "build\" (
	cd build 
	cmake .. %*
) else (
	echo Unable to create "build" folder. Stopping configuration.
)
