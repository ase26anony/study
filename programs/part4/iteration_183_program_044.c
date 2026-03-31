Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations, likely in a compiler code generator. The code is outputting assembly mnemonics for various floating-point comparison conditions.

Here's what each case appears to be doing:

1. **UNORDERED**: Outputs "unord" - typically used for unordered comparison (when either operand is NaN)
2. **ORDERED**: Outputs "ord" - typically used for ordered comparison (when neither operand is NaN)
3. **UNEQ**: Outputs "ueq" - unordered or equal
4. **UNGE**: Outputs "nlt" - not less than (greater than or equal, or unordered)
5. **UNGT**: Outputs "nle" - not less than or equal (greater than, or unordered)
6. **UNLE**: Outputs "ule" - unordered or less than or equal
7. **UNLT**: Outputs "ult" - unordered or less than
8. **LTGT**: Outputs "une" - not equal and ordered (less than or greater than)

The `default` case handles invalid condition codes with an error message.

This looks like it could be from GCC or another compiler's backend code, specifically the part that generates floating-point comparison instructions for x86 or similar architectures. The mnemonics correspond to the condition codes used in floating-point comparison instructions like `FCOM` or `FUCOM` followed by conditional jumps or moves.
