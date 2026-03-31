This is a **loop-unrolled vectorizable pattern** — you’re doing the same operation on 4 independent streams of data (`mul[0..3]`, `init[0..3]`, `result[0..3]`), with the same `input[i]` used across all 4 in each inner iteration.

---

## **What it’s doing**
For each `j` in `0..3`:

1. Start with `acc = init[j]`
2. For each `i` from `0` to `4999`:
   - Multiply `acc` by `mul[j]`
   - Add `input[i]`
   - Store back into `acc`
3. Store final `acc` into `result[j]`

Mathematically, for a given `j`:

\[
\text{result}[j] = \text{init}[j] \times (\text{mul}[j])^{5000} + \sum_{k=0}^{4999} \text{input}[k] \times (\text{mul}[j])^{5000 - k - 1}
\]

It’s essentially evaluating a polynomial in `mul[j]` with coefficients from `input` and an initial constant term `init[j]`.

---

## **Potential optimization**
If `mul[j]` values are the same for all `j`, or if you can use SIMD (Single Instruction Multiple Data), this can be parallelized across the 4 `j` values for each `i`.

Example with **pseudo SIMD intrinsics** (like Intel SSE/AVX):

```cpp
__m128i acc = _mm_load_si128((__m128i*)init);
__m128i mul_vec = _mm_load_si128((__m128i*)mul);

for (int i = 0; i < 5000; i++) {
    __m128i input_vec = _mm_set1_epi32(input[i]); // broadcast input[i] to all lanes
    acc = _mm_add_epi32(_mm_mullo_epi32(acc, mul_vec), input_vec);
}
_mm_store_si128((__m128i*)result, acc);
```

This processes all 4 `j` iterations in parallel for each `i`.

---

## **Complexity**
- Outer loop: 4 iterations
- Inner loop: 5000 iterations  
→ Total iterations = \(4 \times 5000 = 20000\) multiply-add operations.

If SIMD is used, inner loop still 5000 steps, but each step does 4 operations in parallel → effectively 5000 vector multiply-adds instead of 20000 scalar ops.
