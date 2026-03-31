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

The default case handles errors when an invalid condition code is passed.

This looks like it's part of a compiler backend (possibly GCC) that generates assembly code for floating-point comparisons, where these mnemonics correspond to x86 floating-point comparison conditions. The "un" prefix typically indicates "unordered" comparisons that handle NaN values specially.
