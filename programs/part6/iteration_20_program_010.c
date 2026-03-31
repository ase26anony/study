/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
volatile int vi = 100;
volatile unsigned int vu = 200;
volatile long vl = -50;
volatile float vf = 0.75f;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume_result(const void *ptr, int size) {
    asm volatile("" : : "r"(ptr), "r"(size) : "memory");
}

/* Hash function to return non-deterministic result */
__attribute__((noinline))
int compute_hash(_Accum *arr, int len) {
    int hash = 0;
    for (int i = 0; i < len; i++) {
        /* Access bits to create hash */
        union {
            _Accum f;
            int i;
        } u;
        u.f = arr[i];
        hash ^= u.i ^ (i * 0x5bd1e995);
    }
    return hash;
}

int main(void) {
    /* Array to store results */
    _Accum results[20];
    int result_idx = 0;
    
    /* Initialize volatile seeds for fixed-point values */
    volatile _Accum seed_acc = 0.5k;
    volatile _Fract seed_frac = 0.25r;
    volatile unsigned _Fract seed_ufrac = 0.75ur;
    volatile long _Accum seed_lacc = -0.999999999k;
    volatile short _Fract seed_sfrac = 0.9375hr;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 10; i++) {
        /* Vary the seeds based on loop iteration and volatile inputs */
        _Accum a = (_Accum)(seed_acc + (i * 0.1k));
        _Fract f = (_Fract)(seed_frac + (i * 0.05r));
        unsigned _Fract uf = (unsigned _Fract)(seed_ufrac + (i * 0.02ur));
        long _Accum la = (long _Accum)(seed_lacc + (i * 0.000000001k));
        short _Fract sf = (short _Fract)(seed_sfrac - (i * 0.0625hr));
        
        /* EXPRESSION 1: Near-maximum _Accum multiplication 
         * Should trigger max range check for signed accumulative types */
        _Accum a1 = 0.999999k;  /* Very close to max */
        _Accum a2 = 0.999999k;
        _Accum product = a1 * a2;  /* Product > max representable */
        results[result_idx++] = product;
        
        /* EXPRESSION 2: Left shift of fixed-point (emulated via integer cast and shift)
         * This should trigger the specific uncovered lines when checking range */
        int int_val = vi + i;
        _Accum shifted = (_Accum)int_val * 0.015625k;  /* Equivalent to << -6 */
        results[result_idx++] = shifted;
        
        /* EXPRESSION 3: Complex expression with unsigned _Fract near 1.0
         * Should trigger unsigned max range check */
        unsigned _Fract u1 = 0.9999999ur;
        unsigned _Fract u2 = 0.0000001ur;
        unsigned _Fract sum = u1 + u2;  /* Potential wrap-around */
        results[result_idx++] = (_Accum)sum;
        
        /* EXPRESSION 4: Signed fractional types at boundaries */
        _Fract f_neg = -0.999999r;
        _Fract f_pos = 0.999999r;
        _Fract f_prod = f_neg * f_pos;  /* Product near -1.0 */
        results[result_idx++] = (_Accum)f_prod;
        
        /* EXPRESSION 5: Conditional with mixed types
         * Forces range analysis for conversion paths */
        long _Accum la_val = (i & 1) ? la : -la;
        _Accum converted = (_Accum)la_val;
        results[result_idx++] = converted;
        
        /* EXPRESSION 6: Multi-step computation with intermediate overflow potential */
        _Accum temp = a * 2.0k;  /* Could overflow if a > 0.5 */
        _Accum final = temp * 0.75k;
        results[result_idx++] = final;
        
        /* EXPRESSION 7: Integer promotion with fixed-point */
        int int_prom = vu - i;
        _Accum mixed = (_Accum)int_prom * 0.0000152587890625k;  /* ~1/65536 */
        results[result_idx++] = mixed;
        
        /* EXPRESSION 8: Very small long _Accum values that underflow */
        long _Accum tiny = 0.000000001k;
        long _Accum scaled = tiny * 1000000000.0k;  /* Should be ~1.0 */
        results[result_idx++] = (_Accum)scaled;
        
        /* EXPRESSION 9: short _Fract boundary case */
        short _Fract sf_max = 0.9375hr;  /* Max for short _Fract */
        short _Fract sf_half = 0.5hr;
        short _Fract sf_result = sf_max + sf_half;  /* Would overflow */
        results[result_idx++] = (_Accum)sf_result;
        
        /* EXPRESSION 10: Data-dependent shift-like operation */
        _Accum base = 0.25k;
        int shift_amt = (vi + i) % 8;
        /* Emulate left shift: multiply by power of two */
        _Accum shifted2 = base;
        for (int s = 0; s < shift_amt; s++) {
            shifted2 = shifted2 * 2.0k;
        }
        results[result_idx++] = shifted2;
    }
    
    /* Ensure we don't exceed array bounds */
    if (result_idx > 20) result_idx = 20;
    
    /* Consume results to prevent optimization */
    consume_result(results, sizeof(_Accum) * result_idx);
    
    /* Return hash of results */
    return compute_hash(results, result_idx);
}
