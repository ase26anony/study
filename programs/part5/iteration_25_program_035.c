This appears to be a code snippet from a compiler or assembler backend that's outputting condition code mnemonics for floating-point comparisons. The code is handling different comparison modes and outputting their corresponding assembly mnemonics.

Here's what each case represents:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (unordered or not equal)

The `default` case handles errors when an invalid condition code is passed.

This looks like it could be from GCC or another compiler's RTL (Register Transfer Language) expansion phase, where comparison operations are being lowered to target-specific assembly mnemonics. The mnemonics appear to follow x86 floating-point comparison conventions.
