Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations, likely from a compiler code generator. The code is converting internal comparison codes to their string representations for assembly output.

Here's what each case appears to be doing:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (unordered or not equal)

The code is likely from GCC or a similar compiler, handling floating-point comparison conditions for x86 or another architecture's assembly output. These mnemonics correspond to floating-point comparison conditions that check for NaN (Not-a-Number) cases and ordered/unordered comparisons.

The `default` case handles invalid condition codes by calling `output_operand_lossage` with an error message.
