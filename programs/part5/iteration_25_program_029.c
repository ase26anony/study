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

The default case handles errors when an invalid condition code is passed.

This looks like it could be from a compiler code generator (possibly GCC) that's outputting assembly code with floating-point comparison conditions. The naming conventions suggest it's dealing with the special floating-point comparison predicates that handle NaN (Not-a-Number) cases.

The "un" prefix typically means "unordered" (allowing NaN values), while the standard comparisons like "lt", "le", "eq", etc., are the normal ordered comparisons.
