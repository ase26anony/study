### Key Observations:

1. **Phi Node Location**: The phi node would be placed at the control flow merge point after the if-else but before the `val == 0` check.

2. **SSA Properties**:
   - Each variable (`val_A`, `val_B`, `val_phi`) is assigned exactly once
   - The phi node creates a new SSA variable that selects between `val_A` and `val_B` based on which path was taken

3. **Compiler's Perspective**:
