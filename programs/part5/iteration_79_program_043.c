Looking at this code, I can see it has **nested loops** with some shared and unique blocks. Let me analyze the structure:

## Loop Structure
- **Outer loop (L1)**: Runs `N` times
- **Inner loop A (L2)**: Runs `M` times, but only when `condition_a` is true
- **Inner loop B (L3)**: Runs `P` times, but only when `condition_b` is true

## Execution Pattern
For each iteration of the outer loop:
1. Execute `shared_block_1()` once
2. If `condition_a` is true:
   - Execute `shared_block_1()` + `unique_to_A()` `M` times
3. Execute `shared_block_2()` once
4. If `condition_b` is true:
   - Execute `shared_block_2()` + `unique_to_B()` `P` times

## Key Observations
- `shared_block_1()` appears in both L1 and L2
- `shared_block_2()` appears in both L1 and L3
- The inner loops are **conditional** (only execute when their respective conditions are true)
- The conditions `condition_a` and `condition_b` are evaluated on each outer loop iteration
- The inner loops have different iteration counts (`M` and `P`)

## Potential Optimizations
1. **Code duplication**: `shared_block_1()` and `shared_block_2()` appear in multiple places
2. **Condition evaluation**: The conditions are checked on every outer iteration
3. **Loop fusion**: If `condition_a` and `condition_b` are often both true, you might combine operations

## Time Complexity
- Worst case: **O(N × (M + P))** when both conditions are always true
- Best case: **O(N)** when both conditions are always false
- Average case depends on the probability of each condition being true

Would you like me to suggest any specific optimizations or transformations for this code structure?
