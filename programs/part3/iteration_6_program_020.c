## The PHI node (`%val`):

- **Location**: At the start of the `merge` block
- **Function**: Selects the correct value based on which predecessor block was executed
- **Syntax**: `phi type [value1, block1], [value2, block2], ...`
- **At runtime**: When control flow reaches `merge`, the PHI node takes the value from the block that was just executed

## Why PHI nodes are necessary in SSA:

Without PHI nodes, you'd need to violate SSA's "single assignment" rule:
