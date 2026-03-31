Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations. The code is converting internal condition codes to their string representations for output to a file.

The code handles various unordered floating-point comparison conditions:
- `UNORDERED` → "unord" (unordered)
- `ORDERED` → "ord" (ordered)
- `UNEQ` → "ueq" (unordered or equal)
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered or less than or equal)
- `UNLT` → "ult" (unordered or less than)
- `LTGT` → "une" (unordered or not equal)

This looks like it could be from a compiler code generator (possibly GCC) that's generating assembly code with floating-point comparison mnemonics. The mnemonics appear to be for x86 architecture's floating-point comparison instructions.

The code is incomplete - it ends with a `default` case that calls `output_operand_lossage` with an error message, but we don't see the closing brace of the switch statement or the function.
