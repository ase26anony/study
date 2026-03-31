Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations, likely in a compiler backend. The code is converting internal comparison codes to their string representations for assembly output.

Here's what each case represents:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (unordered or not equal)

These appear to be floating-point comparison conditions that handle NaN (Not-a-Number) cases. The "un" prefix typically indicates "unordered" comparisons that account for NaN values.

The code is writing these string representations to a file (likely an assembly output file) using `fputs()`. The `default` case handles invalid condition codes by calling an error function `output_operand_lossage()`.

This pattern is common in compiler code generation, particularly for architectures with specific floating-point comparison instructions that need to handle NaN values properly.
