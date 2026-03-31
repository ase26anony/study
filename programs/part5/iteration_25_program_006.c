Looking at this code snippet, it appears to be part of a switch statement that outputs comparison condition codes to a file. The code is handling different comparison modes and outputting their string representations.

The code is incomplete - it cuts off in the middle of the `default` case. Based on the pattern, I can see it's converting comparison condition codes to their string representations:

- `UNORDERED` → "unord" (unordered comparison)
- `ORDERED` → "ord" (ordered comparison)  
- `UNEQ` → "ueq" (unordered or equal)
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered or less than or equal)
- `UNLT` → "ult" (unordered or less than)
- `LTGT` → "une" (unordered or not equal)

The `default` case appears to be handling an error condition where the operand is not a valid condition code.

This looks like it could be from a compiler code generator (possibly GCC) that's outputting assembly code with comparison condition mnemonics. The naming conventions suggest it's related to floating-point comparisons with special handling for NaN/unordered values.
