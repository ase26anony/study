This code appears to be from a compiler's reload pass (likely GCC), specifically handling different types of reload operations. Here's a breakdown:

## Purpose
This is a switch statement that determines where to store reload instructions based on the type of reload operation being performed. Each case corresponds to a different reason why a reload is needed.

## Reload Types Explained

1. **RELOAD_OTHER** - General reloads not tied to specific operands
2. **RELOAD_FOR_INPUT** - Reloads for input operands
3. **RELOAD_FOR_INPUT_ADDRESS** - Reloads for addresses of input operands
4. **RELOAD_FOR_INPADDR_ADDRESS** - Reloads for addresses of input addresses (nested addressing)
5. **RELOAD_FOR_OUTPUT_ADDRESS** - Reloads for addresses of output operands
6. **RELOAD_FOR_OUTADDR_ADDRESS** - Reloads for addresses of output addresses
7. **RELOAD_FOR_OPERAND_ADDRESS** - Reloads for operand addresses
8. **RELOAD_FOR_OPADDR_ADDR** - Reloads for operand address addresses
9. **RELOAD_FOR_OTHER_ADDRESS** - Reloads for other address computations

## Key Points:
- `rl->opnum` refers to the operand number in the instruction
- Each `where` pointer points to a different list/array that collects reload instructions for that specific purpose
- The reload pass handles cases where operands need to be moved between registers/memory due to register allocation constraints
- The `gcc_unreachable()` at the end ensures all cases are handled at compile time

This is part of the compiler's backend that manages register allocation spill/fill operations when the register allocator cannot satisfy all constraints directly.
