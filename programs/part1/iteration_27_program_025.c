Looking at this code snippet, it appears to be part of a switch statement that converts comparison operation codes to their string representations for output. The code handles various floating-point comparison conditions, particularly unordered comparisons.

The pattern suggests this is likely from a compiler code generator (possibly GCC) that outputs assembly code. The comments about "operand is not a condition code" in the default case indicate this function is specifically for handling condition code operands.

The code converts:
- `UNORDERED` → "unord" (unordered comparison)
- `ORDERED` → "ord" (ordered comparison)  
- `UNEQ` → "ueq" (unordered or equal)
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered or less than or equal)
- `UNLT` → "ult" (unordered or less than)
- `LTGT` → "une" (unordered or not equal)

These are typical floating-point comparison mnemonics used in assembly languages like x86. The naming reflects whether NaNs are considered (unordered comparisons handle NaN cases specially).

The `fputs` function writes these strings to the specified `file` (likely an assembly output file), and `output_operand_lossage` appears to be an error reporting function for invalid operands.
