## Why PHI nodes are needed:

- **SSA requirement**: Each variable can only be assigned once
- **Multiple definitions**: `val` has two possible definitions (1 or 2)
- **Path-sensitive values**: The actual value depends on which path was executed

## Visual representation:
