**Key features that should trigger the uncovered reload types:**

1. **Extreme Register Pressure**: 30+ integer variables, 10 doubles, 10 longs all actively used and kept live across loop iterations.

2. **Complex Addressing Modes**: 
   - Multi-term index calculations: `(i * 7 + v1 * 3 + v2) % size_int`
   - Pointer arithmetic with scaling: `arr_int + (v12 * 3 + v13 * 7 + v14)`
   - Nested addressing through pointers-to-pointers

3. **Inline Assembly with Conflicting Constraints**:
   - Mix of `"r"`, `"m"`, `"+r"` constraints
   - Memory operands that require address computations
   - Clobber lists that force register spilling

4. **Control Flow Splitting Live Ranges**:
   - Switch statement with 8 cases
   - Address computations in one case used in another
   - Variables live across switch boundaries

5. **Mixed Operand Types**:
   - Integers, doubles, and longs used interchangeably
   - Addresses used as both data and address operands
   - Volatile memory accesses preventing optimization

6. **Function Calls with Many Arguments**:
   - Non-inline functions force register shuffling
   - Different argument combinations across calls

**Compilation recommendations**:
