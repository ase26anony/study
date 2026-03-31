This is a **sequential recurrence** because each iteration depends on the result of the previous iteration (`a[i-1]`).  
This creates a **loop-carried dependency**, meaning the loop cannot be parallelized directly.

---

## **Dependency Analysis**

Let’s write the recurrence explicitly:

\[
a[i] = b[i] \cdot c[i] + a[i-1]
\]

with \( a[-1] \) presumably being some initial value (maybe `a[0] = b[0] * c[0] + a[-1]`).

Expanding:

\[
a[0] = b[0] c[0] + a[-1]
\]
\[
a[1] = b[1] c[1] + a[0] = b[1] c[1] + b[0] c[0] + a[-1]
\]
\[
a[2] = b[2] c[2] + a[1] = b[2] c[2] + b[1] c[1] + b[0] c[0] + a[-1]
\]

So in general:

\[
a[i] = a[-1] + \sum_{k=0}^{i} b[k] \cdot c[k]
\]

---

## **Parallelization Approach**

The recurrence is a **prefix sum** (cumulative sum) of the array `d[i] = b[i] * c[i]`, with an initial offset `a[-1]`.

Thus:

\[
a[i] = \text{prefix\_sum}(d)[i] + a[-1]
\]

where `d[i] = b[i] * c[i]`.

**Prefix sums can be parallelized** using well-known algorithms (e.g., work-efficient parallel scan in \(O(n)\) work, \(O(\log n)\) span).

---

## **Parallel Pseudocode (using prefix sum)**

```cpp
// Step 1: Compute d[i] = b[i] * c[i] in parallel
parallel_for i = 0 to N-1:
    d[i] = b[i] * c[i]

// Step 2: Perform parallel prefix sum on d
prefix = parallel_scan(d)  // returns array prefix where prefix[i] = sum_{k=0}^{i} d[k]

// Step 3: Add initial offset a[-1] in parallel
parallel_for i = 0 to N-1:
    a[i] = prefix[i] + a[-1]
```

---

## **Complexity**

- Sequential version: \(O(N)\)
- Parallel version (using a parallel scan):  
  Work: \(O(N)\)  
  Span: \(O(\log N)\)  
  Parallel time (with enough processors): \(O(\log N)\)

---

## **Note on `a[-1]`**
In the original loop, `a[i-1]` for `i=0` accesses `a[-1]`, which is out of bounds unless `a[-1]` is a special variable holding the initial value.  
In practice, you’d handle this by having an initial value `init = a[-1]` (maybe passed separately), and start the loop from `i=0` with `a[0] = b[0]*c[0] + init`.
