#!/bin/bash 

# Written by Stevan Gavrilovic, University of California, Berkeley
# Usage: 
# 	1) cd the build folder containing the .app file of your application
# 	2) Copy this script file over to the build folder
#   3) Set the path to macdeployqt on your system and change
#   4) Change the AppName on line 16 to match the *.app file of your application
#   5) Change the path on line 98
# 	6) Run: bash PackageApp.sh

# ********* THINGS TO CHANGE START *********
# Set the path to your macdeployqt here
pathMacDepQt="/Users/steve/Qt/5.15.2/clang_64/bin/macdeployqt"

# Name of the application
AppName="OpenSRA"
# ********* THINGS TO CHANGE END *********

appFile=$AppName".app"
appdmg=$AppName".dmg"

# Check to see if the required macdeployqt file exists at the given location
if [ ! -f "$pathMacDepQt" ]; then   
	
	echo "Could not find macdeployqt at $pathMacDepQt. Exiting"
	exit
fi


# Get the path to the app file
pathApp=`pwd`/$appFile

# Check to see if the RDT app file exists in the current directory
if ! [ -x "$(command -v open $pathApp)" ]; then
	echo "$appFile does not exist. Exiting."
	exit 
fi

# Variable to the path to the dmg
pathdmg="`pwd`/$appdmg"

# Remove the old dmg file if it already exists
if [ -f "$pathdmg" ]; then   
	
	rm $pathdmg	
fi

	
# Define the paths to the application and to libEsriCommonQt.dylib - this should not change
pathAppBin=$pathApp/Contents/MacOS/OpenSRA

pathAppLib=$pathApp/Contents/Frameworks/libEsriCommonQt.dylib

# Get the paths that are in the libraries - these paths will be changed to relative paths instead of the absolute paths
pathEsriCommonQt=$(otool -L $pathAppBin | grep libEsriCommonQt.dylib | awk '{print $1}')
pathLibruntimecore=$(otool -L $pathAppLib | grep libruntimecore.dylib | awk '{print $1}')

# echo $pathEsriCommonQt
# echo $pathLibruntimecore

# Use install name tool to change these to relative paths
install_name_tool -change $pathEsriCommonQt @rpath/libEsriCommonQt.dylib $pathAppBin
install_name_tool -change $pathLibruntimecore @rpath/libruntimecore.dylib $pathAppLib

# Check to make sure it worked
pathEsriCommonQt=$(otool -L $pathAppBin | grep libEsriCommonQt.dylib | awk '{print $1}')
pathLibruntimecore=$(otool -L $pathAppLib | grep libruntimecore.dylib | awk '{print $1}')

if [ "$pathEsriCommonQt" != "@rpath/libEsriCommonQt.dylib" ]; then
    echo "Failed to change the path $pathEsriCommonQt"
	exit
fi

if [ "$pathLibruntimecore" != "@rpath/libruntimecore.dylib" ]; then
    echo "Failed to change the path $pathLibruntimecore"
	exit
fi

echo "Path substitution complete!"

# Run the macdeployqt to create the dmg
$pathMacDepQt $pathApp -dmg

# Check to see if the dmg was created and exists
if [ ! -f "$pathdmg" ]; then
    
    echo "Could not find the .dmg file at $appdmg. Exiting"
    
    exit
fi


echo "Done!"
