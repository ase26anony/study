This code appears to be from a compiler's reload pass (likely GCC), specifically handling different types of reload operations. Here's a breakdown of what each case does:

## Purpose
The code determines where to store reload instructions based on the type of reload operation being performed. Each reload type has a different destination pointer (`where`) that points to a specific location where the generated reload instructions should be stored.

## Reload Types Explained

1. **`RELOAD_OTHER`** - General reloads not tied to specific operands
   - Stored in `other_input_reload_insns`

2. **`RELOAD_FOR_INPUT`** - Reloads for input operands
   - Stored in `input_reload_insns[rl->opnum]` (indexed by operand number)

3. **`RELOAD_FOR_INPUT_ADDRESS`** - Reloads for addresses of input operands
   - Stored in `input_address_reload_insns[rl->opnum]`

4. **`RELOAD_FOR_INPADDR_ADDRESS`** - Reloads for addresses of input address reloads (nested addressing)
   - Stored in `inpaddr_address_reload_insns[rl->opnum]`

5. **`RELOAD_FOR_OUTPUT_ADDRESS`** - Reloads for addresses of output operands
   - Stored in `output_address_reload_insns[rl->opnum]`

6. **`RELOAD_FOR_OUTADDR_ADDRESS`** - Reloads for addresses of output address reloads
   - Stored in `outaddr_address_reload_insns[rl->opnum]`

7. **`RELOAD_FOR_OPERAND_ADDRESS`** - Reloads for operand addresses
   - Stored in `operand_reload_insns`

8. **`RELOAD_FOR_OPADDR_ADDR`** - Reloads for operand address addresses
   - Stored in `other_operand_reload_insns`

9. **`RELOAD_FOR_OTHER_ADDRESS`** - Other address reloads
   - Stored in `other_input_address_reload_insns`

## Key Points
- The `rl->opnum` field contains the operand number, used to index into arrays for operand-specific reloads
- This organization helps the compiler keep track of different categories of reload instructions for proper code generation
- The `gcc_unreachable()` at the end handles invalid reload types (should never be reached in correct code)
- This is part of GCC's register allocation/reload phase which handles cases where operands can't be directly loaded into registers
