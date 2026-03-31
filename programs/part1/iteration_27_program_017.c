Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations. The code is converting internal condition codes to their string representations for assembly output.

Here's what each case appears to be doing:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (not equal)

The naming seems to follow x86 assembly conventions for floating-point comparisons, where:
- "unord" and "ord" check for NaN conditions
- "ueq", "ult", "ule" are unordered versions of comparisons
- "nlt", "nle" are negated comparisons
- "une" means "not equal" (which is different from regular "ne" because it handles NaN)

This looks like it could be from GCC or another compiler's backend code that generates assembly for floating-point comparison operations. The `default` case handles invalid condition codes with an error message.
