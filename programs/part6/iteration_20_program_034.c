/* test_fixed_point_range.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 0;
static volatile unsigned int vu1 = 0x7FFFFFFF;
static volatile unsigned int vu2 = 0x80000000;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, cold))
void consume_result(const void *ptr, int size) {
    volatile char sink;
    const char *p = (const char *)ptr;
    for (int i = 0; i < size; i++) {
        sink = p[i];
    }
}

/* Complex fixed-point computations designed to trigger range analysis */
int main(void) {
    /* Array to accumulate results */
    long _Accum results[16] = {0};
    int result_idx = 0;
    
    /* Initialize with volatile seeds to prevent compile-time evaluation */
    volatile _Accum seed_a = 0.5k;
    volatile _Accum seed_b = -0.5k;
    volatile unsigned _Fract seed_u = 0.9999999ur;
    volatile signed _Fract seed_s = 0.9999999r;
    volatile long _Accum seed_la = 0.999999999k;
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < 8; i++) {
        /* Vary inputs based on iteration and volatile seeds */
        _Accum a = (_Accum)(seed_a * i);
        _Accum b = (_Accum)(seed_b * (8 - i));
        unsigned _Fract u = seed_u;
        signed _Fract s = seed_s;
        long _Accum la = seed_la;
        
        /* 1. Multiplication near maximum range for _Accum */
        /* This should trigger max_r/max_s initialization and comparison */
        _Accum prod = a * b;
        results[result_idx++] = prod;
        
        /* 2. Left shift operations that could overflow */
        /* The shift amount varies to create different range scenarios */
        int shift = (i % 4) + 1;
        _Accum shifted = prod << shift;
        results[result_idx++] = shifted;
        
        /* 3. Operations with unsigned _Fract near 1.0 */
        /* May trigger unsigned greater-than comparison (ugt) */
        unsigned _Fract u2 = u + (unsigned _Fract)(0.0000001ur * i);
        results[result_idx++] = (_Accum)u2;
        
        /* 4. Complex expression mixing signed and unsigned */
        signed _Fract s2 = s - (signed _Fract)(0.0000001r * i);
        _Accum mixed = (_Accum)s2 * (_Accum)u2;
        results[result_idx++] = mixed;
        
        /* 5. Long _Accum operations that push boundaries */
        /* These are more likely to trigger the specific uncovered code path */
        long _Accum la2 = la * la;  /* Square of value near 1.0 */
        results[result_idx++] = la2;
        
        /* 6. Additional multiplication that could overflow */
        long _Accum la3 = la2 * la;
        results[result_idx++] = la3;
        
        /* 7. Conditional expressions with fixed-point */
        /* Forces range analysis for both branches */
        _Accum cond_result = (i & 1) ? prod : shifted;
        results[result_idx++] = cond_result;
        
        /* 8. Cast from integer with large magnitude */
        /* May trigger min_r/min_s initialization for negative values */
        int large_int = vu1 - i * 1000000;
        _Accum from_int = (_Accum)large_int * 0.001k;
        results[result_idx++] = from_int;
        
        /* 9. Negative boundary checks */
        /* May trigger min_s = min_s.alshift() and sext() logic */
        _Accum neg_val = -0.999999k;
        _Accum neg_prod = neg_val * neg_val;
        results[result_idx++] = neg_prod;
        
        /* 10. Shift with negative value */
        /* Could trigger different path in range analysis */
        _Accum neg_shifted = neg_val << 1;
        results[result_idx++] = neg_shifted;
    }
    
    /* 11. Additional edge case: maximum representable values */
    /* Directly try to create values at the boundary */
    long _Accum max_accum = 0.999999999999999k;
    long _Accum min_accum = -1.0k;
    
    /* Operations that should trigger overflow checks */
    long _Accum max_squared = max_accum * max_accum;
    long _Accum min_squared = min_accum * min_accum;
    
    results[0] = max_squared;  /* Overwrite first element */
    results[1] = min_squared;  /* Overwrite second element */
    
    /* 12. Use all fixed-point types in complex expression */
    short _Fract sf = 0.9999hr;
    long _Fract lf = 0.9999999lr;
    _Accum acc = 0.9999k;
    
    /* Multi-type expression that requires range analysis across types */
    _Accum complex_expr = (_Accum)sf * (_Accum)lf * acc;
    results[2] = complex_expr;
    
    /* 13. Integer promotion with fixed-point */
    /* May trigger different comparison logic */
    unsigned int uint_val = vu2 + i;
    unsigned _Accum uacc = (unsigned _Accum)uint_val * 0.0001uk;
    results[3] = (_Accum)uacc;
    
    /* Prevent dead code elimination */
    consume_result(results, sizeof(results));
    
    /* Return hash of results to ensure all computations matter */
    int hash = 0;
    for (int i = 0; i < 16; i++) {
        /* Access results in a way that prevents optimization */
        hash ^= *((int*)&results[i]);
        hash ^= *(((int*)&results[i]) + 1);
    }
    
    return hash & 0xFF;  /* Return non-zero to indicate execution */
}
