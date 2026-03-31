Now the inner loop is over `j`, and `input[i]` is constant across `j`.  
We can use **SIMD** to compute all 4 `j` at once:

Let:
- `acc_vec` = `[acc0, acc1, acc2, acc3]`
- `mul_vec` = `[mul0, mul1, mul2, mul3]`
- `input_broadcast` = `[input[i], input[i], input[i], input[i]]`

Then each step is:
\[
\text{acc\_vec} = \text{acc\_vec} \times \text{mul\_vec} + \text{input\_broadcast}
\]

---

## Step 3 — SIMD pseudocode (using intrinsics, e.g., SSE/AVX)

Assuming 32-bit integers:
