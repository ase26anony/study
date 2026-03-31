This gives 4-way parallelism.

---

## **5. Parallelizing across i (harder)**

The recurrence over `i` is a prefix sum/scans with multiplication and addition:

\[
\text{acc}_{i+1} = m \cdot \text{acc}_i + x_i
\]

This is a **linear first-order recurrence**, which can be parallelized using **parallel scan** techniques (if we treat it as an associative operation? Let’s check).

---

### **Associativity check**
We can think of each step as applying a linear transformation to `(acc, constant)`:

Let state be \( (a, c) \) meaning `acc = a * init + c`.  
Step with `(m, x_i)` does:

New acc = \( a \cdot \text{init} + c \) → multiply by m → \( m \cdot a \cdot \text{init} + m \cdot c \), then add \( x_i \):  
So new state: \( (m \cdot a, \; m \cdot c + x_i) \).

Composition:  
First `(a1, c1)`, then `(a2, c2)` gives:

Start: acc = \( a1 \cdot \text{init} + c1 \)  
After first: acc = \( a1 \cdot \text{init} + c1 \)  
After second: multiply by a2: \( a2 \cdot a1 \cdot \text{init} + a2 \cdot c1 \), then add c2:  
Final: \( (a2 \cdot a1, \; a2 \cdot c1 + c2) \).

This is associative:  
Combine `(a1, c1)` with `(a2, c2)` = `(a2*a1, a2*c1 + c2)`.

So we can use **parallel prefix scan** to compute all intermediate states in \( O(\log n) \) steps with \( O(n) \) work.

---

### **Parallel scan algorithm outline**

We precompute for each i the transformation `(m, x_i)`.  
We want to compute the composition of transformations from 0 to i-1, apply to init.

1. Build transformation array `T[i] = (mul[j], input[i])` for fixed j.
2. Do parallel prefix scan using the associative combine operator above.
3. Result for step i is `T[0..i-1]` composed, applied to init.

But here we only need the final result after all 5000 steps, so we just need the total transformation after n steps:

Total transform = composition of all `(m, x_i)` = `(m^n, sum_{k=0}^{n-1} x_k * m^{n-1-k})`.

Then final acc = init * m^n + that sum.

---

### **Parallel computation of m^n and the sum**

We can compute m^n in parallel with repeated squaring (O(log n)).

The sum \( S = \sum_{k=0}^{n-1} x_k \cdot m^{n-1-k} \) can be split into chunks:

Split into P chunks:  
Chunk r handles indices \( k = r \cdot \frac{n}{P} \) to \( (r+1)\cdot\frac{n}{P} - 1 \).

Let \( L = n/P \).  
Chunk r’s terms: \( x_{rL} m^{n-1-rL} + x_{rL+1} m^{n-2-rL} + \dots \).

Factor out \( m^{n-1-rL} \):  
Chunk r sum = \( m^{n-1-rL} \cdot (x_{rL} + x_{rL+1} m^{-1} + \dots) \) — but m^{-1} may not be integer.

Better: Each chunk computes local Horner sum with m over its chunk, then scales by m^(remaining steps).

Indeed:  
Chunk r’s contribution to S = m^{n-1-(rL+L-1)} * (local Horner result over chunk starting at x_{rL} with length L).

Wait, let’s define:  
Local Horner: start with acc=0, for s=0..L-1: acc = acc * m + x_{rL + s}.  
This gives: acc_local = x_{rL}*m^{L-1} + x_{rL+1}*m^{L-2} + ... + x_{rL+L-1}.

We want S = sum over r of (acc_local_r * m^{n - rL - L}).

Because acc_local_r * m^{n - rL - L} = x_{rL}*m^{n-1-rL} + ... (matches earlier).

So algorithm:

1. Split input into P chunks.
2. Each chunk computes acc_local = Horner over its chunk (serial within chunk).
3. Also compute m^(L) for scaling.
4. Then combine:  
   Start with total = 0.  
   For r = 0 to P-1: total = total * m^L + acc_local_r.

This is just Horner again over chunks! So we can parallelize across chunks but combine serially, or do a parallel reduction tree.

---

## **6. Final parallelized version (across i and j)**

We can parallelize over j (4-way) and also parallelize the inner loop using chunking:
