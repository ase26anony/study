// To increase pressure further:
// 1. Add more interdependent variables
// 2. Use more floating point operations (FP registers are separate)
// 3. Add function calls within loops
// 4. Use array operations to force memory accesses
// 5. Add SIMD operations for vector register pressure

// Example enhancement:
__m128 v1, v2, v3;  // SSE registers
double da, db, dc;   // More FP precision variants
