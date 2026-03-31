Looking at this code, I can see it has **nested loops** with some interesting characteristics. Let me break down the structure:

## Loop Structure
- **Outer loop L1**: Runs `N` times
- **Inner loop A (L2)**: Runs `M` times, but only when `condition_a` is true
- **Inner loop B (L3)**: Runs `P` times, but only when `condition_b` is true

## Code Block Analysis

### Shared Blocks:
1. **`shared_block_1()`**: 
   - Called once per outer loop iteration (L1)
   - Also called inside inner loop A (L2) - `M` times per L1 iteration when `condition_a` is true

2. **`shared_block_2()`**:
   - Called once per outer loop iteration (L1)
   - Also called inside inner loop B (L3) - `P` times per L1 iteration when `condition_b` is true

### Unique Blocks:
- **`unique_to_A()`**: Only inside inner loop A (L2)
- **`unique_to_B()`**: Only inside inner loop B (L3)

## Execution Counts (assuming conditions are always true):

| Block | Total Executions |
|-------|------------------|
| `shared_block_1()` | `N + N×M = N(1+M)` |
| `shared_block_2()` | `N + N×P = N(1+P)` |
| `unique_to_A()` | `N×M` |
| `unique_to_B()` | `N×P` |

## Time Complexity:
- **Worst-case**: O(N × max(M, P))
- **Best-case** (both conditions false): O(N)
- **Average-case**: Depends on probability of conditions being true

## Potential Optimizations:
1. **Code duplication**: The shared blocks appear in both outer and inner loops
2. **Condition checks**: Could potentially hoist some logic if conditions don't change within loops
3. **Loop fusion**: If `condition_a` and `condition_b` are mutually exclusive or related, could restructure

## Key Observations:
- The two inner loops are **independent** (not nested within each other)
- Each inner loop executes conditionally
- Shared code creates coupling between outer and inner loops
- This pattern might indicate an opportunity for **refactoring** to reduce code duplication
