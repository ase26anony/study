Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for outputting assembly code. The code is converting internal comparison codes to their corresponding assembly mnemonics.

Here's what each case appears to be doing:

1. **UNORDERED** → outputs "unord" (unordered comparison)
2. **ORDERED** → outputs "ord" (ordered comparison)  
3. **UNEQ** → outputs "ueq" (unordered or equal)
4. **UNGE** → outputs "nlt" (not less than)
5. **UNGT** → outputs "nle" (not less than or equal)
6. **UNLE** → outputs "ule" (unordered or less than or equal)
7. **UNLT** → outputs "ult" (unordered or less than)
8. **LTGT** → outputs "une" (unordered or not equal)

This looks like it's part of a compiler backend (possibly GCC) that generates floating-point comparison instructions for x86 or similar architectures. The mnemonics correspond to the condition codes used in floating-point comparison instructions like `FUCOM` and `FCOMI`.

The default case handles errors when an invalid condition code is passed.
