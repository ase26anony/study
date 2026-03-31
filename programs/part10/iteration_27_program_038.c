This is a **loop-unrolled vectorizable pattern** — you’re doing the same recurrence relation across 4 different `mul` and `init` values, but with the same `input` sequence.

If you want to **vectorize** this manually (e.g., using SIMD intrinsics), you could process all 4 `acc` values in parallel for each `i`.

Here’s a **C++ version using AVX2 intrinsics** (assuming 32-bit integers):

```cpp
#include <immintrin.h>

void compute(int init[4], int mul[4], int input[5000], int result[4]) {
    __m128i acc = _mm_loadu_si128((__m128i*)init);   // load init[0..3]
    __m128i mulv = _mm_loadu_si128((__m128i*)mul);   // load mul[0..3]

    for (int i = 0; i < 5000; i++) {
        __m128i in = _mm_set1_epi32(input[i]);       // broadcast input[i]
        acc = _mm_add_epi32(_mm_mullo_epi32(acc, mulv), in);
    }

    _mm_storeu_si128((__m128i*)result, acc);         // store result[0..3]
}
```

**How it works:**
- `acc` is a 128-bit SIMD register holding `[acc0, acc1, acc2, acc3]`.
- `mulv` holds `[mul0, mul1, mul2, mul3]`.
- Each iteration broadcasts `input[i]` to all 4 lanes.
- The operation `acc = acc * mulv + input[i]` is done in parallel for all 4 lanes.

**Benefits:**
- Processes 4 recurrence relations simultaneously.
- Reduces loop overhead.
- Compilers might auto-vectorize the original, but this ensures explicit SIMD usage.

**Note:**  
If you’re on a system without AVX2, you can use SSE4.1 (`_mm_mullo_epi32` requires at least SSE4.1). For wider vectors (AVX-512), you could handle 16 lanes at once.
