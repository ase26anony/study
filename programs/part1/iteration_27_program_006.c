Looking at this code snippet, it appears to be part of a switch statement that converts condition code constants to their string representations for output. The code handles various floating-point comparison conditions.

The pattern suggests this is likely from a compiler code generator (possibly GCC) that outputs assembly code. Each case represents a different floating-point comparison condition:

- `UNORDERED` → "unord" (unordered comparison)
- `ORDERED` → "ord" (ordered comparison)  
- `UNEQ` → "ueq" (unordered or equal)
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered or less than or equal)
- `UNLT` → "ult" (unordered or less than)
- `LTGT` → "une" (not equal)

The code is missing the closing brace for the switch statement and likely has more cases for other condition codes (like EQ, NE, LT, LE, GT, GE, etc.). The `default` case handles invalid condition codes with an error message.

This appears to be part of a larger function that outputs assembly operands, where condition codes need to be converted to their textual representation for the target architecture's assembly syntax.
