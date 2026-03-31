Let’s break this down step by step.

---

## **1. Understanding the ranges**

From the code:

- **If `cond` is true**:  
  `x = some_value % 100` → `x ∈ [0, 99]`.

- **If `cond` is false**:  
  `x = 200 + (some_value % 100)` → `x ∈ [200, 299]`.

So after the `if-else`, `x` is either in `[0, 99]` or `[200, 299]`.

---

## **2. The operation `y = x & 0xFC`**

`0xFC` in binary is `11111100` (8 bits).  
In decimal: `0xFC = 252`.

The `& 0xFC` operation clears the lowest 2 bits of `x` (bits 0 and 1), leaving the rest unchanged.

---

## **3. Effect on each range**

### **Case 1: `x ∈ [0, 99]`**
- The largest number here is 99 (`01100011` in binary).  
  `0xFC` = `11111100` in 8 bits, but `x` fits in 7 bits for `[0,99]` except 96–99 need 7 bits.
- `x & 0xFC` will zero out bits 0 and 1, so `x` becomes a multiple of 4, but also `& 0xFC` for small `x` is same as `x & 252`.
- Let’s check max: 99 & 0xFC = 99 & 252 = 96.  
  Min: 0 & 252 = 0.
- Possible results: `{0, 4, 8, …, 96}` (multiples of 4 from 0 to 96).

---

### **Case 2: `x ∈ [200, 299]`**
- 200 in binary: `11001000`.  
  `200 & 0xFC` = `11001000 & 11111100` = `11001000` = 200.
- 201 & 0xFC = 200, etc., up to 203 & 0xFC = 200.
- 204 & 0xFC = 204 (since 204 is multiple of 4 and less than 252 in lower bits? Wait, 204 = `11001100`, & 0xFC = `11001100` = 204).
- Actually, `& 0xFC` only clears bits 0 and 1, so it rounds down to the nearest multiple of 4 ≤ x.

Let’s check boundaries:
- 299 = `1 0010 1011` (9 bits needed).  
  299 & 0xFC = `1 0010 1011 & ...11111100` = `1 0010 1000` = 296.

So range for `y` in this case: from 200 to 296, step 4.

---

## **4. Merged possible values of `y`**

From case 1: `{0, 4, 8, …, 96}`  
From case 2: `{200, 204, 208, …, 296}`

No overlap because case 1 max = 96, case 2 min = 200.

So `y` is in:
\[
\{0, 4, 8, \dots, 96\} \cup \{200, 204, 208, \dots, 296\}
\]

---

## **5. Final answer**

\[
\boxed{[0,96] \text{ step 4} \cup [200,296] \text{ step 4}}
\]
More precisely:  
`y ∈ {4k | k=0..24} ∪ {4k | k=50..74}`.
