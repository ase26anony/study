/* test_fixed_range.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 0;
static volatile unsigned int vu1 = 0xFFFFFFFF;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Fixed-point type definitions for clarity */
typedef short _Fract sfract_t;
typedef _Fract fract_t;
typedef long _Fract lfract_t;
typedef unsigned short _Fract usfract_t;
typedef unsigned _Fract ufract_t;
typedef unsigned long _Fract ulfract_t;
typedef short _Accum saccum_t;
typedef _Accum accum_t;
typedef long _Accum laccum_t;
typedef unsigned short _Accum usaccum_t;
typedef unsigned _Accum uaccum_t;
typedef unsigned long _Accum ulaccum_t;

int main(void) {
    /* Array to accumulate results and prevent optimization */
    accum_t results[32];
    int result_idx = 0;
    
    /* Initialize with volatile to prevent constant propagation */
    volatile fract_t f1 = 0.999999r;  /* Near max for signed fract */
    volatile fract_t f2 = -0.999999r; /* Near min for signed fract */
    volatile ufract_t uf1 = 0.9999999ur; /* Near max for unsigned fract */
    
    volatile accum_t a1 = 0.999999999k;  /* Near max for signed accum */
    volatile accum_t a2 = -0.999999999k; /* Near min for signed accum */
    volatile uaccum_t ua1 = 0.9999999999uk; /* Near max for unsigned accum */
    
    volatile laccum_t la1 = 0.999999999999999k;  /* Near max for long accum */
    volatile laccum_t la2 = -0.999999999999999k; /* Near min for long accum */
    
    /* Loop with varying conditions to force range analysis */
    for (int i = 0; i < 16; i++) {
        /* Use volatile index to prevent loop unrolling from simplifying */
        volatile int idx = i;
        
        /* TEST 1: Multiplication near overflow boundaries */
        /* This should trigger max range checks for signed accum */
        accum_t temp1 = a1 * (accum_t)(0.9k + (accum_t)(idx * 0.0001k));
        results[result_idx++] = temp1;
        
        /* TEST 2: Left shift operations (simulated via multiplication) */
        /* Shifting fixed-point left requires range checking */
        fract_t temp2 = f1 * (fract_t)(1 << (idx % 4));  /* Scale up */
        results[result_idx++] = (accum_t)temp2;
        
        /* TEST 3: Complex expression with intermediate overflow potential */
        /* The compiler must track range through multiple operations */
        accum_t temp3 = a1 * a2;  /* Negative result, but range check needed */
        temp3 = temp3 * (accum_t)(vi1 ? 0.5k : -0.5k);
        results[result_idx++] = temp3;
        
        /* TEST 4: Unsigned near overflow with addition */
        uaccum_t temp4 = ua1 + (uaccum_t)(0.0000000001uk * idx);
        /* Cast to signed to potentially trigger different range checks */
        results[result_idx++] = (accum_t)temp4;
        
        /* TEST 5: Conditional expressions with boundary values */
        /* Force VRP to analyze both branches */
        accum_t temp5 = (idx & 1) ? a1 : a2;
        temp5 = temp5 * (accum_t)((idx & 2) ? 0.999k : 1.001k);
        results[result_idx++] = temp5;
        
        /* TEST 6: Integer to fixed-point conversion with scaling */
        /* This requires range analysis for the conversion */
        int int_val = (vi2 + idx) * 1000;
        accum_t temp6 = (accum_t)int_val * 0.001k;
        results[result_idx++] = temp6;
        
        /* TEST 7: Long accum operations - wider ranges */
        laccum_t temp7 = la1 * (laccum_t)(0.999999999999k + 
                         (laccum_t)(idx * 0.000000000001k));
        results[result_idx++] = (accum_t)(temp7 >> 16); /* Downscale */
        
        /* TEST 8: Fract type with near-1.0 values */
        /* Multiplication of two near-1.0 values stays in range but tests logic */
        fract_t temp8 = f1 * (fract_t)(0.999r - (fract_t)(idx * 0.0001r));
        results[result_idx++] = (accum_t)temp8;
        
        /* TEST 9: Explicit overflow check pattern */
        /* Simulate overflow check the compiler might analyze */
        accum_t test_val = a1 * (accum_t)1.000000001k;
        if (test_val > 0.999999999k) {
            /* This branch should be taken if overflow wraps */
            results[result_idx++] = (accum_t)-0.5k;
        } else {
            results[result_idx++] = test_val;
        }
        
        /* TEST 10: Mixed-type expressions forcing promotions */
        sfract_t sf1 = 0.9999hr;
        accum_t temp10 = (accum_t)sf1 * a1;
        results[result_idx++] = temp10;
    }
    
    /* Additional edge case: Direct maximum value operations */
    /* These are most likely to trigger the exact uncovered lines */
    {
        /* Construct values that should hit max/min representable values */
        volatile int scale = vi3 + 2;  /* Non-constant scale factor */
        
        /* Operation that mathematically exceeds fixed-point range */
        accum_t edge1 = a1 * (accum_t)(scale * 0.6k);
        results[result_idx++] = edge1;
        
        /* Another attempt with different scaling */
        accum_t edge2 = (accum_t)(0.999999999k * 1.000000001k);
        results[result_idx++] = edge2;
        
        /* Use all bits set pattern which might correspond to max_s = -1 */
        unsigned int all_ones = vu1;
        accum_t edge3 = (accum_t)((all_ones >> 16) * 0.0000152587890625k);
        results[result_idx++] = edge3;
    }
    
    /* Prevent dead code elimination */
    consume(results, sizeof(results));
    
    /* Return hash of results to ensure all computations are needed */
    int hash = 0;
    for (int i = 0; i < 32; i++) {
        /* Access as bytes to get consistent hash */
        char *bytes = (char *)&results[i];
        for (size_t j = 0; j < sizeof(accum_t); j++) {
            hash = (hash * 31) + bytes[j];
        }
    }
    
    return hash & 0xFF;
}
