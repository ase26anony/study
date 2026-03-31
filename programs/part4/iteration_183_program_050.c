This code appears to be from a compiler backend (likely GCC) that's outputting assembly code for floating-point comparisons. It's handling different comparison modes for floating-point operations, particularly dealing with unordered comparisons which are special for floating-point due to NaN values.

Let me explain what each case represents:

**Standard comparisons:**
- `ORDERED`: Both operands are not NaN (ordered)
- `UNORDERED`: At least one operand is NaN (unordered)

**Unordered comparisons** (these handle NaN cases specially):
- `UNEQ`: Unordered or equal (a == b or either is NaN)
- `UNGE`: Unordered or greater than or equal (a >= b or either is NaN)
- `UNGT`: Unordered or greater than (a > b or either is NaN)
- `UNLE`: Unordered or less than or equal (a <= b or either is NaN)
- `UNLT`: Unordered or less than (a < b or either is NaN)

**Special case:**
- `LTGT`: Less than or greater than (a < b or a > b, but not equal and neither is NaN)

The interesting part is that the output strings don't always directly match the enum names. For example:
- `UNGE` outputs "nlt" (not less than)
- `UNGT` outputs "nle" (not less than or equal)
- `LTGT` outputs "une" (unordered or not equal)

This is because different assembly languages have different mnemonics for these comparisons, and the compiler is mapping its internal representation to the appropriate assembly syntax.

The `default` case handles errors when an invalid condition code is passed, calling `output_operand_lossage` with an error message.
