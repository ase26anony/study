## Key Design Elements:

1. **Conditional Branches with Constant RHS (0 or 1):**
   - Multiple `if (var == 0)`, `if (var == 1)`, `if (var != 0)`, `if (var != 1)` statements
   - Also `if (var > 0)` (RHS is 0) and `if (var < 1)` (RHS is 1)

2. **SSA-Form LHS from PHI Nodes:**
   - Variables modified in loops (`loop_var` in hot loop)
   - Variables assigned in different control flow paths (`phi_var` in cold function)
   - All comparisons use SSA_NAMEs as LHS

3. **Chain of Single Assignments:**
   - Multiple chains like `tmp1 = loop_var; tmp2 = tmp1; tmp3 = tmp2;`
   - Each assignment is a simple copy (GIMPLE_ASSIGN with single SSA_NAME RHS)

4. **Multiple Successor Edges:**
   - Each conditional has both taken and not-taken paths
   - Use of `volatile` and input-dependent values prevents constant folding
   - Nested conditionals create complex control flow

5. **Annotated Basic Blocks:**
   - Hot loop executes 1,000,000 times
   - Cold function called only 1 in 1000 iterations
   - Clear hot/cold path differentiation for profile annotation

6. **Function Complexity:**
   - Multiple functions with different attributes (`hot`, `cold`, `noinline`)
   - Nested loops and conditionals
   - Function calls within hot paths

## Compilation and Execution:
