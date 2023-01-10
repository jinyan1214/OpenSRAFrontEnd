@ECHO OFF

:: How to use this file:
:: 1. Create a folder where you want to build OpenSRA
:: 2. Copy this batch file into the folder. 
:: 3. Check the paths below to ensure they match the locations of Qt, Python, MSVC, etc., on your system
:: 4. Run the batch file. The OpenSRA executable will be in the build folder. 

ECHO "Starting Build of OpenSRA for Windows"

SET BATCHPATH=%~dp0

ECHO %BATCHPATH%

:: Set the system variables below

::set PYTHON=C:\PYTHON38-x64
::set PYTHONNET_PYDLL=%PYTHON%\python3.8.dll
set PYTHON=C:\Users\barry\miniconda3\envs\opensra_clean
set PYTHONNET_PYDLL=%PYTHON%\python310.dll
set QT=C:\Qt\5.15.2\msvc2019_64\bin
set PATH=%PYTHON%;%PYTHON%\Scripts;%QT%;%PATH%
:: call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

echo %PATH%

:: Check if python exists

python -V || exit /b

:: Clone the repos

cd %BATCHPATH%

git clone https://github.com/bzheng10/OpenSRA_dev.git
git clone https://github.com/bzheng10/SimCenterCommon.git
git clone https://github.com/bzheng10/R2DTool.git
git clone https://github.com/NHERI-SimCenter/OpenSRAFrontEnd.git
git clone https://github.com/bzheng10/QGISPlugin.git
:: git clone https://github.com/NHERI-SimCenter/SimCenterCommon.git
:: git clone https://github.com/NHERI-SimCenter/R2DTool.git
:: git clone https://github.com/sgavrilovic/OpenSRAFrontEnd
:: git clone https://github.com/sgavrilovic/QGISPlugin.git


cd QGISPlugin
git checkout QGISUpdate
git pull
cd ..

cd R2DTool
git pull
cd ..

cd SimCenterCommon
git pull
cd ..


cd OpenSRAFrontEnd
git pull
cd ..


cd OpenSRA_dev
git pull
cd ..


dir


:: Build OpenSRA

mkdir build
cd build
qmake ..\OpenSRAFrontEnd\OpenSRA.pro
:: set CL=/MP
:: nmake
C:\Qt\Tools\QtCreator\bin\jom\jom.exe -f Makefile.Release
cd ..


:: Make release folder and copy over only required files
mkdir Release

:: move OpenSRA.exe from build to release

>NUL xcopy /s /y %BATCHPATH%build\Release\OpenSRA.exe %BATCHPATH%Release\

ECHO "Copied OpenSRA.exe to %BATCHPATH%Release\"


:: Run windeployqt to copy over the qt dlls into Release

%QT%\windeployqt.exe %BATCHPATH%Release\
%QT%\windeployqt.exe %BATCHPATH%Release\OpenSRA.exe

ECHO "Ran windeployqt"

:: Manually add missing Qt dlls that windeployqt does not copy over for some reason

>NUL xcopy /s /y %QT%\Qt5WinExtras.dll %BATCHPATH%Release\
>NUL xcopy /s /y %QT%\Qt5OpenGL.dll %BATCHPATH%Release\
>NUL xcopy /s /y %QT%\Qt5QuickWidgets.dll %BATCHPATH%Release\
>NUL xcopy /s /y %QT%\Qt5Qml.dll %BATCHPATH%Release\
>NUL xcopy /s /y %QT%\Qt5SerialPort.dll %BATCHPATH%Release\
>NUL xcopy /s /y %QT%\Qt5Positioning.dll %BATCHPATH%Release\
>NUL xcopy /s /y %QT%\Qt5Quick.dll %BATCHPATH%Release\
>NUL xcopy /s /y %QT%\Qt5QmlModels.dll %BATCHPATH%Release\


:: Use InstallReleaseLibs to copy over the QGIS files
>NUL xcopy /s /y %BATCHPATH%\QGISPlugin\InstallReleaseLibs.cmd %BATCHPATH%Release\
cmd /c "%BATCHPATH%Release\InstallReleaseLibs.cmd"
 
ECHO "Ran InstallReleaseLibs"

 
:: Copy over the QGIS plugins 

:: >NUL xcopy /s /y %BATCHPATH%QGISPlugin\mac\Install\share\qgis %BATCHPATH%Release\


:: Copy over the OpenSRA backend

mkdir %BATCHPATH%Release\OpenSRA

>NUL xcopy /s /y %BATCHPATH%OpenSRA_dev %BATCHPATH%Release\OpenSRA\

ECHO "Copied over OpenSRA backend"


:: Copy over the library datasets to backend
::mkdir %BATCHPATH%Release\OpenSRA\lib\Datasets
::
::set opensraDataDir="C:\Users\barry\OneDrive - SlateGeotech\CEC\OpenSRA\lib\Datasets\"
::echo %opensraDataDir%
::
::>NUL xcopy /s /y "%opensraDataDir%\" %BATCHPATH%Release\OpenSRA\lib\Datasets\ /EXCLUDE:"%opensraDataDir%superseded"
::
::ECHO "Copied over OpenSRA backend datasets"


:: copy over Python environment

mkdir %BATCHPATH%Release\python_env

>NUL xcopy /s /y %PYTHON% %BATCHPATH%Release\python_env\

ECHO "Copied over Python environment"


::
:: need to add command to download datasets from onedrive link, extract, and copy into Release\OpenSRA\lib\Datasets

::


ECHO "Done Building OpenSRA Executable for Windows"

ECHO "You can find OpenSRA.exe in "%BATCHPATH%build