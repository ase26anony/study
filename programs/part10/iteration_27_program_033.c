This is a **loop-unrolled vectorizable pattern** — you’re doing the same operation on 4 independent streams of data (`init[0..3]`, `mul[0..3]`), processing the same `input[i]` across all 4 streams at each `i`.  

We can **vectorize** this manually using SIMD intrinsics (e.g., SSE/AVX) to process all 4 `acc` values in parallel.

Here’s a rewritten version using **AVX2** (assuming 32-bit integers):

```cpp
#include <immintrin.h>

void compute(int* init, int* mul, int* input, int* result) {
    // Load init and mul into SIMD registers
    __m128i v_acc = _mm_loadu_si128((__m128i*)init);   // acc[0..3]
    __m128i v_mul = _mm_loadu_si128((__m128i*)mul);    // mul[0..3]

    for (int i = 0; i < 5000; i++) {
        // Broadcast input[i] to all 4 lanes
        __m128i v_bcast = _mm_set1_epi32(input[i]);
        
        // acc = acc * mul + input[i]
        v_acc = _mm_add_epi32(_mm_mullo_epi32(v_acc, v_mul), v_bcast);
    }

    // Store results back
    _mm_storeu_si128((__m128i*)result, v_acc);
}
```

**Explanation:**
- `__m128i` holds 4 × 32-bit integers.
- `_mm_set1_epi32` broadcasts `input[i]` to all 4 lanes.
- `_mm_mullo_epi32` multiplies 32-bit integers (low 32 bits of each product).
- The loop runs 5000 times, updating all 4 accumulators in parallel per iteration.

**Benefits:**
- 4× fewer loop iterations in the inner loop (scalar version had 4 inner loops × 5000 iterations each = 20k iterations; this does 5000 iterations processing 4 lanes at once).
- Better instruction-level parallelism.

**Requirements:**
- CPU with SSE4.1 (for `_mm_mullo_epi32`) or AVX2 for wider versions.
- If using `float`/`double`, adjust intrinsics accordingly.

**For wider SIMD (AVX2 with 8 lanes):**
If you had 8 independent streams, you’d use `__m256i` and `_mm256_...` intrinsics, but here you have exactly 4 streams, so SSE4.1/AVX (128-bit) is sufficient.
