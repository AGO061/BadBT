@echo off
if exist ..\..\fbt (
  echo FBT FOUND!
) else (
  echo WARNING! FBT NOT FOUND!
  echo Please make sure your BadBT folder is placed in ^<firmware^>^/applications_user^/
  echo.
  echo Press any key to close the app...
  pause>NUL
)


set MYDIR=%~dp0
if "%MYDIR:~-1%" == "\" set "MYDIR1=%MYDIR:~0,-1%"

for %%f in ("%MYDIR1%") do set "badbtfolder=%%~nxf"

cd ..
cd ..


echo Press enter if you are sure you have your flipper connected
pause>NUL
echo Hang on... BadBT is currently being built...
echo.
echo No output is displayed here, all the output is in the build.log file

echo >> --NEW BUILD ATTEMPT--
fbt launch_app APPSRC=applications/%badbtfolder%/ >> applications_user/%badbtfolder%/build.log
cd applications_user
cd %badbtfolder%