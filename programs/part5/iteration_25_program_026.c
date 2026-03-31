Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations, likely from a compiler code generator. The code is converting internal comparison codes to their string representations for assembly output.

Here's what each case appears to be doing:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (not equal, ordered)

These are typical floating-point comparison mnemonics used in assembly languages like x86. The "un" prefix often indicates "unordered" comparisons that handle NaN values specially.

The code is writing these strings to a `file` using `fputs()`, and there's a `default` case that calls `output_operand_lossage()` with an error message when an invalid condition code is encountered.

This looks like it could be from GCC or another compiler's backend code that generates assembly output for floating-point comparisons.
