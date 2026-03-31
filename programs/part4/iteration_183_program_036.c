Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations, likely in a compiler code generation context. The code is converting internal comparison codes to their string representations for assembly output.

Here's what each case appears to be doing:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (not equal)

The default case handles errors when an invalid condition code is passed.

This looks like it could be from GCC or another compiler's backend code, specifically the part that generates floating-point comparison instructions for x86 or similar architectures. The mnemonics correspond to x86's floating-point comparison conditions used with the `FCOM`/`FUCOM` instructions and conditional jumps.

The pattern suggests these are "unordered" floating-point comparisons that handle NaN values specially, unlike regular integer comparisons.
