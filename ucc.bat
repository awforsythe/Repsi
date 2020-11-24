@echo off

call %~dp0\vars.bat

if [%~1]==[] goto no_commandlet
set COMMANDLET=%~1
for /f "tokens=1,* delims= " %%a in ("%*") do set REMAIN=%%b

set UCC_CMD="%UE4EDITOR_CMD_EXE%" "%UPROJECT_PATH%" -run=%COMMANDLET% %REMAIN%
echo %UCC_CMD%
call %UCC_CMD%
goto done

:no_commandlet
echo "Please specify a commandlet name; e.g. 'ucc cook'"
exit /b 1

:done
