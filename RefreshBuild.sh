#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Navigate to build directory
cd "$SCRIPT_DIR/build" || { echo "Build directory not found!"; exit 1; }

# Run make
make -j4 || { echo "Make failed!"; exit 1; }

# Navigate back to parent directory
cd "$SCRIPT_DIR" || { echo "Parent directory not found!"; exit 1; }

# Launch the GUI
./bin/EventBuilderGui &
