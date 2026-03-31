Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations. The code is converting internal condition codes to their string representations for assembly output.

Here's what each case represents:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (unordered or not equal)

These are typical floating-point comparison conditions used in assembly languages like x86. The code is likely from a compiler backend that generates assembly code for floating-point comparisons.

The `default` case handles invalid condition codes by calling `output_operand_lossage` with an error message.

This pattern is common in compiler code generation where internal representations of operations need to be translated to target-specific assembly mnemonics.
