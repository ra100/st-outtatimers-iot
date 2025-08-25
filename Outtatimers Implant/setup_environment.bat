@echo off
REM Outtatimers Implant - Environment Setup Script for Windows
REM This script creates a conda environment with all necessary dependencies

echo 🚀 Setting up Outtatimers Implant development environment...
echo ==========================================================

REM Check if conda is available
where conda >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ Conda is not installed or not in PATH
    echo Please install Miniconda or Anaconda first:
    echo   https://docs.conda.io/en/latest/miniconda.html
    pause
    exit /b 1
)

echo ✅ Conda found
conda --version

REM Check if environment already exists
conda env list | findstr "outtatimers-implant" >nul
if %errorlevel% equ 0 (
    echo ⚠️  Environment 'outtatimers-implant' already exists
    set /p choice="Do you want to remove and recreate it? (y/N): "
    if /i "%choice%"=="y" (
        echo 🗑️  Removing existing environment...
        conda env remove -n outtatimers-implant -y
    ) else (
        echo 🔄 Updating existing environment...
        conda env update -f environment.yml
        echo ✅ Environment updated successfully!
        pause
        exit /b 0
    )
)

REM Create new environment
echo 🔧 Creating conda environment from environment.yml...
conda env create -f environment.yml

if %errorlevel% neq 0 (
    echo ❌ Failed to create environment
    pause
    exit /b 1
)

echo ✅ Environment created successfully!

REM Activate environment
echo 🔌 Activating environment...
call conda activate outtatimers-implant

REM Verify key dependencies
echo 🧪 Verifying key dependencies...
echo.

REM Test protobuf (correct import method)
echo Testing protobuf import...
python -c "from google import protobuf; print('✅ protobuf (google.protobuf): OK')" 2>nul
if %errorlevel% equ 0 (
    echo ✅ protobuf (google.protobuf): OK
) else (
    python -c "import protobuf; print('✅ protobuf (direct): OK')" 2>nul
    if %errorlevel% equ 0 (
        echo ✅ protobuf (direct): OK
    ) else (
        echo ❌ protobuf: Failed to import
        echo Trying to fix protobuf installation...
        pip install --upgrade protobuf
    )
)

REM Test pynng
echo Testing pynng import...
python -c "import pynng; print('✅ pynng: OK')" 2>nul
if %errorlevel% neq 0 (
    echo ❌ pynng: Failed to import
    echo Trying to fix pynng installation...
    pip install --upgrade pynng
)

REM Test rich
echo Testing rich import...
python -c "import rich; print('✅ rich: OK')" 2>nul
if %errorlevel% neq 0 (
    echo ❌ rich: Failed to import
    echo Trying to fix rich installation...
    pip install --upgrade rich
)

REM Final verification
echo.
echo 🎯 Final dependency verification...
python -c "
try:
    from google import protobuf
    print('✅ protobuf (google.protobuf): OK')
except ImportError:
    try:
        import protobuf
        print('✅ protobuf (direct): OK')
    except ImportError:
        print('❌ protobuf: Failed')

try:
    import pynng
    print('✅ pynng: OK')
except ImportError:
    print('❌ pynng: Failed')

try:
    import rich
    print('✅ rich: OK')
except ImportError:
    print('❌ rich: Failed')
"

echo.
echo 🎉 Environment setup complete!
echo ==========================================================
echo.
echo 📋 Next steps:
echo 1. Activate the environment: conda activate outtatimers-implant
echo 2. Test the schematic generator: python generate_schematic.py
echo 3. Make sure KiCad is running with API enabled for full IPC functionality
echo.
echo 💡 Note: protobuf is imported as 'from google import protobuf'
echo    This is the correct way to import it in this environment
echo.
echo 🔧 If you encounter issues, run: fix_protobuf.bat
echo.
pause
