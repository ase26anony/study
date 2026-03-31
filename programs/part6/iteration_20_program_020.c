/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O2 -ffixed-point -fdump-tree-vrp-details -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;
static volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, used))
static void consume_result(const void *p, int size) {
    asm volatile("" : : "r"(p), "r"(size) : "memory");
}

/* Helper to create complex data-dependent expressions */
__attribute__((noinline))
static signed _Accum compute_product(signed _Accum a, signed _Accum b, int shift) {
    /* This multiplication may overflow the fixed-point range */
    signed _Accum prod = a * b;
    
    /* Left shift can also cause overflow */
    if (shift > 0) {
        /* Simulate left shift via multiplication by power of two */
        signed _Accum multiplier = (signed _Accum)(1 << shift) / 256.0k;
        prod = prod * multiplier;
    }
    
    return prod;
}

__attribute__((noinline))
static unsigned _Fract compute_fractional(unsigned _Fract a, unsigned _Fract b) {
    /* Operations near the upper bound of unsigned fract */
    unsigned _Fract sum = a + b;
    
    /* Multiplication of values close to 1.0 */
    unsigned _Fract prod = a * b;
    
    /* Conditional that may trigger range analysis */
    return (sum > 0.999999ur) ? prod : sum;
}

__attribute__((noinline))
static long _Accum compute_long_accum(long _Accum a, long _Accum b) {
    /* Near-maximum values for long accum */
    long _Accum result;
    
    /* Complex expression that may overflow */
    result = a * b;
    
    /* Additional operation that could push beyond limits */
    if (result > 0.0lk) {
        result = result + (result * 0.0000001lk);
    }
    
    return result;
}

__attribute__((noinline))
static short _Fract handle_signed_fractional(short _Fract a, short _Fract b) {
    /* Operations near both bounds of signed fract */
    short _Fract diff = a - b;
    short _Fract avg = (a + b) / 2.0hr;
    
    /* This comparison may trigger range analysis */
    if (diff < -0.999hr || diff > 0.999hr) {
        return avg;
    }
    
    return diff * avg;
}

int main(void) {
    /* Array to accumulate results */
    signed _Accum results_acc[8];
    unsigned _Fract results_uf[8];
    long _Accum results_lacc[4];
    short _Fract results_sf[4];
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = vi;
    volatile unsigned int useed = vu;
    volatile long lseed = vl;
    
    /* Loop with varying values to force dynamic range analysis */
    for (int i = 0; i < 8; i++) {
        /* Create data-dependent fixed-point values */
        int int_val = seed + i * 100;
        unsigned int uint_val = useed + i * 50;
        long long_val = lseed + i * 200;
        
        /* Convert to various fixed-point types */
        signed _Accum a1 = (signed _Accum)int_val / 128.0k;
        signed _Accum a2 = (signed _Accum)(int_val + 64) / 128.0k;
        
        /* Near-maximum values for signed accum */
        if (i == 0) {
            a1 = 0.999999999k;  /* Very close to max */
            a2 = 0.999999999k;
        } else if (i == 1) {
            a1 = -0.999999999k; /* Very close to min */
            a2 = 0.5k;
        }
        
        /* Compute product that may overflow */
        results_acc[i] = compute_product(a1, a2, i % 3);
        
        /* Unsigned fract operations near 1.0 */
        unsigned _Fract uf1 = (unsigned _Fract)uint_val / 256.0ur;
        unsigned _Fract uf2 = (unsigned _Fract)(uint_val + 128) / 256.0ur;
        
        /* Force values near upper bound */
        if (i == 2) {
            uf1 = 0.9999999ur;
            uf2 = 0.0000001ur;
        } else if (i == 3) {
            uf1 = 0.999999ur;
            uf2 = 0.999999ur;
        }
        
        results_uf[i] = compute_fractional(uf1, uf2);
        
        /* Long accum for wider range */
        if (i < 4) {
            long _Accum la1 = (long _Accum)long_val / 32768.0lk;
            long _Accum la2 = (long _Accum)(long_val + 16384) / 32768.0lk;
            
            /* Extreme values for long accum */
            if (i == 0) {
                la1 = 0.999999999999999lk;
                la2 = 0.999999999999999lk;
            } else if (i == 1) {
                la1 = -0.999999999999999lk;
                la2 = 0.999999999999999lk;
            }
            
            results_lacc[i] = compute_long_accum(la1, la2);
        }
        
        /* Signed fract with values near both bounds */
        if (i < 4) {
            short _Fract sf1 = (short _Fract)(int_val % 256 - 128) / 128.0hr;
            short _Fract sf2 = (short _Fract)((int_val + 64) % 256 - 128) / 128.0hr;
            
            /* Edge cases for signed fract */
            if (i == 0) {
                sf1 = 0.999hr;
                sf2 = -0.999hr;
            } else if (i == 1) {
                sf1 = -0.999hr;
                sf2 = -0.999hr;
            }
            
            results_sf[i] = handle_signed_fractional(sf1, sf2);
        }
        
        /* Mix with integer promotions */
        if (i > 4) {
            int temp_int = (int)(results_acc[i] * 256.0k);
            results_acc[i] = (_Accum)temp_int * 0.00390625k; /* 1/256 */
        }
    }
    
    /* Additional overflow-triggering expressions */
    
    /* Left shift simulation via multiplication */
    signed _Accum max_accum = 0.999999999k;
    for (int shift = 1; shift <= 4; shift++) {
        /* This should trigger overflow check for left shift */
        signed _Accum shifted = max_accum * (signed _Accum)(1 << shift) / 256.0k;
        results_acc[shift % 8] += shifted;
    }
    
    /* Complex conditional with mixed types */
    unsigned _Fract max_ufract = 0.999999ur;
    for (int i = 0; i < 3; i++) {
        /* Operations that could wrap around */
        unsigned _Fract temp = max_ufract + (unsigned _Fract)i * 0.000001ur;
        results_uf[i] = (temp > 0.999999ur) ? 0.999999ur : temp;
    }
    
    /* Final consumption to prevent optimization */
    consume_result(results_acc, sizeof(results_acc));
    consume_result(results_uf, sizeof(results_uf));
    consume_result(results_lacc, sizeof(results_lacc));
    consume_result(results_sf, sizeof(results_sf));
    
    /* Return hash of results */
    int hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= (int)(results_acc[i] * 1000000.0k);
        hash ^= (int)(results_uf[i] * 1000000.0ur);
        if (i < 4) {
            hash ^= (int)(results_lacc[i] * 1000000.0lk);
            hash ^= (int)(results_sf[i] * 1000000.0hr);
        }
    }
    
    return hash & 0xFF;
}
