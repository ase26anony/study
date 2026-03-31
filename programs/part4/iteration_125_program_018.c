Let's break this down step by step.

---

## **1. Understanding the ranges**

From the code:

- **If `cond` is true**:  
  `x = some_value % 100` → `x ∈ [0, 99]`.

- **If `cond` is false**:  
  `x = 200 + (some_value % 100)` → `x ∈ [200, 299]`.

So after the `if-else`, `x` is either in `[0, 99]` or `[200, 299]`.

---

## **2. Operation `y = x & 0xFC`**

`0xFC` in binary is `11111100` (8 bits).  
In decimal: `0xFC = 252`.

The `& 0xFC` operation clears the lowest 2 bits of `x` (makes them `0`), leaving the rest unchanged.

---

### **Case 1: `x ∈ [0, 99]`**
- Max in this range is `99` = binary `01100011`.  
- `0xFC` = `11111100`.  
- `x & 0xFC` will zero out bits 0 and 1, so `x` becomes a multiple of 4, ≤ `x` originally.
- Possible results:  
  For `x` from 0 to 99, `x & 0xFC` ∈ `{0, 4, 8, …, 96}` (since 100 & 0xFC = 96, but 100 is not in range; max x=99 → 99 & 0xFC = 96).
  So range: `[0, 96]`, step 4.

---

### **Case 2: `x ∈ [200, 299]`**
Let's check boundaries:

- `200` = binary `11001000` → `200 & 0xFC = 200` (since lowest 2 bits are 0 already).
- `201` = `11001001` → `200`.
- `202` = `11001010` → `200`.
- `203` = `11001011` → `200`.
- `204` = `11001100` → `204`.

So `x & 0xFC` will be in `{200, 204, 208, …, 296}` (since max x=299 → 299 & 0xFC = 296).

Range: `[200, 296]`, step 4.

---

## **3. Merged possible values for `y`**

From both cases:  
`y` ∈ `[0, 96] step 4` ∪ `[200, 296] step 4`.

No overlap because `96 < 200`.

---

## **4. Final answer**

Possible values of `y` are:

\[
\{0, 4, 8, \dots, 96\} \quad \cup \quad \{200, 204, 208, \dots, 296\}
\]

That is, all multiples of 4 from 0 to 96 inclusive, and all multiples of 4 from 200 to 296 inclusive.

---

\[
\boxed{[0,96] \text{ step 4} \cup [200,296] \text{ step 4}}
\]
