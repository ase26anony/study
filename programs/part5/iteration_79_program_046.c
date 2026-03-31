Looking at this code, I can see it has nested loops with some shared blocks. Let me analyze the structure:

## Code Structure:
- **Outer loop L1**: Runs N times
- **Two conditional inner loops** (L2 and L3) that may execute depending on conditions
- **Shared blocks**: `shared_block_1()` appears in both L1 and L2, `shared_block_2()` appears in both L1 and L3

## Execution Pattern:
For each iteration of L1:
1. Execute `shared_block_1()` once
2. If `condition_a` is true, execute L2 (M times):
   - Each iteration: `shared_block_1()` + `unique_to_A()`
3. Execute `shared_block_2()` once  
4. If `condition_b` is true, execute L3 (P times):
   - Each iteration: `shared_block_2()` + `unique_to_B()`

## Key Observations:
- `shared_block_1()` is executed **N times** from L1, plus **N×M×p_a** times from L2 (where p_a is probability `condition_a` is true)
- `shared_block_2()` is executed **N times** from L1, plus **N×P×p_b** times from L3 (where p_b is probability `condition_b` is true)
- The inner loops are independent and don't nest within each other
- Both conditions are evaluated on each outer iteration

## Potential Optimizations:
1. **Code duplication**: The shared blocks appear in multiple places
2. **Condition evaluation**: Both conditions are checked every iteration regardless of previous results
3. **Loop-invariant code**: Check if anything in the shared blocks could be hoisted outside loops

Would you like me to suggest specific optimizations or analyze the time complexity?
