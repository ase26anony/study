/* test_fixed_range.c
 * Designed to trigger fixed-point range analysis in GCC's fixed-value.cc
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -fdump-tree-vrp-details -c test_fixed_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 0;
static volatile unsigned int vu1 = 0xFFFFFFFF;
static volatile unsigned int vu2 = 0x7FFFFFFF;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume_result(const void *ptr, int size) {
    volatile char sink;
    const char *p = (const char *)ptr;
    for (int i = 0; i < size; i++) {
        sink = p[i];
    }
}

/* Helper to create complex data-dependent expressions */
__attribute__((noinline, noipa))
long _Accum compute_product(long _Accum a, long _Accum b, int shift) {
    /* Multi-step expression to create intermediate ranges */
    long _Accum temp = a * b;
    
    /* This multiplication near limits should trigger range checks */
    if (shift > 0) {
        /* Simulate left shift via multiplication - may overflow */
        for (int i = 0; i < shift; i++) {
            temp = temp * 2.0lk;
        }
    }
    
    /* Conditional that depends on overflow-like behavior */
    if (temp > 0.999999999lk) {
        return 0.5lk;
    } else if (temp < -0.999999999lk) {
        return -0.5lk;
    }
    
    return temp;
}

int main(void) {
    /* Array to accumulate results */
    long _Accum results[16];
    unsigned _Fract uf_results[16];
    _Accum s_results[16];
    
    /* Initialize with edge-case values */
    long _Accum la_max = 0.999999999lk;      /* Near maximum */
    long _Accum la_min = -0.999999999lk;     /* Near minimum */
    unsigned _Fract uf_max = 0.9999999ur;    /* Near 1.0 */
    _Accum s_mid = 0.5k;
    
    /* Seed values from volatile to prevent constant folding */
    int seed1 = vi1;
    int seed2 = vi2;
    unsigned int useed1 = vu1;
    unsigned int useed2 = vu2;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 16; i++) {
        /* Vary the input values based on iteration and volatile seeds */
        long _Accum a = la_max * (long _Accum)((i + seed1) % 10) / 10.0lk;
        long _Accum b = la_min * (long _Accum)((i + seed2) % 10) / 10.0lk;
        
        /* Operation that should trigger max/min range checks */
        results[i] = compute_product(a, b, i % 4);
        
        /* Unsigned fixed-point near overflow boundary */
        unsigned _Fract u1 = uf_max * (unsigned _Fract)((i + (useed1 & 0xF)) % 8) / 8.0ur;
        unsigned _Fract u2 = 0.0000001ur * (unsigned _Fract)(i + 1);
        
        /* Addition that could overflow unsigned range */
        uf_results[i] = u1 + u2;
        
        /* Signed _Accum with integer promotion */
        int int_val = (seed1 * i - seed2 * (i / 2)) % 100;
        _Accum promoted = (_Accum)int_val * 0.01k;
        
        /* Complex expression mixing operations */
        s_results[i] = promoted * s_mid;
        
        /* Left-shift simulation via multiplication - critical for overflow */
        if (i % 3 == 0) {
            s_results[i] = s_results[i] * 4.0k;  /* Could overflow */
        }
        
        /* Conditional assignment based on overflow-like check */
        if (results[i] > 0.999lk || results[i] < -0.999lk) {
            results[i] = results[i] * 0.5lk;
        }
        
        /* Mix with integer in conditional expression */
        int_val = (useed2 >> (i % 16)) & 1;
        uf_results[i] = int_val ? uf_results[i] : 0.5ur;
    }
    
    /* Additional edge-case tests outside loop */
    
    /* Test 1: Direct maximum boundary test */
    {
        long _Accum test1 = 0.999999999lk;
        long _Accum test2 = 0.999999999lk;
        long _Accum product = test1 * test2;  /* Should approach 1.0 */
        
        /* Force range analysis with conditional */
        if (product > 0.999999999lk) {
            results[0] = 0.0lk;
        }
    }
    
    /* Test 2: Minimum boundary with negative values */
    {
        _Accum neg_max = -0.999999k;
        _Accum neg_min = -0.999999k;
        _Accum neg_product = neg_max * neg_min;  /* Should approach 1.0 */
        
        /* This should trigger min_s calculations */
        if (neg_product < -0.999999k) {
            s_results[0] = 0.0k;
        }
    }
    
    /* Test 3: Shift-like operation that requires range extension */
    {
        short _Fract sf = 0.9999hr;
        /* Simulate left shift - may overflow short _Fract range */
        for (int j = 0; j < 3; j++) {
            sf = sf * 2.0hr;
        }
        /* Cast to larger type to see if range was tracked */
        _Accum from_sf = (_Accum)sf;
        results[1] = from_sf;
    }
    
    /* Test 4: Mixed-type expression forcing range comparisons */
    {
        long _Accum la = (long _Accum)(seed1) / 1000.0lk;
        _Accum sa = (_Accum)(seed2) / 100.0k;
        
        /* Operation between different fixed-point types */
        long _Accum mixed = la * (long _Accum)sa;
        
        /* Check against boundaries */
        if (mixed > 0.999lk || mixed < -0.999lk) {
            results[2] = mixed * 0.5lk;
        }
    }
    
    /* Prevent optimization of results */
    consume_result(results, sizeof(results));
    consume_result(uf_results, sizeof(uf_results));
    consume_result(s_results, sizeof(s_results));
    
    /* Create a simple hash to return */
    int hash = 0;
    for (int i = 0; i < 16; i++) {
        hash ^= *(int*)(&results[i]);
        hash ^= *(unsigned short*)(&uf_results[i]);
        hash ^= *(int*)(&s_results[i]);
    }
    
    return hash & 0xFF;
}
