**Key features that should trigger the partitioning string lookup:**

1. **Complex Data Clauses**: Multiple `private`, `firstprivate`, and `reduction` clauses on different data types (arrays and scalars).

2. **Mixed Partitioning Directives**: 
   - `gang worker vector` (fully partitioned)
   - `gang vector` (gang+vector partitioned)  
   - `gang worker` (gang+worker partitioned)
   - Nested `loop vector` inside `parallel loop gang worker`

3. **Multi-dimensional Arrays**: 2D arrays with nested loop access patterns that require different broadcasting strategies.

4. **Conditional Data Access**: `if` conditions inside parallel regions create data-dependent access patterns that may require different neutering/broadcast decisions.

5. **Mixed OpenACC/OpenMP**: OpenMP parallel for outside OpenACC regions stresses the compiler's ability to manage multiple offloading paradigms.

6. **`acc declare` Usage**: Persistent device data (`global_matrix`, `global_sum`) that requires partitioning decisions across multiple compute regions.

**Recommended compilation commands to maximize coverage:**
