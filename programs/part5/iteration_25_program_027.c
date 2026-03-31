Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for outputting assembly code. The code is converting internal condition code representations to their corresponding assembly mnemonics.

Here's what each case appears to be doing:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (unordered or not equal)

The code seems to be handling floating-point comparison conditions, particularly for x86 assembly where these mnemonics are commonly used for floating-point comparisons (like with the `FUCOM` instructions).

The `default` case handles invalid condition codes by calling `output_operand_lossage` with an error message.

This looks like it could be from a compiler backend (like GCC) where it's generating assembly code for floating-point comparisons. The naming conventions (UNORDERED, ORDERED, UNEQ, etc.) suggest these are condition codes for floating-point operations that need to handle NaN (Not-a-Number) values specially.
