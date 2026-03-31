This test program is designed to comprehensively trigger the uncovered pretty-printer code:

1. **Covers all `update` modifier cases**: The program includes `depend(in:)`, `depend(inout:)`, `depend(out:)`, `depend(mutexinoutset:)`, `depend(inoutset:)`, and `depend(destroy:)` clauses.

2. **Triggers pretty-printer via diagnostics**: Each OpenMP construct is followed by an unused variable assignment, which will trigger `-Wunused-variable` warnings. When GCC generates these warnings, it invokes the pretty-printer to display the problematic OpenMP constructs.

3. **Multiple contexts for robustness**:
   - Template function `process_depend_clauses` ensures all clause types are instantiated
   - Class member function `OpenMPProcessor::process_with_depend`
   - Lambda expression in `nested_depend_context`
   - Global function `global_scope_test`
   - Parallel region with task dependencies in `main`

4. **AST complexity**: Using templates, lambdas, classes, and global variables increases AST complexity, ensuring the pretty-printer processes all constructs.

5. **Valid OpenMP usage**: The code uses `#pragma omp declare target` for variables used in target regions, making the OpenMP constructs semantically valid.

**Compilation commands to trigger coverage**:

1. **For warning-based coverage** (triggers pretty-printer during diagnostic generation):
