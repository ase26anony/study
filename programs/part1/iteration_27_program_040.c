Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations. The code is converting internal condition codes to their string representations for output to a file.

Here's what each case appears to be doing:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (not equal)

This looks like it's part of a compiler code generation routine, likely from GCC or a similar compiler, where it's outputting assembly language mnemonics for floating-point comparison conditions. The "un" prefix typically indicates "unordered" comparisons that handle NaN (Not-a-Number) values specially in floating-point arithmetic.

The default case handles errors when an invalid condition code is passed.
