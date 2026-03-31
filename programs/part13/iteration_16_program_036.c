This code appears to be from a compiler's register allocation/reload phase, specifically handling different types of reload operations. Here's a breakdown of what it does:

## Purpose
This switch statement determines where to store reload instructions based on the type of reload operation being performed. Each case corresponds to a different context where reloads might be needed.

## Reload Types Explained

1. **`RELOAD_OTHER`** - General reloads not tied to specific operands
2. **`RELOAD_FOR_INPUT`** - Reloads for input operands
3. **`RELOAD_FOR_INPUT_ADDRESS`** - Reloads for addresses of input operands
4. **`RELOAD_FOR_INPADDR_ADDRESS`** - Reloads for addresses of input addresses (nested addressing)
5. **`RELOAD_FOR_OUTPUT_ADDRESS`** - Reloads for addresses of output operands
6. **`RELOAD_FOR_OUTADDR_ADDRESS`** - Reloads for addresses of output addresses
7. **`RELOAD_FOR_OPERAND_ADDRESS`** - Reloads for operand addresses
8. **`RELOAD_FOR_OPADDR_ADDR`** - Reloads for operand address addresses
9. **`RELOAD_FOR_OTHER_ADDRESS`** - Reloads for other address computations

## Key Points:
- `rl->opnum` refers to the operand number, used to index into arrays for input/output-specific reloads
- The `where` pointer is being set to point to different instruction lists based on the reload type
- This is part of organizing reload instructions so they can be inserted in the correct places in the instruction stream
- The `gcc_unreachable()` at the end handles invalid reload types (should never be reached in correct code)

This pattern is typical in compilers that need to handle complex addressing modes and register allocation scenarios, especially in architectures with limited registers or complex memory addressing requirements.
