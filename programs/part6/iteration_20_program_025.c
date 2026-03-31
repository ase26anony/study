/* Fixed-point range analysis test targeting GCC's fixed-value.cc uncovered lines */
/* Compile with: gcc -O3 -ffixed-point -ftree-vrp -fwrapv -fdump-tree-vrp-details -c test.c */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
volatile int vi1 = 1;
volatile int vi2 = -1;
volatile int vi3 = 0;
volatile unsigned int vu1 = 0xFFFFFFFF;
volatile unsigned int vu2 = 1;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, used))
static void consume(void *p, int size) {
    volatile char *cp = (volatile char *)p;
    for (int i = 0; i < size; i++) {
        cp[i];
    }
}

/* Fixed-point type definitions */
typedef _Fract sfract_t;
typedef unsigned _Fract ufract_t;
typedef _Accum saccum_t;
typedef unsigned _Accum uaccum_t;
typedef long _Accum lsaccum_t;
typedef long _Fract lsfract_t;
typedef unsigned long _Fract lufract_t;
typedef short _Fract ssfract_t;
typedef unsigned short _Fract usfract_t;

int main(void) {
    /* Array to accumulate results */
    saccum_t results[32] = {0};
    int result_idx = 0;
    
    /* Seed values near boundaries */
    volatile saccum_t near_max_acc = 0.999999999k;
    volatile saccum_t near_min_acc = -0.999999999k;
    volatile uaccum_t near_max_uacc = 0.999999999uk;
    volatile lsaccum_t near_max_lsacc = 0.999999999999999lk;
    
    volatile sfract_t near_max_fract = 0.9999999r;
    volatile sfract_t near_min_fract = -0.9999999r;
    volatile ufract_t near_max_ufract = 0.9999999ur;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 16; i++) {
        /* Use volatile to prevent compile-time evaluation */
        int idx = vi1 + i;
        
        /* TEST 1: Signed accum multiplication near overflow */
        /* This should trigger max range check for signed accum */
        saccum_t a1 = near_max_acc;
        saccum_t b1 = (saccum_t)(0.9999999k + (idx * 0.000000001k));
        saccum_t prod1 = a1 * b1;  /* Product may exceed 1.0 */
        results[result_idx++] = prod1;
        
        /* TEST 2: Left shift of fixed-point (converted to integer shift) */
        /* This often triggers the specific uncovered comparison */
        sfract_t f1 = near_max_fract;
        int shift1 = vi2 + 2;  /* Could be 1 or other small positive */
        /* Simulate left shift via multiplication by power of two */
        sfract_t shifted1 = f1 * (sfract_t)(1 << shift1);
        results[result_idx++] = (saccum_t)shifted1;
        
        /* TEST 3: Complex expression with mixed types */
        ufract_t uf1 = near_max_ufract;
        ufract_t uf2 = (ufract_t)(0.5ur + (i * 0.03125ur));
        ufract_t sum_uf = uf1 + uf2;  /* May overflow unsigned fract */
        results[result_idx++] = (saccum_t)sum_uf;
        
        /* TEST 4: Conditional with boundary values */
        /* Force compiler to analyze both branches */
        saccum_t cond_val = (i & 1) ? near_max_acc : near_min_acc;
        sfract_t scaled = (sfract_t)(cond_val * 0.5k);
        
        /* Left shift simulation that might trigger the uncovered code */
        int shift2 = (i % 3) + 1;
        sfract_t shifted2 = scaled * (sfract_t)(1 << shift2);
        results[result_idx++] = (saccum_t)shifted2;
        
        /* TEST 5: Long accum operations */
        lsaccum_t lsa1 = near_max_lsacc;
        lsaccum_t lsa2 = (lsaccum_t)(0.999999999999999lk - (i * 0.000000000000001lk));
        lsaccum_t lprod = lsa1 * lsa2;
        results[result_idx++] = (saccum_t)lprod;
        
        /* TEST 6: Conversion from integer with overflow potential */
        int int_val = vu1 - i * 1000000;
        saccum_t from_int = (saccum_t)int_val * 0.000001k;
        results[result_idx++] = from_int;
        
        /* TEST 7: Nested operations to create complex range */
        sfract_t temp1 = near_max_fract;
        sfract_t temp2 = (sfract_t)(temp1 * 0.75r);
        sfract_t temp3 = (sfract_t)(temp2 + 0.25r);
        /* This multiplication might overflow the fract range */
        sfract_t final_fract = temp3 * near_max_fract;
        results[result_idx++] = (saccum_t)final_fract;
        
        /* TEST 8: Unsigned accum near overflow */
        uaccum_t ua1 = near_max_uacc;
        uaccum_t ua2 = (uaccum_t)(0.999999999uk - (i * 0.000000001uk));
        uaccum_t uprod = ua1 * ua2;
        results[result_idx++] = (saccum_t)uprod;
    }
    
    /* Additional edge case: Direct boundary value test */
    /* These should directly exercise the range comparison logic */
    {
        /* Maximum representable _Accum */
        saccum_t max_acc = 0.999999999k;
        saccum_t min_acc = -0.999999999k;
        
        /* Operations that mathematically exceed boundaries */
        saccum_t overflow_test1 = max_acc * max_acc;
        saccum_t overflow_test2 = min_acc * min_acc;
        saccum_t overflow_test3 = max_acc * (saccum_t)2.0k;
        
        results[result_idx++] = overflow_test1;
        results[result_idx++] = overflow_test2;
        results[result_idx++] = overflow_test3;
        
        /* Shift-like operations via multiplication */
        for (int shift = 1; shift <= 4; shift++) {
            sfract_t f = near_max_fract;
            sfract_t shifted = f * (sfract_t)(1 << shift);
            results[result_idx++] = (saccum_t)shifted;
        }
    }
    
    /* Mix with short fract types */
    {
        ssfract_t short_max = 0.999r;
        usfract_t ushort_max = 0.999ur;
        
        /* Operations that might overflow short fract range */
        ssfract_t sprod = short_max * short_max;
        usfract_t usum = ushort_max + ushort_max * 0.5ur;
        
        results[result_idx++] = (saccum_t)sprod;
        results[result_idx++] = (saccum_t)usum;
    }
    
    /* Create a hash of results to return */
    int hash = 0;
    for (int i = 0; i < 32 && i < result_idx; i++) {
        /* Access the bit pattern */
        union {
            saccum_t f;
            int i;
        } u;
        u.f = results[i];
        hash ^= u.i ^ (i * 0x5A5A5A5A);
    }
    
    /* Prevent dead code elimination */
    consume(results, sizeof(results));
    
    return hash & 0xFF;
}
