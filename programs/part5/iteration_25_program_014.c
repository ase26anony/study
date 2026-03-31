Looking at this code, it appears to be part of a switch statement that handles different comparison conditions for outputting assembly code. The code is converting internal comparison codes to their corresponding assembly mnemonics.

The code handles these comparison conditions:
- `UNORDERED` → "unord" (unordered comparison)
- `ORDERED` → "ord" (ordered comparison)
- `UNEQ` → "ueq" (unordered or equal)
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered, less than, or equal)
- `UNLT` → "ult" (unordered or less than)
- `LTGT` → "une" (not equal)

This looks like it's from a compiler backend (likely GCC) that's generating floating-point comparison operations. The naming conventions suggest these are for handling NaN (Not-a-Number) cases in floating-point comparisons, where comparisons can be "unordered" when NaN values are involved.

The `default` case handles any unexpected condition code with an error message.
