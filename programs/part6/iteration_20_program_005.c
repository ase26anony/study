/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -c test_fixed_point_ranges.c -o test.o
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;
static volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char *cp = (volatile char *)p;
    for (int i = 0; i < size; i++) {
        cp[i];
    }
}

/* Fixed-point type definitions */
typedef _Fract fr_t;
typedef _Accum ac_t;
typedef long _Accum lac_t;
typedef unsigned _Fract ufr_t;
typedef unsigned _Accum uac_t;
typedef short _Fract sfr_t;
typedef short _Accum sac_t;

/* Array to store results */
static fr_t fr_results[16];
static ac_t ac_results[16];
static lac_t lac_results[16];
static ufr_t ufr_results[16];
static int result_index = 0;

int main(void) {
    /* Initialize with volatile seeds to prevent compile-time evaluation */
    int seed1 = vi;
    int seed2 = vu;
    long seed3 = vl;
    
    /* Force the compiler to consider various fixed-point ranges */
    for (int i = 0; i < 16; i++) {
        /* Vary inputs to force range analysis */
        int idx = (seed1 + i) & 0xF;
        
        /* 1. Test signed _Accum near maximum range */
        ac_t a1 = 0.999999999k;  /* Very close to max */
        ac_t a2 = 0.999999999k;
        
        /* Multiplication that would overflow the fixed-point range */
        ac_t prod = a1 * a2;
        ac_results[idx] = prod;
        
        /* 2. Test long _Accum with left shift simulation */
        lac_t la1 = 0.999999999999999k;
        lac_t la2 = 0.999999999999999k;
        
        /* Multi-step expression to create intermediate ranges */
        lac_t temp = la1 * la2;
        /* Simulate left shift by multiplying by power of two */
        lac_t shifted = temp * 2.0k;
        lac_results[idx] = shifted;
        
        /* 3. Test unsigned _Fract near 1.0 */
        ufr_t u1 = 0.9999999ur;
        ufr_t u2 = 0.0000001ur;
        
        /* Operation that could wrap around */
        ufr_t sum = u1 + u2;
        ufr_results[idx] = sum;
        
        /* 4. Test signed _Fract with values near -1.0 and 1.0 */
        fr_t f1 = (i & 1) ? 0.9999999r : -0.9999999r;
        fr_t f2 = 0.0000001r;
        
        /* Complex expression with conditional */
        fr_t f_result = (f1 > 0.5r) ? (f1 * f2) : (f1 / f2);
        fr_results[idx] = f_result;
        
        /* 5. Test with integer promotions and casts */
        int int_val = seed2 + i;
        ac_t from_int = (ac_t)int_val * 0.5k;
        ac_results[(idx + 1) & 0xF] = from_int;
        
        /* 6. Test short _Accum with overflow potential */
        sac_t sa1 = 0.999999k;
        sac_t sa2 = 0.999999k;
        sac_t sa_prod = sa1 * sa2;
        /* Cast to larger type to force range analysis */
        ac_results[(idx + 2) & 0xF] = (ac_t)sa_prod;
        
        /* 7. Create data-dependent expression that varies per iteration */
        fr_t base = (fr_t)((i - 8) * 0.125r);
        fr_t scaled = base * base * 4.0r;
        fr_results[(idx + 1) & 0xF] = scaled;
        
        /* 8. Test boundary case with explicit overflow check simulation */
        uac_t ua_max = 0.999999999uk;
        uac_t ua_inc = 0.000000001uk;
        uac_t ua_test = ua_max + ua_inc;
        /* Force conditional that compares against max */
        if (ua_test > 0.999999999uk) {
            ufr_results[(idx + 1) & 0xF] = 0.5ur;
        }
        
        /* 9. Test negative range for signed types */
        lac_t negative = -0.999999999999999k;
        lac_t neg_squared = negative * negative;
        lac_results[(idx + 1) & 0xF] = neg_squared;
        
        /* 10. Mixed-type expression to force conversions */
        fr_t mixed = (fr_t)((ac_t)0.9999999k * (lac_t)0.999999999999999k);
        fr_results[(idx + 2) & 0xF] = mixed;
        
        /* Update seeds to vary next iteration */
        seed1 = seed1 * 1103515245 + 12345;
        seed2 = seed2 * 1664525 + 1013904223;
    }
    
    /* Additional test cases outside loop for specific edge cases */
    
    /* Test minimum representable value logic */
    {
        fr_t min_test = -0.9999999r;
        fr_t min_result = min_test * min_test;
        fr_results[15] = min_result;
    }
    
    /* Test with volatile intermediate to prevent optimization */
    volatile ac_t vol_accum = 0.999999999k;
    ac_t volatile_test = vol_accum * vol_accum;
    ac_results[15] = volatile_test;
    
    /* Test shift-like operation through multiplication */
    {
        uac_t ua = 0.999999999uk;
        /* Multiply by slightly more than 1 to simulate overflow */
        uac_t ua_overflow = ua * 1.000000001uk;
        /* Cast to force range check */
        ufr_results[15] = (ufr_t)ua_overflow;
    }
    
    /* Test the specific comparison pattern from uncovered lines */
    /* by creating values that would require max/min range computation */
    {
        /* These operations should trigger the range bounds calculation */
        lac_t large1 = 0.999999999999999k;
        lac_t large2 = 0.999999999999999k;
        lac_t large_prod = large1 * large2 * large1 * large2;
        lac_results[15] = large_prod;
    }
    
    /* Prevent dead code elimination */
    consume(fr_results, sizeof(fr_results));
    consume(ac_results, sizeof(ac_results));
    consume(lac_results, sizeof(lac_results));
    consume(ufr_results, sizeof(ufr_results));
    
    /* Create a simple hash of results to return */
    int hash = 0;
    for (int i = 0; i < 16; i++) {
        hash ^= *(int*)&fr_results[i];
        hash ^= *(int*)&ac_results[i];
        hash ^= *(int*)&lac_results[i] ^ (*(int*)&lac_results[i] >> 32);
        hash ^= *(unsigned int*)&ufr_results[i];
    }
    
    return hash & 0xFF;
}
