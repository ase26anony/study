## Key Design Features:

1. **Partial Block Overlap**: The loops are designed so that:
   - Some inner loops have blocks that are strict subsets of outer loops
   - Sequential loops at the same level share some basic blocks (like condition checks) but not all
   - Different loop types (for, while, do-while) create varied control flow graphs

2. **Complex Control Flow**:
   - Multiple `continue` and `break` statements
   - Conditional statements inside loops
   - Switch statements creating multiple basic blocks
   - Early exits from loops

3. **Hardware Loop Candidates**:
   - Loops with compile-time known bounds (8, 32, etc.)
   - Simple induction variables
   - Arithmetic operations in loop bodies

4. **Optimization Prevention**:
   - `volatile` variables in loop bounds
   - `__attribute__((noinline, noipa))` on functions
   - Data dependencies between loop results
   - Final observable output via `printf`

## Compilation and Testing:
