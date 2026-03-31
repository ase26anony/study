## Key Design Elements:

1. **High Register Pressure**: 12+ volatile local variables with complex interdependencies force register spilling.

2. **Multiple Insertion Points**: 
   - Function calls (`helper1`, `helper2`, `helper3`, `external_func`)
   - Inline assembly with explicit register clobbering
   - Both create opportunities for caller-save insertion

3. **Mid-Block Insertions**: The inline assembly statements are placed between variable uses, ensuring insertions happen inside basic blocks (not at boundaries).

4. **BB_END Update Potential**: The sequence `helper2()` → `asm volatile()` is designed so `helper2()` might be the block end before insertion, triggering the `BB_END(bb) = ins` update.

5. **Anti-Optimization Measures**:
   - `volatile` variables prevent elimination
   - `noinline`/`noipa` attributes preserve function calls
   - Complex computations keep variables live
   - Loop with varying seed prevents constant folding

## Compilation Options:
