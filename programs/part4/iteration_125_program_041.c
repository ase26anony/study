Let's break this down step-by-step.

---

## **1. Understanding the code structure**

We have two possible ranges for `x`:

- **If `cond` is true**:  
  `x = some_value % 100`  
  Since `% 100` yields a value in `[0, 99]`, `x ∈ [0, 99]`.

- **If `cond` is false**:  
  `x = 200 + (some_value % 100)`  
  This gives `x ∈ [200, 299]`.

After the `if-else`, we compute:
```cpp
int y = x & 0xFC;
```
where `0xFC` in binary is `11111100` (decimal 252).

---

## **2. Effect of `x & 0xFC`**

`0xFC` masks off the lowest 2 bits of `x` (bits 0 and 1), leaving bits 2 and above unchanged.

So `y` will be `x` rounded down to the nearest multiple of 4.

---

## **3. Possible values of `y` in each branch**

### **Branch 1 (`cond` true):**  
`x ∈ [0, 99]`  
`y = x & 0xFC` means `y` will be in `{0, 4, 8, ..., 96}` (multiples of 4 ≤ 99).  
Max `y` here is `96`.

### **Branch 2 (`cond` false):**  
`x ∈ [200, 299]`  
`y = x & 0xFC` means `y` will be in `{200, 204, 208, ..., 296}` (multiples of 4 between 200 and 299).  
Max `y` here is `296`.

---

## **4. Merging the possible ranges for `y`**

From branch 1: `y ∈ {0, 4, 8, ..., 96}`  
From branch 2: `y ∈ {200, 204, ..., 296}`

These two sets are disjoint because branch 1 max is 96, branch 2 min is 200.

So `y` can be **any multiple of 4** in:
- `[0, 96]` (step 4)  
- `[200, 296]` (step 4)

No other values possible.

---

## **5. Final answer**

The possible values of `y` are:

\[
y \in \{0, 4, 8, \dots, 96\} \cup \{200, 204, 208, \dots, 296\}
\]

That is, all multiples of 4 between 0 and 96 inclusive, and all multiples of 4 between 200 and 296 inclusive.
