Looking at this code snippet, it appears to be part of a switch statement that converts comparison operation codes to their string representations for output. The code handles various comparison conditions like UNORDERED, ORDERED, UNEQ, etc., and writes corresponding strings like "unord", "ord", "ueq", etc., to a file.

The code seems to be from a compiler or assembler backend, likely for outputting comparison operations in assembly language. The pattern suggests it's handling floating-point comparison conditions where special codes are needed for unordered comparisons (NaN handling).

The switch cases map:
- UNORDERED → "unord" (unordered comparison)
- ORDERED → "ord" (ordered comparison)  
- UNEQ → "ueq" (unordered equal)
- UNGE → "nlt" (not less than)
- UNGT → "nle" (not less than or equal)
- UNLE → "ule" (unordered less than or equal)
- UNLT → "ult" (unordered less than)
- LTGT → "une" (unordered not equal)

The `default` case handles errors when an invalid condition code is passed.
