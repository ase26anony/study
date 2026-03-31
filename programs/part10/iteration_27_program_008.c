This is a **loop-unrolled vectorizable pattern** — you’re performing the same recurrence across 4 independent streams, each with its own `init` and `mul` values, over the same `input` array.

If you want to **vectorize this manually** (e.g., using SIMD intrinsics), you could restructure it so that for each `i`, you process all 4 `j` values in parallel.

Here’s a **conceptual rewrite** using C++ with AVX2 intrinsics (assuming 32-bit integers):

```cpp
#include <immintrin.h>

void compute(int init[4], int mul[4], int input[5000], int result[4]) {
    __m128i acc = _mm_loadu_si128((__m128i*)init);   // load init[0..3]
    __m128i mulv = _mm_loadu_si128((__m128i*)mul);   // load mul[0..3]

    for (int i = 0; i < 5000; i++) {
        __m128i input_broadcast = _mm_set1_epi32(input[i]); // broadcast input[i]
        acc = _mm_add_epi32(_mm_mullo_epi32(acc, mulv), input_broadcast);
    }

    _mm_storeu_si128((__m128i*)result, acc);
}
```

**What changed:**
- The outer `j` loop disappears — instead, we treat `init` and `mul` as 4-element vectors.
- Inside the `i` loop, we broadcast `input[i]` to all 4 SIMD lanes.
- Each step does `acc = acc * mul + input[i]` for all 4 streams in parallel.

**Benefits:**
- Processes 4 streams at once.
- Reduces loop overhead.
- Uses SIMD parallelism.

**Caveats:**
- Requires AVX2 support for `_mm_mullo_epi32`.
- If `mul` and `init` aren’t 16-byte aligned, use `loadu`/`storeu`.
- If you need more than 4 streams, use wider vectors (AVX512 for 16 ints at once).
- Integer overflow behavior matches the original if that matters.

This transformation is essentially **horizontal to vertical vectorization** — turning separate scalar accumulations into a single vector accumulation.
