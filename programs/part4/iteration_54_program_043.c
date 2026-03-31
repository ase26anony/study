   - Casts to `unsigned HOST_WIDE_INT` (likely `uint64_t`) for unsigned comparison
   - If high parts differ, comparison is complete

2. **If high parts are equal, compare the low parts:**
