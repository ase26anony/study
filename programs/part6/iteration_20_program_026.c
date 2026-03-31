/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks in GCC
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
volatile int vi = 100;
volatile unsigned int vu = 200;
volatile long vl = 300;
volatile float vf = 0.5f;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char *cp = (volatile char *)p;
    for (int i = 0; i < size; i++) {
        cp[i];
    }
}

/* Helper to create complex data-dependent expressions */
__attribute__((noinline))
long _Accum compute_product(long _Accum a, long _Accum b, int shift) {
    /* This should trigger range analysis for multiplication */
    long _Accum temp = a * b;
    
    /* Mix with integer promotion - may trigger additional checks */
    int scaled = (int)temp * shift;
    
    /* More complex expression with potential overflow */
    long _Accum result = temp + (long _Accum)scaled * 0.0001lk;
    
    return result;
}

__attribute__((noinline))
unsigned _Fract compute_fractional(unsigned _Fract a, unsigned _Fract b) {
    /* Operations near the upper bound of unsigned fract */
    unsigned _Fract sum = a + b;
    unsigned _Fract prod = a * b;
    
    /* This addition could overflow/wrap */
    unsigned _Fract result = sum + prod;
    
    return result;
}

__attribute__((noinline))
_Accum compute_signed_range(_Accum a, _Accum b, int flag) {
    /* Complex conditional with fixed-point operations */
    _Accum temp = (flag > 0) ? a * 0.999999k : b * -0.999999k;
    
    /* Chain operations that could overflow */
    _Accum scaled = temp * temp;
    
    /* Near-boundary operations */
    if (scaled > 0.9k) {
        scaled = scaled * 1.1k;
    } else if (scaled < -0.9k) {
        scaled = scaled * 1.1k;
    }
    
    return scaled;
}

int main(void) {
    /* Array to store results and prevent optimization */
    static long _Accum results_acc[10];
    static unsigned _Fract results_uf[10];
    static _Accum results_sa[10];
    static _Fract results_sf[10];
    
    /* Initialize with volatile to prevent constant propagation */
    volatile int seed1 = vi;
    volatile unsigned int seed2 = vu;
    volatile long seed3 = vl;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 10; i++) {
        /* Create values near boundaries of fixed-point types */
        
        /* 1. Long Accum - near maximum positive value */
        long _Accum la1 = 0.999999999lk;
        long _Accum la2 = 0.999999998lk + (long _Accum)(i * 0.000000001lk);
        
        /* This multiplication should trigger overflow check */
        results_acc[i] = compute_product(la1, la2, seed1 + i);
        
        /* 2. Unsigned Fract - near 1.0 */
        unsigned _Fract uf1 = 0.9999999ur;
        unsigned _Fract uf2 = 0.0000001ur * (i + 1);
        
        /* Addition that could wrap */
        results_uf[i] = compute_fractional(uf1, uf2);
        
        /* 3. Signed Accum - boundary values */
        _Accum sa1 = (i % 2 == 0) ? 0.999999k : -0.999999k;
        _Accum sa2 = 0.5k + (_Accum)(seed2 % 100) * 0.01k;
        
        results_sa[i] = compute_signed_range(sa1, sa2, seed3 + i);
        
        /* 4. Signed Fract - using all ranges */
        _Fract sf1 = (i < 5) ? 0.999999r : -0.999999r;
        _Fract sf2 = (_Fract)((seed1 + i) % 100) * 0.01r;
        
        /* Complex expression with multiple operations */
        _Fract temp = sf1 * sf2;
        _Fract shifted = temp + temp;  /* Effectively << 1 */
        results_sf[i] = shifted * shifted;
        
        /* Mix with integer arithmetic to trigger promotions */
        int int_val = (int)(results_acc[i] * 1000);
        results_sf[i] = results_sf[i] + (_Fract)int_val * 0.001r;
    }
    
    /* Additional edge cases in a separate loop */
    for (int i = 0; i < 5; i++) {
        /* Test left-shift equivalent for fixed-point */
        short _Fract sf = (i % 2) ? 0.9999hr : -0.9999hr;
        
        /* Multiplication by power of two (simulating shift) */
        for (int j = 0; j < 4; j++) {
            sf = sf * 2.0hr;  /* This should trigger overflow checks */
        }
        
        /* Store to prevent elimination */
        results_sf[i] = results_sf[i] + (_Fract)sf;
    }
    
    /* Force conversion boundary checks */
    {
        /* Maximum and minimum values for various fixed-point types */
        long _Accum max_la = 0.999999999lk;
        long _Accum min_la = -0.999999999lk;
        
        unsigned _Fract max_uf = 0.9999999ur;
        
        _Accum max_sa = 0.999999k;
        _Accum min_sa = -0.999999k;
        
        /* Operations that should trigger the specific comparison logic */
        long _Accum test1 = max_la * max_la;  /* Should overflow */
        unsigned _Fract test2 = max_uf + max_uf * 0.5ur;
        _Accum test3 = min_sa * min_sa;  /* Should overflow positive */
        
        /* Use results to prevent dead code elimination */
        results_acc[0] = results_acc[0] + test1;
        results_uf[0] = results_uf[0] + test2;
        results_sa[0] = results_sa[0] + test3;
    }
    
    /* Consume all results to prevent optimization */
    consume(results_acc, sizeof(results_acc));
    consume(results_uf, sizeof(results_uf));
    consume(results_sa, sizeof(results_sa));
    consume(results_sf, sizeof(results_sf));
    
    /* Create a simple hash of results for return value */
    int hash = 0;
    for (int i = 0; i < 10; i++) {
        hash ^= (int)(results_acc[i] * 1000000);
        hash ^= (int)(results_uf[i] * 1000000);
        hash ^= (int)(results_sa[i] * 1000000);
        hash ^= (int)(results_sf[i] * 1000000);
    }
    
    return hash & 0xFF;
}
