This code appears to be from a compiler backend (likely GCC) that's handling floating-point comparison operations and their inverse/negated forms. Let me break down what's happening:

## Context
This is part of a switch statement that converts internal comparison operation codes to their assembly mnemonics for output. The codes represent different floating-point comparison conditions.

## The Pattern

The code shows two naming conventions:
1. **Direct comparisons**: `UNORDERED`, `ORDERED`, `UNEQ`, `LTGT`
2. **Negated comparisons**: `UNGE`, `UNGT`, `UNLE`, `UNLT` (which output as "nlt", "nle", "ule", "ult")

## What the Codes Mean

- **UNORDERED**: At least one operand is NaN (Not a Number)
- **ORDERED**: Neither operand is NaN
- **UNEQ**: Unordered or Equal (either NaN or equal)
- **UNGE**: Unordered or Greater Than or Equal
- **UNGT**: Unordered or Greater Than
- **UNLE**: Unordered or Less Than or Equal  
- **UNLT**: Unordered or Less Than
- **LTGT**: Less Than or Greater Than (ordered and not equal)

## The Output Mnemonics

The interesting part is how negated comparisons are output:
- `UNGE` → "nlt" (not less than)
- `UNGT` → "nle" (not less than or equal)
- `UNLE` → "ule" (unordered or less than or equal) - this one seems inconsistent
- `UNLT` → "ult" (unordered or less than) - also inconsistent

There appears to be a mix of conventions here. Some use "n" prefix for negation, while others use the direct unordered form.

## Missing Case

The code snippet cuts off at the `default` case, which would handle invalid condition codes by calling `output_operand_lossage()` with an error message.

This pattern is typical in compiler code generation where internal IR (Intermediate Representation) operations need to be mapped to target-specific assembly instructions.
