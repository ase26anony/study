## Why This Forces DWARF Generation:

1. **Type Usage**: The `encrypted_string` type is used in a struct that has external linkage
2. **Function Parameter**: The struct is passed to `process_msg()`, so the compiler needs to describe:
   - How the struct is laid out in memory
   - How to access its fields
   - The types of those fields

## Practical Use Case:

This pattern could be useful for:
- **Debugging encrypted data**: The DWARF info would help debuggers understand the structure even though the actual data is encrypted
- **Type documentation**: The typedef name `encrypted_string` provides semantic meaning in debug info
- **ABI compatibility**: Ensures consistent layout information is available for debugging tools

## Compilation Note:

To compile this with GCC, you might need:
