@echo OFF
ninja.exe
if %errorlevel%==0 echo d | xcopy /y "D:\Projects\NSMBW\NSMBW-Codebase\Codebase\bin" "%DOLPHIN_USER_DIR%\Load\Riivolution\Codebase\Code"