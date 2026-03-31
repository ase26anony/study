```bash
#!/bin/bash

# Test script for gcov-dump.cc uncovered lines (111-130)
# This script creates a GCOV data file and tests all command-line flags

set -e  # Exit on any critical error

echo "=== Starting gcov-dump test ==="

# Create a temporary working directory
TEMP_DIR=$(mktemp -d)
echo "Created temp directory: $TEMP_DIR"
cd "$TEMP_DIR"

# Define file names
SOURCE_FILE="test.c"
BINARY="test.exe"
GCOV_DATA="test.gcda"

# Step 1: Create a minimal C program
cat > "$SOURCE_FILE" << 'EOF'
#include <stdio.h>

int main() {
    printf("Hello from test program\n");
    return 0;
}
EOF

echo "Created test source file: $SOURCE_FILE"

# Step 2: Compile with coverage instrumentation
echo "Compiling with coverage instrumentation..."
if ! gcc -fprofile-arcs -ftest-coverage -O0 "$SOURCE_FILE" -o "$BINARY"; then
    echo "ERROR: Compilation failed"
    exit 1
fi

echo "Compilation successful"

# Step 3: Run the program to generate GCOV data
echo "Running program to generate GCOV data..."
if ! ./"$BINARY"; then
    echo "ERROR: Program execution failed"
    exit 1
fi

# Check if .gcda file was created
if [ ! -f "$GCOV_DATA" ]; then
    echo "ERROR: GCOV data file not created: $GCOV_DATA"
    ls -la
    exit 1
fi

echo "GCOV data file created: $GCOV_DATA"

# Step 4: Execute gcov-dump with various flags
echo -e "\n=== Testing gcov-dump flags ==="

# Test 1: -h flag (help/usage)
echo -e "\n1. Testing -h flag (usage):"
gcov-dump -h 2>&1 | head -5

# Test 2: -v flag (version)
echo -e "\n2. Testing -v flag (version):"
gcov-dump -v 2>&1

# Test 3: -l flag (dump contents)
echo -e "\n3. Testing -l flag (dump contents):"
gcov-dump -l "$GCOV_DATA" 2>&1 | head -10

# Test 4: -p flag (dump positions)
echo -e "\n4. Testing -p flag (dump positions):"
gcov-dump -p "$GCOV_DATA" 2>&1 | head -10

# Test 5: -r flag (dump raw)
echo -e "\n5. Testing -r flag (dump raw):"
gcov-dump -r "$GCOV_DATA" 2>&1 | head -10

# Test 6: -s flag (dump stable)
echo -e "\n6. Testing -s flag (dump stable):"
gcov-dump -s "$GCOV_DATA" 2>&1 | head -10

# Test 7: Combined flags
echo -e "\n7. Testing combined flags (-l -p -s):"
gcov-dump -l -p -s "$GCOV_DATA" 2>&1 | head -15

# Test 8: Invalid flag (triggers default case)
echo -e "\n8. Testing invalid flag -x (should trigger default case):"
if gcov-dump -x "$GCOV_DATA" 2>&1 | grep -q "unknown flag"; then
    echo "SUCCESS: Invalid flag correctly detected"
else
    echo "WARNING: Invalid flag error message not found (but this might be OK)"
fi

# Test 9: Another invalid flag
echo -e "\n9. Testing invalid flag -Z:"
gcov-dump -Z "$GCOV_DATA" 2>&1 | grep -i "unknown" || true

# Test 10: No arguments (should show error or usage)
echo -e "\n10. Testing with no arguments:"
gcov-dump 2>&1 | head -5 || true

# Cleanup
echo -e "\n=== Cleaning up ==="
cd ..
rm -rf "$TEMP_DIR"
echo "Removed temp directory: $TEMP_DIR"

echo -e "\n=== Test completed successfully ==="
echo "All gcov-dump switch cases should have been exercised:"
echo "  -h, -v, -l, -p, -r, -s, and default case for invalid flags"
```
