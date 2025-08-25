#!/bin/bash

# Outtatimers Implant - Environment Setup Script
# This script creates a conda environment with all necessary dependencies

set -e

echo "🚀 Setting up Outtatimers Implant development environment..."
echo "=========================================================="

# Check if conda is available
if ! command -v conda &> /dev/null; then
    echo "❌ Conda is not installed or not in PATH"
    echo "Please install Miniconda or Anaconda first:"
    echo "  https://docs.conda.io/en/latest/miniconda.html"
    exit 1
fi

echo "✅ Conda found: $(conda --version)"

# Check if environment already exists
if conda env list | grep -q "outtatimers-implant"; then
    echo "⚠️  Environment 'outtatimers-implant' already exists"
    read -p "Do you want to remove and recreate it? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "🗑️  Removing existing environment..."
        conda env remove -n outtatimers-implant -y
    else
        echo "🔄 Updating existing environment..."
        conda env update -f environment.yml
        echo "✅ Environment updated successfully!"
        exit 0
    fi
fi

# Create new environment
echo "🔧 Creating conda environment from environment.yml..."
conda env create -f environment.yml

if [ $? -eq 0 ]; then
    echo "✅ Environment created successfully!"
else
    echo "❌ Failed to create environment"
    exit 1
fi

# Activate environment
echo "🔌 Activating environment..."
source $(conda info --base)/etc/profile.d/conda.sh
conda activate outtatimers-implant

# Verify key dependencies
echo "🧪 Verifying key dependencies..."
echo ""

# Test protobuf (correct import method)
echo "Testing protobuf import..."
if python -c "from google import protobuf; print('✅ protobuf (google.protobuf): OK')" 2>/dev/null; then
    echo "✅ protobuf (google.protobuf): OK"
elif python -c "import protobuf; print('✅ protobuf (direct): OK')" 2>/dev/null; then
    echo "✅ protobuf (direct): OK"
else
    echo "❌ protobuf: Failed to import"
    echo "Trying to fix protobuf installation..."
    pip install --upgrade protobuf
fi

# Test pynng
echo "Testing pynng import..."
if python -c "import pynng; print('✅ pynng: OK')" 2>/dev/null; then
    echo "✅ pynng: OK"
else
    echo "❌ pynng: Failed to import"
    echo "Trying to fix pynng installation..."
    pip install --upgrade pynng
fi

# Test rich
echo "Testing rich import..."
if python -c "import rich; print('✅ rich: OK')" 2>/dev/null; then
    echo "✅ rich: OK"
else
    echo "❌ rich: Failed to import"
    echo "Trying to fix rich installation..."
    pip install --upgrade rich
fi

# Final verification
echo ""
echo "🎯 Final dependency verification..."
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

echo ""
echo "🎉 Environment setup complete!"
echo "=========================================================="
echo ""
echo "📋 Next steps:"
echo "1. Activate the environment: conda activate outtatimers-implant"
echo "2. Test the schematic generator: python generate_schematic.py"
echo "3. Make sure KiCad is running with API enabled for full IPC functionality"
echo ""
echo "💡 Note: protobuf is imported as 'from google import protobuf'"
echo "   This is the correct way to import it in this environment"
echo ""
echo "🔧 If you encounter issues, run: ./fix_protobuf.sh"
