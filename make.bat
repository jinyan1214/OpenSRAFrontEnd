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

set PYTHON=C:\PYTHON38-x64
set PYTHONNET_PYDLL=%PYTHON%\python3.8.dll
set QT=C:\Qt\5.15.2\msvc2019_64\bin
set PATH=%PYTHON%;%PYTHON%\Scripts;%QT%;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

echo %PATH%

:: Check if python exists

python -V || exit /b

:: Clone the repos

cd %BATCHPATH%

git clone https://github.com/bzheng10/OpenSRA_dev.git
git clone https://github.com/NHERI-SimCenter/SimCenterCommon.git
git clone https://github.com/NHERI-SimCenter/R2DTool.git
git clone https://github.com/sgavrilovic/OpenSRAFrontEnd
git clone https://github.com/sgavrilovic/QGISPlugin.git


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
C:\Qt\Tools\QtCreator\bin\jom\jom.exe
cd ..


:: Run windeployqt to copy over the qt dlls

%QT%\windeployqt.exe %BATCHPATH%build

:: Copy over the QGIS files

>NUL xcopy /s /y %BATCHPATH%\QGISPlugin\win\DLLs %BATCHPATH%build\
 
:: Copy over the QGIS plugins 

>NUL xcopy /s /y %BATCHPATH%QGISPlugin\mac\Install\share\qgis %BATCHPATH%build\


mkdir %BATCHPATH%build\OpenSRABackEnd

>NUL xcopy /s /y %BATCHPATH%OpenSRA_dev %BATCHPATH%build\OpenSRABackEnd\

ECHO "Done Building OpenSRA for Windows"

ECHO "You can find OpenSRA.exe in "%BATCHPATH%build