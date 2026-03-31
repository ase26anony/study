Let's break this down step by step.

---

## **1. Understanding the possible ranges of `x`**

From the code:

- **If `cond` is true**:  
  `x = some_value % 100` → `x ∈ [0, 99]` (since `% 100` yields `0` to `99`).
  The comment says `[0, 100]`, but `% 100` can’t give `100`, so it’s actually `[0, 99]`.

- **If `cond` is false**:  
  `x = 200 + (some_value % 100)` → `x ∈ [200, 299]`.

So after the `if-else`, `x` is either in `[0, 99]` or `[200, 299]`.

---

## **2. Applying `y = x & 0xFC`**

`0xFC` in binary is `11111100` (the lowest 2 bits are `0`, so this operation clears the lowest 2 bits of `x`).

Let’s see what happens in each range:

### **Case 1: `x ∈ [0, 99]`**
- The largest number here is `99` (`0b01100011`).
- `x & 0xFC` will zero out bits 0 and 1, so `x` becomes the largest multiple of 4 ≤ `x`.
- Possible results: `0, 4, 8, …, 96` (since 99 & 0xFC = 96).
- So `y ∈ {0, 4, 8, …, 96}`.

### **Case 2: `x ∈ [200, 299]`**
- Smallest: `200` (`0b11001000`), `200 & 0xFC = 200`.
- Largest: `299` (`0b100101011` in 9 bits, but let’s check: 299 decimal = `0x12B` = `0b1 0010 1011`; lower 8 bits = `0b10101011` = 171? Wait, let’s compute carefully.)

Better to compute directly:  
299 in binary: 256 + 32 + 8 + 2 + 1 = `1 0010 1011`.  
Lower 8 bits: `0010 1011` = 43. But that’s wrong — I’m mixing up. Let’s do it cleanly:

299 decimal:  
256 (2^8) + 43 = 299.  
43 in binary: `0010 1011`.  
So 299 = `1 0010 1011` binary (9 bits).  
Now `0xFC` = `11111100` (8 bits).  
We must take `x` mod 256 for the lower 8 bits before AND? No — `&` is bitwise on all bits, but `0xFC` is 8-bit, so it will zero lower 2 bits of the lower 8 bits of `x`, upper bits unchanged.

So `x & 0xFC` for `x = 299`:  
299 = `0b1 0010 1011`  
Lower byte: `0010 1011` = 0x2B  
0x2B & 0xFC = 0x28 (since 0x2B = 0b00101011 → clear last 2 bits → 0b00101000 = 0x28).  
Upper bits: `1` (from bit 8) remains because 0xFC doesn’t touch bit 8.  
So result = 256 + 0x28 = 256 + 40 = 296.

Check: 299 & 0xFC = 296. Yes.

Similarly, 200 & 0xFC = 200 (since 200 = 0xC8 = 0b11001000, last two bits already 0).

So in this range, `y ∈ {200, 204, 208, …, 296}`.

---

## **3. Merging both cases**

Possible `y` values:

- From first case: `{0, 4, 8, …, 96}` (step 4)
- From second case: `{200, 204, 208, …, 296}` (step 4)

So `y` is always a multiple of 4, and either `y ≤ 96` or `y ≥ 200`.

---

## **4. Range summary**

`y ∈ {0, 4, 8, …, 96} ∪ {200, 204, …, 296}`.

That means `y` is never in `[100, 199]` and never odd, always divisible by 4, and never 100..199.

---

**Final answer:**
```cpp
// y is in {0, 4, 8, ..., 96} ∪ {200, 204, ..., 296}
// i.e., y is a multiple of 4, and either ≤ 96 or ≥ 200.
```
