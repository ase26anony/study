   - The casts to `unsigned HOST_WIDE_INT` ensure unsigned comparison
   - If high parts differ, we know the result immediately

2. **If high parts are equal, compare the low parts**:
