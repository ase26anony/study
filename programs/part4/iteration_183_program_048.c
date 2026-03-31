Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations, likely in a compiler backend. The code is converting internal condition codes to their string representations for assembly output.

Here's what each case represents:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (not equal)

These are typical floating-point comparison mnemonics used in assembly languages like x86. The "un" prefix often indicates "unordered" comparisons that handle NaN values specially.

The `default` case handles invalid condition codes with an error message. The code writes the strings to a file (likely an assembly output file) using `fputs`.

This is likely from GCC or another compiler's RTL (Register Transfer Language) expansion phase where comparison operations are being lowered to target-specific assembly mnemonics.
