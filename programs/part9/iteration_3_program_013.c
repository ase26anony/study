## Key Design Elements:

1. **Register Pressure Creation**:
   - Multiple local variables (8-10 per function) that must be kept live across calls
   - `register` hints for specific call-clobbered registers (r10, r11, r12)
   - Inline asm that clobbers call-used registers in helper functions

2. **Basic Block Structure**:
   - Loops create natural basic block boundaries
   - Switch statements create multiple basic blocks with different patterns
   - Each test function has instructions positioned to potentially be at BB_END

3. **Call Patterns**:
   - Consecutive calls to non-inline functions that clobber registers
   - Results used in subsequent computations to maintain liveness
   - Mix of different helper functions to prevent pattern recognition

4. **Movement Candidates**:
   - Instructions like `v3 = v4 + v5 + i` placed right before loop back-edges
   - Pointer arithmetic (`ptr++`) that might need repositioning relative to spills
   - Arithmetic using call-clobbered registers that span calls

## Compilation Options:

For best coverage of the target lines, compile with:
