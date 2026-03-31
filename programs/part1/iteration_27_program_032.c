Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations. The code is outputting assembly mnemonics for various unordered/ordered floating-point comparison conditions.

Here's what each case represents:

1. **UNORDERED**: Outputs "unord" - tests if either operand is NaN (unordered)
2. **ORDERED**: Outputs "ord" - tests if neither operand is NaN (ordered)
3. **UNEQ**: Outputs "ueq" - unordered or equal
4. **UNGE**: Outputs "nlt" - not less than (greater than or equal, or unordered)
5. **UNGT**: Outputs "nle" - not less than or equal (greater than, or unordered)
6. **UNLE**: Outputs "ule" - unordered or less than or equal
7. **UNLT**: Outputs "ult" - unordered or less than
8. **LTGT**: Outputs "une" - not equal and ordered (less than or greater than)

This looks like it's from a compiler code generator (possibly GCC) that's generating assembly code for floating-point comparisons. The mnemonics appear to be x86 assembly style for the `FCOM`/`FUCOM` family of instructions.

The default case handles errors when an invalid condition code is passed, calling `output_operand_lossage` with an error message.

Is there something specific you'd like to know about this code or would you like me to explain any particular aspect in more detail?
