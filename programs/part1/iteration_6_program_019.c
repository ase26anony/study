## What it's doing:

1. **Structure**: A `double_int` appears to be a structure with two parts:
   - `high`: The high-order bits (likely 64 bits)
   - `low`: The low-order bits (likely 64 bits)
   - Together they form a 128-bit integer

2. **Comparison logic**:
   - First compares the high parts as **unsigned** integers
   - If high parts differ, return result immediately
   - If high parts are equal, compare the low parts
   - Returns -1, 0, or 1 (though 0 case is missing - likely returns 0 at the end)

## Missing part:

The function should end with:
