Looking at this code snippet, it appears to be part of a switch statement that outputs different string representations for comparison operators or condition codes. The code seems to be converting from some internal representation to a textual representation for output to a file.

The pattern suggests this is likely from a compiler code generator, possibly GCC or similar, where different comparison operations need to be output in assembly language format. The names like `UNORDERED`, `ORDERED`, `UNEQ`, etc. suggest floating-point comparison operations.

Here's what each case appears to be doing:

- `UNORDERED` → outputs "unord" (unordered comparison)
- `ORDERED` → outputs "ord" (ordered comparison)  
- `UNEQ` → outputs "ueq" (unordered or equal)
- `UNGE` → outputs "nlt" (not less than)
- `UNGT` → outputs "nle" (not less than or equal)
- `UNLE` → outputs "ule" (unordered or less than or equal)
- `UNLT` → outputs "ult" (unordered or less than)
- `LTGT` → outputs "une" (unordered or not equal)

The default case handles errors when an invalid condition code is passed.

The code structure is typical for an output routine in a compiler backend that needs to emit assembly mnemonics for different comparison operations.
