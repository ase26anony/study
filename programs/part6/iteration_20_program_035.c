/* test_fixed_range.c - Target GCC's fixed-point range analysis */
#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 100;
static volatile int vi4 = -100;

/* Dummy function to prevent optimization */
__attribute__((noinline, used))
static void consume(void *p, int size) {
    volatile char *cp = (volatile char *)p;
    for (int i = 0; i < size; i++) {
        cp[i];
    }
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[8] = {0};
    int result_index = 0;
    
    /* Seed values near boundaries */
    volatile _Accum near_max_acc = 0.999999999k;  /* Very close to max */
    volatile _Accum near_min_acc = -0.999999999k; /* Very close to min */
    volatile unsigned _Fract near_one_uf = 0.9999999ur;
    volatile _Fract near_one_f = 0.9999999r;
    volatile _Fract near_minus_one_f = -0.9999999r;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 4; i++) {
        /* Vary inputs based on volatile counters */
        int shift = vi1 + i;
        int scale = vi2 + i;
        
        /* TEST 1: Signed accumulative types near max boundary */
        /* This should trigger max range check in multiplication */
        _Accum a1 = near_max_acc * (_Accum)0.999k;
        _Accum a2 = a1 * (_Accum)(1.0k - 0.000000001k);
        
        /* Left shift that could overflow */
        long _Accum la1 = (long _Accum)a2;
        long _Accum la2 = la1 << shift;  /* May overflow */
        
        results[result_index++] = (_Accum)la2;
        
        /* TEST 2: Unsigned fractional types */
        unsigned _Fract uf1 = near_one_uf;
        /* Operation that could wrap to/from 1.0 */
        unsigned _Fract uf2 = uf1 + (unsigned _Fract)(0.0000001ur * i);
        /* Multiplication near boundary */
        unsigned _Fract uf3 = uf2 * 0.9999999ur;
        
        results[result_index++] = (_Accum)uf3;
        
        /* TEST 3: Signed fractional types with negative values */
        _Fract f1 = (i % 2) ? near_one_f : near_minus_one_f;
        /* Complex expression that could exceed [-1, 1] range */
        _Fract f2 = f1 * (_Fract)(0.5r + 0.5r * (i / 4.0r));
        /* Conditional that forces range analysis */
        _Fract f3 = (f2 > 0.9r) ? f2 * 1.1r : f2 * 0.9r;
        
        results[result_index++] = (_Accum)f3;
        
        /* TEST 4: Mixed integer/fixed-point with promotions */
        int int_val = vi3 + i * vi4;
        /* Cast and multiply - may overflow fixed-point range */
        _Accum mixed = (_Accum)int_val * 0.123456789k;
        /* Left shift in fixed-point domain */
        _Accum shifted = mixed * (1 << (shift % 4));
        
        results[result_index++] = shifted;
        
        /* TEST 5: Nested operations that approach boundaries */
        /* This creates intermediate values needing range checks */
        long _Fract lf1 = (long _Fract)near_one_f;
        long _Fract lf2 = lf1 * lf1;  /* Square of near-1 */
        long _Fract lf3 = lf2 * lf2;  /* Fourth power */
        /* Shift that could overflow */
        long _Fract lf4 = lf3 * (1 << (scale % 3));
        
        results[result_index++] = (_Accum)lf4;
        
        /* TEST 6: Explicit overflow check simulation */
        /* Force compiler to consider overflow in condition */
        _Accum test_val = near_max_acc;
        for (int j = 0; j < 2; j++) {
            test_val = test_val * 1.000000001k;
            /* This comparison may invoke range analysis */
            if (test_val > 0.999999999k) {
                test_val = 0.999999999k;
            }
        }
        results[result_index++] = test_val;
        
        /* TEST 7: Boundary case with all bits set */
        /* This corresponds to the max_s = max_s.zext(i_f_bits) logic */
        short _Accum sa1 = 0.9999999hk;
        short _Accum sa2 = sa1 / 0.0000001hk;  /* Large result */
        
        results[result_index++] = (_Accum)sa2;
        
        /* TEST 8: Minimum value handling */
        /* Corresponds to min_s = min_s.alshift() logic */
        _Accum min_test = near_min_acc;
        for (int j = 0; j < 2; j++) {
            min_test = min_test * (_Accum)(-1.000000001k);
            /* May trigger min range check */
            if (min_test < -0.999999999k) {
                min_test = -0.999999999k;
            }
        }
        results[result_index++] = min_test;
        
        /* Reset index if needed */
        if (result_index >= 8) result_index = 0;
    }
    
    /* TEST 9: Additional edge cases in separate scope */
    {
        /* Force range analysis for different fixed-point modes */
        volatile _Sat _Fract sat_f = 0.9999999r;
        sat_f = sat_f + 0.0000001r;  /* Should saturate */
        
        volatile unsigned short _Fract usf = 0.9999uhr;
        usf = usf * usf;  /* Square near 1.0 */
        
        results[0] += (_Accum)sat_f + (_Accum)usf;
    }
    
    /* Prevent dead code elimination */
    consume(results, sizeof(results));
    
    /* Return hash of results to ensure execution */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= *((int*)&results[i]);
    }
    return hash & 0xFF;
}
