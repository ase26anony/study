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
In decimal, `0xFC = 252`.

The `& 0xFC` operation clears the **lowest 2 bits** of `x`, making `x` a multiple of 4, and also masks to only keep bits up to bit 7 (since `0xFC` is `0b11111100`, it zeros bits beyond 7 if `x` is larger than 8 bits, but here `x` fits in at least 9 bits because max is 299).

---

### **Case 1: `x ∈ [0, 99]`**
- `x & 0xFC` will be in `{0, 4, 8, …, 96}` (multiples of 4 from 0 to ≤99).  
  Max here is `96` because 100 & 0xFC = 96, but 100 is not in range (max x=99 → 99 & 0xFC = 96).

---

### **Case 2: `x ∈ [200, 299]`**
Let’s see what `x & 0xFC` does for these values.

`0xFC` in binary: `11111100` (8 bits).  
If we think of `x` in binary, `& 0xFC` will zero the lower 2 bits of `x`, but also **if `x` has bits above bit 7 set, those remain** because `0xFC` is effectively `0x000000FC` in 32-bit (assuming `int` is 32-bit), so bits above bit 7 are unchanged.

Let’s check `200`:
- `200` in binary: `11001000`  
  `200 & 0xFC = 11001000 & 11111100 = 11001000` = 200.

`201`: `11001001 & 11111100 = 11001000` = 200.

So for `x ∈ [200, 203]`, `x & 0xFC = 200`.  
For `x ∈ [204, 207]`, result = 204, etc., up to `x ∈ [296, 299]`, result = 296.

So possible results in this case:  
`{200, 204, 208, …, 296}` (multiples of 4 from 200 to ≤299).

---

## **3. Merging possible values of `y`**

From case 1: `y ∈ {0, 4, 8, …, 96}`  
From case 2: `y ∈ {200, 204, 208, …, 296}`

So `y` is a multiple of 4, and either ≤96 or ≥200 and ≤296.

---

## **4. Can `y` be 100?**
No, because:
- In case 1, max is 96.
- In case 2, min is 200.

So 100 is impossible.

---

## **5. Can `y` be 252?**
Check case 1: max 96 → no.  
Case 2: 252 is a multiple of 4, is it in `[200, 296]`? Yes, 252 is in that range.  
Could `x` be 252, 253, 254, or 255?  
Yes, if `x ∈ [252, 255]` and `x` is in `[200, 299]`, then `x & 0xFC = 252`.  
So `y = 252` is possible.

---

**Final answer:**
- **100** → ❌ not possible
- **252** → ✅ possible

---

\[
\boxed{252}
\]
