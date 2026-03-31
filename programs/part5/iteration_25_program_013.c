Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations. The code is converting internal condition codes to their string representations for assembly output.

The pattern I notice is:
- `UNORDERED` → "unord" (unordered)
- `ORDERED` → "ord" (ordered)
- `UNEQ` → "ueq" (unordered or equal)
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered or less than or equal)
- `UNLT` → "ult" (unordered or less than)
- `LTGT` → "une" (unordered or not equal)

This looks like it's part of a compiler backend (likely GCC) that's generating assembly code for floating-point comparisons. The conditions appear to be for the x86 architecture's FPU comparison instructions, which have special handling for NaN values.

The missing cases would likely include the standard ordered comparisons:
- `EQ` → "eq" (equal)
- `NE` → "ne" (not equal)
- `GE` → "ge" (greater than or equal)
- `GT` → "gt" (greater than)
- `LE` → "le" (less than or equal)
- `LT` → "lt" (less than)

The code is using `fputs` to write the condition code strings to a file (presumably an assembly output file), and there's a default case that calls `output_operand_lossage` with an error message when an invalid condition code is encountered.
