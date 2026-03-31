Looking at this code, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations, likely in a compiler backend. The code is outputting assembly mnemonics for various floating-point comparison conditions.

Here's what each case is doing:

1. **UNORDERED**: Outputs "unord" (unordered comparison)
2. **ORDERED**: Outputs "ord" (ordered comparison)  
3. **UNEQ**: Outputs "ueq" (unordered or equal)
4. **UNGE**: Outputs "nlt" (not less than)
5. **UNGT**: Outputs "nle" (not less than or equal)
6. **UNLE**: Outputs "ule" (unordered or less than or equal)
7. **UNLT**: Outputs "ult" (unordered or less than)
8. **LTGT**: Outputs "une" (unordered or not equal)

The naming conventions suggest this is for x86 assembly output, as these mnemonics match x86's floating-point comparison instructions (FUCOM, FCOM, etc. with these condition codes).

The `default` case handles invalid condition codes by calling `output_operand_lossage` with an error message.

This code would typically be part of a larger function that outputs assembly code for floating-point comparisons in a compiler's code generation phase.
