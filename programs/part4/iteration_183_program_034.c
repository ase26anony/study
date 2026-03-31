This code appears to be from a compiler backend (likely GCC) that's handling floating-point comparison conditions for output to assembly code. It's converting internal comparison codes to their corresponding assembly mnemonics.

Looking at the pattern, this seems to be for x86 floating-point comparisons where:
- `UNORDERED` → "unord" (unordered comparison)
- `ORDERED` → "ord" (ordered comparison)  
- `UNEQ` → "ueq" (unordered or equal)
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered or less than or equal)
- `UNLT` → "ult" (unordered or less than)
- `LTGT` → "une" (not equal, ordered)

The naming convention suggests these are for the `FCOM`/`FUCOM` family of x86 floating-point instructions, where conditions are tested using the `FSTSW`/`FNSTSW` instructions and the `SAHF` trick or direct testing of the status word.

The code is missing the closing brace for the `switch` statement and likely has more cases after the `default` case. The `output_operand_lossage` function would be called when an invalid condition code is encountered.
