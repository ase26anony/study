Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for outputting their string representations to a file. The code is likely from a compiler or assembler backend that generates assembly code.

The code handles various floating-point comparison conditions:
- `UNORDERED`: outputs "unord" (unordered comparison)
- `ORDERED`: outputs "ord" (ordered comparison)
- `UNEQ`: outputs "ueq" (unordered or equal)
- `UNGE`: outputs "nlt" (not less than)
- `UNGT`: outputs "nle" (not less than or equal)
- `UNLE`: outputs "ule" (unordered or less than or equal)
- `UNLT`: outputs "ult" (unordered or less than)
- `LTGT`: outputs "une" (unordered or not equal)

The default case handles errors when an invalid condition code is passed.

This is typical code for generating assembly mnemonics for floating-point comparison operations, where different conditions need to be mapped to their corresponding assembly instruction suffixes or modifiers.
