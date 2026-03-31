/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O2 -ffixed-point -fdump-tree-vrp-details -c test_fixed_point_ranges.c
 */

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

/* Fixed-point type declarations */
typedef _Fract sfract_t;
typedef unsigned _Fract ufract_t;
typedef _Accum saccum_t;
typedef unsigned _Accum uaccum_t;
typedef long _Accum lsaccum_t;
typedef long _Fract lsfract_t;
typedef short _Fract hsfract_t;
typedef short _Accum hsaccum_t;

int main(void) {
    /* Array to accumulate results */
    saccum_t results[32] = {0};
    int result_idx = 0;
    
    /* Initialize with volatile values to prevent constant propagation */
    volatile int seed = vi1;
    volatile unsigned int useed = vu1;
    
    /* Loop with varying values to force range analysis */
    for (int i = 0; i < 16; i++) {
        /* Vary the seed values */
        int signed_seed = seed + i * vi2;
        unsigned int unsigned_seed = useed - i * vu2;
        
        /* 1. Test signed accumulative types near maximum range */
        {
            /* Create values near max representable */
            saccum_t a = (saccum_t)(0.999999999k);
            saccum_t b = (saccum_t)(0.999999999k);
            
            /* Force range analysis by using volatile-dependent scaling */
            saccum_t scale = (saccum_t)(signed_seed * 0.000000001k);
            a = a - scale;
            b = b - scale;
            
            /* Multiplication that could overflow */
            saccum_t product = a * b;
            results[result_idx++] = product;
            
            /* Left shift operation on accum */
            saccum_t shifted = product << (signed_seed & 0x3);
            results[result_idx++] = shifted;
        }
        
        /* 2. Test long accum with extreme values */
        {
            lsaccum_t la = (lsaccum_t)(0.999999999999999k);
            lsaccum_t lb = (lsaccum_t)(0.999999999999999k);
            
            /* Complex expression to force range computation */
            lsaccum_t temp = la * lb;
            temp = temp << ((unsigned_seed & 0x7) + 1);
            
            /* Cast to smaller type - may trigger range check */
            saccum_t converted = (saccum_t)temp;
            results[result_idx++] = converted;
        }
        
        /* 3. Test unsigned fract types near 1.0 */
        {
            ufract_t uf = (ufract_t)(0.9999999ur);
            ufract_t tiny = (ufract_t)(0.0000001ur);
            
            /* Addition that could wrap */
            ufract_t sum = uf + tiny;
            
            /* Multiplication that could overflow */
            ufract_t product = uf * uf;
            
            /* Mix with integer promotion */
            int int_val = (int)(sum * 256);
            ufract_t restored = (ufract_t)(int_val / 256);
            
            results[result_idx++] = (saccum_t)product;
            results[result_idx++] = (saccum_t)restored;
        }
        
        /* 4. Test signed fract types near boundaries */
        {
            sfract_t sf_pos = (sfract_t)(0.9999999r);
            sfract_t sf_neg = (sfract_t)(-0.9999999r);
            
            /* Operations that push boundaries */
            sfract_t diff = sf_pos - sf_neg;  /* Could approach 2.0 */
            sfract_t sum = sf_pos + sf_pos;   /* Could overflow positive */
            sfract_t neg_sum = sf_neg + sf_neg; /* Could overflow negative */
            
            /* Conditional based on overflow-like check */
            sfract_t selected = (diff > (sfract_t)(1.9r)) ? sf_pos : sf_neg;
            
            results[result_idx++] = (saccum_t)diff;
            results[result_idx++] = (saccum_t)selected;
        }
        
        /* 5. Test short fixed-point types */
        {
            hsaccum_t hsa = (hsaccum_t)(0.99999k);
            hsfract_t hsf = (hsfract_t)(0.99999r);
            
            /* Multiplication with different types */
            hsaccum_t mixed = hsa * (hsaccum_t)hsf;
            
            /* Shift operation */
            hsaccum_t shifted = mixed << ((signed_seed & 0x1) + 1);
            
            results[result_idx++] = (saccum_t)shifted;
        }
        
        /* 6. Test unsigned accum types */
        {
            uaccum_t ua = (uaccum_t)(0.999999999uk);
            uaccum_t ub = (uaccum_t)(0.999999999uk);
            
            /* Operations near maximum */
            uaccum_t u_prod = ua * ub;
            uaccum_t u_shifted = u_prod << 1;
            
            /* Convert to signed - may trigger range analysis */
            saccum_t s_conv = (saccum_t)u_shifted;
            
            results[result_idx++] = s_conv;
        }
        
        /* 7. Complex expression mixing types and operations */
        {
            /* Start with value that depends on volatile */
            saccum_t base = (saccum_t)((signed_seed & 0xFF) * 0.00390625k); /* 1/256 */
            
            /* Chain of operations */
            saccum_t step1 = base * (saccum_t)(0.9999999k);
            saccum_t step2 = step1 << ((unsigned_seed & 0x3) + 1);
            saccum_t step3 = step2 * step2;
            saccum_t step4 = step3 >> 1;
            
            /* Final conditional assignment */
            saccum_t final = (step4 > (saccum_t)(0.5k)) ? 
                           step4 * (saccum_t)(1.5k) : 
                           step4 * (saccum_t)(0.5k);
            
            results[result_idx++] = final;
        }
        
        /* 8. Explicit overflow check simulation */
        {
            lsaccum_t large = (lsaccum_t)(0.999999999999k);
            
            /* Create a value that might exceed max when shifted */
            int shift_amount = 2 + (i & 0x1);
            lsaccum_t shifted_large = large << shift_amount;
            
            /* Check if we're at maximum (simulating overflow check) */
            lsaccum_t max_val = (lsaccum_t)(0.999999999999k);
            int is_max = (shifted_large >= max_val);
            
            /* Use the result */
            results[result_idx++] = is_max ? (saccum_t)(0.5k) : (saccum_t)(-0.5k);
        }
    }
    
    /* Ensure we don't exceed array bounds */
    if (result_idx > 32) result_idx = 32;
    
    /* Consume results to prevent elimination */
    consume(results, sizeof(results));
    
    /* Create a simple hash of results for return value */
    int hash = 0;
    for (int i = 0; i < result_idx; i++) {
        /* Access as bytes to create hash */
        char *bytes = (char *)&results[i];
        for (size_t j = 0; j < sizeof(saccum_t); j++) {
            hash = (hash * 31) + bytes[j];
        }
    }
    
    return hash & 0xFF;
}
