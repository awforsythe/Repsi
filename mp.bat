@echo off

call %~dp0\vars.bat

set SERVER_CMD="%UE4EDITOR_EXE%" "%UPROJECT_PATH%" -server -log -consolex=0 -consoley=0
echo %SERVER_CMD%
start "" %SERVER_CMD%

timeout /t 3
set CLIENT_CMD="%UE4EDITOR_EXE%" "%UPROJECT_PATH%" 127.0.0.1 -game -log -consolex=0 -consoley=500 -windowed -resx=960 -resy=540 -winx=640 -winy=540
echo %CLIENT_CMD%
start "" %CLIENT_CMD%
