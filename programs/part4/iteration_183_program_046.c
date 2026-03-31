Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations, likely in a compiler code generation context. The code is converting internal condition code representations to their corresponding assembly mnemonics.

Here's what each case appears to be doing:

1. **UNORDERED** → "unord" (unordered comparison)
2. **ORDERED** → "ord" (ordered comparison)  
3. **UNEQ** → "ueq" (unordered or equal)
4. **UNGE** → "nlt" (not less than)
5. **UNGT** → "nle" (not less than or equal)
6. **UNLE** → "ule" (unordered or less than or equal)
7. **UNLT** → "ult" (unordered or less than)
8. **LTGT** → "une" (unordered or not equal)

The naming convention suggests these are floating-point comparison conditions that handle NaN (Not-a-Number) cases. The "UN" prefix typically indicates "unordered" comparisons that consider NaN values.

This looks like it could be from GCC or another compiler's backend code generation, specifically for x86 floating-point comparison operations where different mnemonics are needed to handle the various IEEE 754 floating-point comparison scenarios properly.

The `default` case handles invalid condition codes by calling an error function `output_operand_lossage()` with an appropriate error message.
