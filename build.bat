@echo off
REM ============================================================
REM build.bat — Windows Build Script
REM Person 6's Module
REM Usage:
REM   build.bat         → compile the compiler
REM   build.bat run     → compile and run with input.txt
REM   build.bat clean   → remove build artifacts
REM ============================================================

if "%1"=="clean" (
    echo Cleaning build artifacts...
    del /Q *.o compiler.exe 2>nul
    echo Done.
    exit /b 0
)

echo ========================================================
echo   Building Mini Compiler...
echo ========================================================
echo.

g++ -std=c++17 -Wall -Wextra -o compiler.exe main.cpp lexer.cpp symbol_table.cpp parser.cpp semantic.cpp icg.cpp

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo   BUILD FAILED!
    exit /b 1
)

echo   BUILD SUCCESSFUL!
echo.

if "%1"=="run" (
    echo ========================================================
    echo   Running compiler with input.txt...
    echo ========================================================
    echo.
    compiler.exe input.txt
)
