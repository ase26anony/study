## What's Happening

1. **SSA Requirement**: Each variable is assigned exactly once
2. **Branch Creates Two Versions**: 
   - `val1 = 1` in the true branch
   - `val2 = 2` in the false branch
3. **PHI Node Merges Values**: At the control flow merge point, a φ (phi) function selects the appropriate value based on which path was taken

## The PHI Node in Detail

The φ function works like this:
