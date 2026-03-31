### Key Features Triggering Internal Clauses:

1. **`OMP_CLAUSE_ENTER`**: 
   - `#pragma omp declare target enter(global_data)` at file scope directly generates an `enter` clause.

2. **`OMP_CLAUSE__REDUCTEMP_`**:
   - Multiple `reduction` clauses in a single construct (e.g., `reduction(+:sum1, sum2)`).
   - Array reductions (`reduction(+:arr_sum[:5])`).
   - Custom reduction (`declare reduction`).
   - Target offloading with reduction (`target teams ... reduction`).

3. **`OMP_CLAUSE__CONDTEMP_`**:
   - `if` clauses with runtime-dependent conditions (`if(target: argc > 2)`, `if(parallel: threshold > 25)`).
   - `final` clause with runtime condition (`final(argc > 4)`).
   - Volatile variables in conditions.

4. **`OMP_CLAUSE__SCANTEMP_`**:
   - Explicit `scan` directive with `inscan` reduction (`reduction(inscan, +:scan_sum)`).

### Compilation & Execution:
