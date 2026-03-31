/* test_fixed_range.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 10;
static volatile int vi2 = -5;
static volatile unsigned int vu1 = 100;
static volatile _Fract vf = 0.5r;
static volatile _Accum va = 0.5k;
static volatile long _Accum vla = 0.999999999lk;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *ptr = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = ptr[i];
    }
}

/* Helper to create complex range conditions */
__attribute__((noinline, noipa))
_Accum complex_range_expr(_Accum a, _Accum b, int shift) {
    /* Multi-step expression to force range analysis */
    _Accum temp = a * b;           /* Could overflow */
    _Accum shifted = temp;         /* Keep for potential shift */
    
    /* Conditional based on value ranges */
    if (shift > 0) {
        /* Simulate left shift via multiplication */
        for (int i = 0; i < shift && i < 4; i++) {
            shifted = shifted * 2.0k;  /* Potential overflow */
        }
    }
    
    /* Mix with integer promotion */
    int i_temp = (_Fract)a * 100;  /* Conversion range check */
    _Accum result = shifted + (_Accum)i_temp * 0.01k;
    
    return result;
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[8] = {0k};
    unsigned _Fract uf_results[8] = {0ur};
    
    /* Edge-case values for different fixed-point types */
    const long _Accum max_laccum = 0.999999999lk;
    const long _Accum min_laccum = -1.0lk;
    const unsigned _Fract max_ufract = 0.9999999ur;
    const _Fract max_fract = 0.9999999r;
    const _Fract min_fract = -1.0r;
    const _Accum max_accum = 0.9999999k;
    const _Accum min_accum = -1.0k;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 8; i++) {
        /* Use volatile to prevent compile-time evaluation */
        int idx = vi1 + i;
        if (idx < 0) idx = -idx;
        idx = idx % 7;
        
        /* Test 1: Signed accumulative types near maximum */
        _Accum a1 = max_accum - (_Accum)idx * 0.0000001k;
        _Accum a2 = max_accum - (_Accum)(idx * 2) * 0.0000001k;
        _Accum prod = a1 * a2;  /* Likely overflows range */
        
        /* Test 2: Left shift simulation for accum */
        _Accum shifted = prod;
        if (idx & 1) {
            shifted = shifted * 2.0k;  /* Overflow check */
        }
        
        /* Test 3: Complex expression with data-dependent ranges */
        results[i] = complex_range_expr(a1, a2, idx);
        
        /* Test 4: Unsigned fract near 1.0 */
        unsigned _Fract uf = max_ufract;
        for (int j = 0; j < (idx % 3); j++) {
            uf = uf * 0.9999999ur;  /* Stays in range but needs checking */
        }
        
        /* Test 5: Addition that could wrap for unsigned fract */
        unsigned _Fract small = 0.0000001ur;
        uf = uf + small * (_Fract)idx;  /* Range analysis needed */
        
        /* Test 6: Conditional with mixed types */
        uf = (idx > 2) ? (uf * 0.5ur) : (uf + 0.0000001ur);
        
        uf_results[i] = uf;
        
        /* Test 7: Long accum with extreme values */
        long _Accum la = vla;
        if (idx & 2) {
            la = la * 0.999999999lk;  /* Near max product */
        } else {
            la = la * (-0.999999999lk); /* Near min product */
        }
        
        /* Convert to results array */
        results[i] = results[i] + (_Accum)la;
        
        /* Test 8: Integer to fixed-point with range check */
        int large_int = vi2 * idx * 1000;
        _Accum from_int = (_Accum)large_int * 0.001k;  /* Conversion overflow check */
        results[i] = results[i] + from_int;
    }
    
    /* Additional edge case: Direct maximum value operations */
    _Accum max_test = max_accum;
    _Accum almost_one = 0.9999999k;
    
    /* This multiplication should trigger overflow range check */
    _Accum overflow_test = max_test * almost_one;
    results[0] = results[0] + overflow_test;
    
    /* Test with minimum values */
    _Accum min_test = min_accum;
    _Accum neg_almost_one = -0.9999999k;
    _Accum underflow_test = min_test * neg_almost_one;
    results[1] = results[1] + underflow_test;
    
    /* Mix signed and unsigned operations */
    unsigned _Fract uf_max = max_ufract;
    _Fract sf_max = max_fract;
    
    /* Conversion between signed/unsigned needs range analysis */
    _Fract mixed = (_Fract)uf_max * sf_max;
    results[2] = results[2] + (_Accum)mixed;
    
    /* Prevent dead code elimination */
    consume(results, sizeof(results));
    consume(uf_results, sizeof(uf_results));
    
    /* Create a hash to return (prevents optimization of entire program) */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= *((int*)&results[i]);
        hash ^= *((int*)&uf_results[i]);
    }
    
    return hash & 0xFF;
}
