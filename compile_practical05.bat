@echo off
echo Compiling practical05.c...
gcc practical05.c -o practical05.exe
if %errorlevel% equ 0 (
    echo Compilation successful!
    echo Running practical05.exe...
    echo.
    practical05.exe
) else (
    echo Compilation failed!
    pause
)