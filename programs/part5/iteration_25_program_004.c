Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for outputting their string representations to a file. The code is likely from a compiler backend or assembly code generator.

The pattern suggests this is converting internal comparison codes to their textual representations for output. Here's what I notice:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (unordered or not equal)

The naming convention seems to be:
- **UN** prefix often means "unordered or"
- **LT** = less than
- **GT** = greater than
- **EQ** = equal
- **GE** = greater than or equal
- **LE** = less than or equal

The default case handles errors when an invalid condition code is passed.

This looks like it could be from GCC or another compiler's RTL (Register Transfer Language) output code, specifically handling floating-point comparison conditions where NaN values can cause unordered results.
