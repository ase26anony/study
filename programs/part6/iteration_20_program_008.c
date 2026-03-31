/* test_fixed_point_ranges.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O2 -ffixed-point -fdump-tree-vrp-details -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 0;
static volatile unsigned int vu1 = 0x80000000U;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume_result(const void *ptr, int size) {
    volatile char sink;
    const char *p = (const char *)ptr;
    for (int i = 0; i < size; i++) {
        sink = p[i];
    }
}

/* Fixed-point type definitions */
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
    /* Array to accumulate results */
    fract_t results[32];
    int result_idx = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile fract_t vf1 = 0.999999r;  /* Near max signed fract */
    volatile fract_t vf2 = -0.999999r; /* Near min signed fract */
    volatile ufract_t vuf1 = 0.9999999ur; /* Near max unsigned fract */
    
    volatile accum_t vac1 = 0.999999999k;  /* Near max accum */
    volatile accum_t vac2 = -0.999999999k; /* Near min accum */
    volatile uaccum_t vuac1 = 0.9999999999uk; /* Near max uaccum */
    
    /* Test 1: Signed fract multiplication near boundaries */
    {
        fract_t a = vf1;
        fract_t b = vf1;
        /* This product mathematically exceeds 1.0, requiring range check */
        fract_t prod = a * b;
        results[result_idx++] = prod;
        
        /* Test with negative near-boundary values */
        fract_t c = vf2;
        fract_t d = 0.5r;
        fract_t prod2 = c * d;
        results[result_idx++] = prod2;
    }
    
    /* Test 2: Unsigned fract operations at overflow boundary */
    {
        ufract_t u1 = vuf1;
        ufract_t u2 = 0.0000001ur;
        /* Addition that would overflow beyond 1.0 */
        ufract_t sum = u1 + u2;
        results[result_idx++] = (fract_t)sum;
        
        /* Multiplication that could overflow */
        ufract_t u3 = 0.999999ur;
        ufract_t u4 = 0.999999ur;
        ufract_t prod = u3 * u4;
        results[result_idx++] = (fract_t)prod;
    }
    
    /* Test 3: Accum type with left shift (scaling) */
    {
        accum_t a1 = vac1;
        accum_t a2 = vac2;
        
        /* Left shift that could overflow */
        accum_t shifted1 = a1 << vi1;  /* Shift by 1 */
        results[result_idx++] = (fract_t)shifted1;
        
        /* Negative value left shift */
        accum_t shifted2 = a2 << 1;
        results[result_idx++] = (fract_t)shifted2;
        
        /* Multiplication of two near-max values */
        accum_t prod = a1 * a1;
        results[result_idx++] = (fract_t)prod;
    }
    
    /* Test 4: Mixed integer and fixed-point with promotions */
    {
        int i = vi1;
        unsigned int u = vu1;
        
        /* Integer to fixed-point conversion with multiplication */
        accum_t conv1 = (accum_t)i * 0.999999k;
        results[result_idx++] = (fract_t)conv1;
        
        /* Unsigned integer to unsigned accum */
        uaccum_t conv2 = (uaccum_t)u * 0.5uk;
        results[result_idx++] = (fract_t)conv2;
        
        /* Conditional expression with fixed-point */
        fract_t cond = (i > 0) ? vf1 : vf2;
        results[result_idx++] = cond;
    }
    
    /* Test 5: Loop with varying fixed-point values */
    {
        volatile int counter = vi3;
        accum_t acc = 0.5k;
        
        for (int i = 0; i < 8; i++) {
            /* Varying shift amount based on loop counter */
            int shift = (i + counter) & 3;
            
            /* Left shift that changes per iteration */
            accum_t temp = acc << shift;
            
            /* Multiplication with varying operand */
            accum_t multiplier = (accum_t)(i * 0.125k);
            accum_t product = temp * multiplier;
            
            results[result_idx++] = (fract_t)product;
            
            /* Update accumulator with near-boundary value */
            acc = (i & 1) ? vac1 : vac2;
        }
    }
    
    /* Test 6: Complex expression chain */
    {
        fract_t x = vf1;
        fract_t y = vf2;
        
        /* Chain of operations that could overflow at multiple points */
        fract_t step1 = x * 0.75r;
        fract_t step2 = step1 + 0.5r;
        fract_t step3 = step2 * y;
        fract_t step4 = step3 - 0.25r;
        fract_t step5 = step4 * 0.999r;
        
        results[result_idx++] = step5;
        
        /* Test with explicit overflow check simulation */
        accum_t large1 = vac1;
        accum_t large2 = 0.9999999k;
        accum_t large_prod = large1 * large2;
        
        /* Manually check if we're at max representable value */
        accum_t max_accum = 0.999999999k;
        if (large_prod > max_accum) {
            results[result_idx++] = (fract_t)max_accum;
        } else {
            results[result_idx++] = (fract_t)large_prod;
        }
    }
    
    /* Test 7: Long fixed-point types */
    {
        laccum_t la1 = 0.99999999999999999lk;
        laccum_t la2 = -0.99999999999999999lk;
        
        /* Operations on long accum types */
        laccum_t lprod = la1 * la1;
        results[result_idx++] = (fract_t)lprod;
        
        laccum_t lsum = la1 + la2;
        results[result_idx++] = (fract_t)lsum;
        
        /* Left shift on long accum */
        laccum_t lshifted = la1 << 2;
        results[result_idx++] = (fract_t)lshifted;
    }
    
    /* Test 8: Short fixed-point types */
    {
        sfract_t sf1 = 0.999r;
        sfract_t sf2 = -0.999r;
        
        /* Operations that could overflow short fract */
        sfract_t sf_prod = sf1 * sf1;
        results[result_idx++] = (fract_t)sf_prod;
        
        sfract_t sf_sum = sf1 + 0.01r;
        results[result_idx++] = (fract_t)sf_sum;
    }
    
    /* Ensure we use all results to prevent elimination */
    consume_result(results, sizeof(results[0]) * result_idx);
    
    /* Create a simple hash of results for return value */
    int hash = 0;
    for (int i = 0; i < result_idx; i++) {
        /* Access as bytes to avoid type issues */
        char *bytes = (char *)&results[i];
        for (size_t j = 0; j < sizeof(results[i]); j++) {
            hash = (hash * 31) + bytes[j];
        }
    }
    
    return hash & 0xFF;
}
