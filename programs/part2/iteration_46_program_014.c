This code demonstrates a technique to force DWARF debug information generation for a custom bit-precise integer type. Here's what's happening:

## Key Points:

1. **`_BitInt(128)`**: A C23 feature for arbitrary-width integers. This creates a 128-bit integer type.

2. **`typedef _BitInt(128) encrypted_string`**: Creates an alias `encrypted_string` for a 128-bit integer. The name suggests it's used to store encrypted strings, though it's technically just a large integer.

3. **The Trick**: By placing `encrypted_string` inside a `struct` that's used in a function parameter, you ensure the compiler generates DWARF debug information for this type. Without this, the type might be optimized away or not included in debug info.

## Why This Matters for Debugging:

When debugging (e.g., with GDB), you need DWARF information to:
- Know the type exists
- Understand its size (128 bits)
- Properly display variables of this type

## Compilation Example:

```bash
# With debug info
gcc -g -std=c23 -o test test.c

# Check DWARF info
readelf -wi test | grep -A5 -B5 encrypted_string
```

## Practical Use Case:

This pattern is useful when:
- Working with custom hardware that has unusual integer sizes
- Implementing cryptography that needs specific bit widths
- Ensuring debuggers can properly interpret custom types

The `(void)m;` in `process_msg` simply suppresses the "unused parameter" warning.
