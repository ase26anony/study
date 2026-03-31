Looking at this code, it appears to be part of a switch statement that converts comparison operation codes to their string representations for output. The code handles various floating-point comparison conditions, particularly those involving unordered comparisons (which occur with NaN values in floating-point arithmetic).

The pattern shows:
- `UNORDERED` → "unord" (unordered comparison)
- `ORDERED` → "ord" (ordered comparison)
- `UNEQ` → "ueq" (unordered equal)
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered less than or equal)
- `UNLT` → "ult" (unordered less than)
- `LTGT` → "une" (unordered not equal)

This looks like it's from a compiler code generator (likely GCC or similar) that's outputting assembly code. The mnemonics correspond to x86 floating-point comparison conditions or similar architecture-specific comparison codes.

The code is incomplete as it shows the `default` case but not the closing brace of the switch statement or the function context. The `output_operand_lossage` function would be called when an invalid condition code is passed.
