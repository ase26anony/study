Looking at this code snippet, it appears to be part of a switch statement that handles different comparison conditions for floating-point operations. The code is converting internal condition codes to their string representations for output to a file.

The pattern shows:
- Internal condition codes (like `UNORDERED`, `ORDERED`, `UNEQ`, etc.)
- Their corresponding string representations ("unord", "ord", "ueq", etc.)

This looks like it could be from a compiler code generator (possibly GCC) that's outputting assembly code. The condition codes appear to be for floating-point comparisons where special handling is needed for NaN values.

Some observations:
1. `UNORDERED` → "unord" (true if either operand is NaN)
2. `ORDERED` → "ord" (true if neither operand is NaN)
3. `UNEQ` → "ueq" (unordered or equal)
4. `UNGE` → "nlt" (not less than, which includes unordered cases)
5. `UNGT` → "nle" (not less than or equal)
6. `UNLE` → "ule" (unordered or less than or equal)
7. `UNLT` → "ult" (unordered or less than)
8. `LTGT` → "une" (not equal and ordered, i.e., less than or greater than but not unordered)

The code is missing cases for the standard ordered comparisons (like `EQ`, `NE`, `LT`, `LE`, `GT`, `GE`), which suggests this is only handling the special floating-point cases that differ from integer comparisons.

Is there something specific you'd like to know about this code or would you like me to explain any particular aspect of it?
