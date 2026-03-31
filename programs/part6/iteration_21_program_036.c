This script creates a comprehensive test that:

1. **Sets and resets target system root variables** using `--sysroot` and `-isysroot`
2. **Manipulates dump file variables** with `-dumpdir`, `-dumpbase`, and `-dumpbase-ext`
3. **Tests save-temps flags** with different modes (`obj`, `cwd`, default)
4. **Uses time reporting** with `-ftime-report`
5. **Manipulates specs** with a custom spec file
6. **Triggers warning/error status changes** with `-Werror` and syntax errors
7. **Exercises the full compilation pipeline** (`-E`, `-S`, `-c`, linking)
8. **Tests help/version flags** that set `print_help_list`, `print_version`, etc.
9. **Uses environment variables** to force driver re-initialization
10. **Tests complex combinations** of options to ensure thorough state coverage
11. **Verifies clean state** with a final simple compilation

The script creates temporary test files and cleans up after itself. Each GCC invocation is designed to set specific global state variables that should be reset by the `driver::finalize()` function before the next invocation.

To run this test:
