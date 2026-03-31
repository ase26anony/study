   - If the unsigned high part of `a` is less than `b`'s, return -1
   - If greater, return 1
   - The casts to `unsigned HOST_WIDE_INT` ensure unsigned comparison

2. **If high parts are equal, compare the low parts:**
