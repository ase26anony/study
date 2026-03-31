(With `acc` initialized from `init` first.)

This accesses `input[i]` once for all `j`, better spatial locality.

---

### **4. Use-case example**
This pattern appears in:
- **Multiple linear congruential generators** in parallel (different multipliers, same increment sequence from `input`).
- **Horner’s method** for evaluating 4 different polynomials at different points (`mul[j]` is the variable `x`, `input` are coefficients, `init` is initial value).
- **Finite impulse response (FIR) filter** with 4 different filters applied to the same signal.

---

### **5. Example with numbers**
Let’s say:
