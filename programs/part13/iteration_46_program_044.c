**Key features that should trigger the partitioning type lookup:**

1. **Multiple partitioning scenarios**: The code uses `gang worker vector`, `gang vector`, and nested loop configurations that should trigger different integer codes (0-7) in the partitioning analysis.

2. **Complex data clauses**: `private`, `firstprivate`, `reduction`, `copy`, `copyin` clauses with multi-dimensional arrays and nested loops.

3. **Conditional data access**: Multiple `if` conditions with different access patterns that may force different broadcasting strategies.

4. **Mixed OpenACC/OpenMP**: OpenMP outside OpenACC regions and `acc update` used with host-side OpenMP computations.

5. **Persistent device data**: `acc declare create` for global arrays, with updates throughout the computation.

6. **Dynamic loop bounds**: Using `argc` prevents constant folding and optimization away of the compute regions.

**Recommended compilation commands to maximize coverage:**
