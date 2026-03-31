This script systematically tests various combinations that should trigger the reset logic:

1. **Multiple dump options** (`-save-temps`, `-dumpdir`, `-dumpbase`) are set and changed between files
2. **Help/version options** are mixed with compilation commands
3. **Response files** (`@file` syntax) are used to set `at_file_supplied`
4. **Different processing modes** (`-E`, `-S`, `-c`) are chained in single commands
5. **Language specifications** (`-x`) change between files
6. **Environment variables** that might affect driver state
7. **Multiple conflicting options** that force state resets

The script creates temporary C files, response files, and invokes `gcc` with carefully crafted command lines. Many invocations will fail (hence the `2>/dev/null || true`), but the goal is to execute the driver code paths, not to produce valid output.

To run this test:
