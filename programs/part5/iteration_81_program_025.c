This program is designed to trigger the specific uncovered lines in `double-int.cc` by:

1. **Wide Integer Comparisons**: Uses `__int128` and `unsigned __int128` types with explicit comparison operations in sorting, searching, and conditional logic.

2. **Mixed Signed/Unsigned Comparisons**: Includes `compare_mixed()` function and comparisons between signed and unsigned values, including in the unreachable code path.

3. **Constant Wide Integers**: Defines large constants like `LARGE_POS`, `LARGE_NEG`, and `LARGE_UNSIGNED` that require both high and low parts.

4. **Arithmetic Result Comparisons**: The `arithmetic_checks()` function performs arithmetic operations and compares results, including range checks and overflow detection.

5. **Control Flow Based on Comparisons**: Uses comparison results in `if-else` chains, loop conditions, and sorting algorithms.

The execution flow initializes an array with mixed values, performs sorting and searching (which involve many comparisons), and outputs a checksum to prevent optimization removal. The unreachable `dead_code_path()` ensures comparison logic is generated even if not executed.

**Compilation recommendations:**
