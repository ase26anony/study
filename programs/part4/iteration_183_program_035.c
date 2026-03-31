This code appears to be from a compiler backend (likely GCC or similar) that's outputting assembly code for floating-point comparisons. It's converting internal comparison codes to assembly mnemonics for different floating-point comparison modes.

Here's what each case is doing:

1. **UNORDERED**: Outputs "unord" (unordered comparison)
2. **ORDERED**: Outputs "ord" (ordered comparison)
3. **UNEQ**: Outputs "ueq" (unordered or equal)
4. **UNGE**: Outputs "nlt" (not less than)
5. **UNGT**: Outputs "nle" (not less than or equal)
6. **UNLE**: Outputs "ule" (unordered or less than or equal)
7. **UNLT**: Outputs "ult" (unordered or less than)
8. **LTGT**: Outputs "une" (unordered or not equal)

These are typical floating-point comparison mnemonics used in x86 assembly (like with the `FUCOM`/`FCOM` instructions and their variants). The "un" prefix typically means "unordered" (when one or both operands are NaN), and these mnemonics handle the various combinations of ordered/unordered comparisons.

The `default` case handles errors when an invalid condition code is passed.
