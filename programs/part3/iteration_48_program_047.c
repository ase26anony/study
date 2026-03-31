## Key Insight

The phi node `val_3` can have two possible values:
- `1` from Path A (when `some_condition(i)` is true)
- `0` from Path B (when `some_condition(i)` is false)

The condition `val_3 == 0` will be true **only when** Path B was taken (i.e., `some_condition(i)` is false).

## Optimization Opportunity

A compiler could potentially optimize this by:
1. **Condition propagation**: The check `val_3 == 0` is equivalent to checking if Path B was taken
2. **Dead code elimination**: If the compiler can prove something about `some_condition(i)`, it might optimize the phi node away
3. **Branch simplification**: Could potentially rewrite as:
