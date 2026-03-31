This is a **loop-unrolled vectorizable pattern** — it’s computing 4 independent accumulations in parallel, each with a different multiplier (`mul[j]`) and starting from a different initial value (`init[j]`), but all using the same input sequence `input[i]`.

---

## **What the code does**

For each `j` in `{0,1,2,3}`:

1. Start with `acc = init[j]`.
2. For each `i` from `0` to `4999`:
   - Update:  
     \[
     \text{acc} = \text{acc} \times \text{mul}[j] + \text{input}[i]
     \]
3. Store final `acc` in `result[j]`.

---

## **Mathematical form**

This is essentially evaluating a **linear recurrence** for each `j`:

Let \( a_{j,i} \) be the accumulator value for multiplier `mul[j]` at step `i`, with \( a_{j,0} = \text{init}[j] \).

Then:
\[
a_{j,i+1} = a_{j,i} \cdot m_j + x_i
\]
where \( m_j = \text{mul}[j] \), \( x_i = \text{input}[i] \).

---

## **Closed-form expression**

This recurrence has a known closed form:
\[
a_{j,N} = \text{init}[j] \cdot (m_j)^N + \sum_{k=0}^{N-1} x_k \cdot (m_j)^{N-1-k}
\]
where \( N = 5000 \).

So `result[j]` is:
\[
\text{result}[j] = \text{init}[j] \cdot m_j^{5000} + \sum_{k=0}^{4999} \text{input}[k] \cdot m_j^{4999-k}
\]

---

## **Potential optimization**

The code is written in a way that **prevents straightforward SIMD vectorization** across `j` because `mul[j]` and `init[j]` are different for each `j`.  
However, if `mul[0] == mul[1] == mul[2] == mul[3]`, then the inner loop could be vectorized across the 4 accumulators.

If the multipliers are different, you could still vectorize by using **FMA (fused multiply-add)** instructions per lane, but each lane would use a different multiplier constant.

---

## **Alternative rewriting for clarity**

If you wanted to make it look more symmetric (though not necessarily faster), you could swap the loops:

```cpp
int acc[4];
for (int j = 0; j < 4; j++) acc[j] = init[j];

for (int i = 0; i < 5000; i++) {
    for (int j = 0; j < 4; j++) {
        acc[j] = acc[j] * mul[j] + input[i];
    }
}

for (int j = 0; j < 4; j++) result[j] = acc[j];
```

But this changes memory access pattern — the original has better locality in the inner loop (sequential `input[i]` access). The rewritten version accesses `input[i]` 4 times per `i`, which is fine if it’s in register/cache.
