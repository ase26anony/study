Now the inner `j` loop is over 4 elements — easy to vectorize with SIMD intrinsics or compiler auto-vectorization.

---

## **4. Potential issues**

- **Integer overflow**: If `mul[j]` and `input[i]` are large, `acc` might overflow `int`. The C/C++ standard says signed overflow is undefined behavior, so we’d need to ensure it doesn’t happen or use `unsigned` wrap-around semantics if intended.
- **Performance**: The interchanged version accesses `acc[j]` and `mul[j]` sequentially, good for SIMD. But `input[i]` is broadcast to all 4 lanes (in SIMD terms).

---

## **5. SIMD implementation idea (SSE/AVX2)**

Using AVX2 for 8× `int` parallel (if we had 8 recurrences), but for 4× `int`, SSE is enough:
