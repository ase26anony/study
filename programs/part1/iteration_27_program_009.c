Looking at this code snippet, it appears to be part of a switch statement that converts condition code constants to their string representations for output. The code seems to be handling floating-point comparison conditions, likely from a compiler backend.

The pattern shows:
- `UNORDERED` → "unord" (unordered comparison)
- `ORDERED` → "ord" (ordered comparison)  
- `UNEQ` → "ueq" (unordered or equal)
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered or less than or equal)
- `UNLT` → "ult" (unordered or less than)
- `LTGT` → "une" (unordered or not equal)

This looks like it's from GCC or a similar compiler, handling the output of floating-point comparison conditions in assembly code generation. The `file` parameter is likely a FILE* stream where the assembly code is being written.

The code is incomplete as it shows a `default` case that calls `output_operand_lossage` but doesn't show the closing brace of the switch statement or the function context.
