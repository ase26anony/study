Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions and outputs their string representations to a file. The code seems to be converting internal comparison codes to their textual representations.

The code handles several comparison conditions:
- `UNORDERED` → "unord"
- `ORDERED` → "ord"
- `UNEQ` → "ueq"
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered or less than or equal)
- `UNLT` → "ult" (unordered or less than)
- `LTGT` → "une" (unordered or not equal)

This looks like it could be from a compiler code generator (possibly GCC) that's generating assembly code with floating-point comparison conditions. The naming conventions suggest these are floating-point comparison conditions that handle NaN (Not-a-Number) cases, which is why there are "unordered" variants.

The code is incomplete - it ends with `output_operand_lossage` being called for the default case, which suggests there's an error handling mechanism for unrecognized condition codes.
