This code appears to be from a compiler code generator (likely GCC or similar) that's outputting assembly mnemonics for floating-point comparison conditions. It's converting internal comparison codes to their corresponding assembly mnemonics.

Here's what each case is doing:

**Case breakdown:**
- `UNORDERED` → outputs "unord" (unordered comparison)
- `ORDERED` → outputs "ord" (ordered comparison)
- `UNEQ` → outputs "ueq" (unordered equal)
- `UNGE` → outputs "nlt" (not less than)
- `UNGT` → outputs "nle" (not less than or equal)
- `UNLE` → outputs "ule" (unordered less than or equal)
- `UNLT` → outputs "ult" (unordered less than)
- `LTGT` → outputs "une" (unordered not equal)

**Context:**
This is likely part of an x86 or x86-64 assembly backend, specifically handling floating-point comparisons. These mnemonics correspond to the `FCOM`/`FUCOM` family of instructions and their condition code checks.

**The pattern:**
- "unord" = unordered (at least one operand is NaN)
- "ord" = ordered (neither operand is NaN)
- "ueq" = unordered or equal
- "nlt" = not less than (greater than or equal)
- "nle" = not less than or equal (greater than)
- "ule" = unordered or less than or equal
- "ult" = unordered or less than
- "une" = unordered or not equal

The `default` case handles errors when an invalid condition code is passed.
