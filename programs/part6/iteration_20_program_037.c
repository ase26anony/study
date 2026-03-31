/* test_fixed_point_ranges.c
 * Designed to trigger GCC's fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -ftree-vrp -fwrapv -c test_fixed_point_ranges.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi = 0;
static volatile unsigned int vu = 0;
static volatile long vl = 0;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
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
    /* Array to accumulate results and prevent optimization */
    fract_t results[32] = {0};
    int result_idx = 0;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    int seed1 = vi;
    unsigned int seed2 = vu;
    long seed3 = vl;
    
    /* Test 1: Signed accumulative types near maximum range */
    for (int i = 0; i < 8; i++) {
        /* Use values that approach the maximum representable value */
        accum_t a = (accum_t)(0.999999k) + (accum_t)(i * 0.000001k);
        accum_t b = (accum_t)(0.999999k) - (accum_t)(i * 0.000001k);
        
        /* Multiplication that could overflow the fixed-point range */
        accum_t prod = a * b;
        
        /* Left shift to potentially exceed range */
        accum_t shifted = prod;
        for (int j = 0; j < 2; j++) {
            shifted = shifted + shifted; /* Simulate << 1 */
        }
        
        /* Complex expression with intermediate overflow checks */
        accum_t temp = (a + b) / (accum_t)2.0k;
        accum_t final_val = temp * shifted;
        
        /* Store result with potential saturation */
        if (final_val > (accum_t)0.999999k) {
            results[result_idx++] = (fract_t)1.0r;
        } else if (final_val < (accum_t)-0.999999k) {
            results[result_idx++] = (fract_t)-1.0r;
        } else {
            results[result_idx++] = (fract_t)final_val;
        }
    }
    
    /* Test 2: Unsigned fractional types near 1.0 */
    for (unsigned int i = 0; i < 8; i++) {
        ufract_t u = (ufract_t)0.9999999ur;
        ufract_t increment = (ufract_t)0.0000001ur;
        
        /* Operations that could wrap around */
        ufract_t sum = u + increment * (ufract_t)i;
        
        /* Multiplication that could exceed 1.0 */
        ufract_t scaled = sum * (ufract_t)0.9999999ur;
        
        /* Conditional assignment based on overflow check */
        ufract_t result;
        if (scaled > (ufract_t)0.9999999ur) {
            result = (ufract_t)1.0ur;
        } else {
            result = scaled;
        }
        
        results[result_idx++] = (fract_t)result;
    }
    
    /* Test 3: Signed fractional types near boundaries */
    for (int i = -4; i < 4; i++) {
        fract_t f = (fract_t)i * (fract_t)0.25r;
        
        /* Operations that push against -1.0 and 1.0 */
        fract_t squared = f * f;
        fract_t cubed = squared * f;
        
        /* Complex expression with multiple overflow points */
        fract_t expr = (fract_t)1.5r * squared - (fract_t)0.5r * cubed;
        
        /* Left shift simulation through multiplication */
        fract_t shifted_expr = expr * (fract_t)2.0r;
        
        /* Check if we're at the boundary */
        if (shifted_expr >= (fract_t)1.0r || shifted_expr <= (fract_t)-1.0r) {
            /* Force range analysis for boundary values */
            fract_t boundary_val = (shifted_expr > 0) ? (fract_t)1.0r : (fract_t)-1.0r;
            results[result_idx++] = boundary_val;
        } else {
            results[result_idx++] = shifted_expr;
        }
    }
    
    /* Test 4: Mixed integer and fixed-point with promotions */
    for (int i = 0; i < 8; i++) {
        int int_val = seed1 + i * 100;
        
        /* Convert integer to fixed-point with potential overflow */
        accum_t fixed_val = (accum_t)int_val * (accum_t)0.001k;
        
        /* Operations that depend on the sign */
        if (int_val > 0) {
            fixed_val = fixed_val * (accum_t)0.999k;
        } else {
            fixed_val = fixed_val * (accum_t)-0.999k;
        }
        
        /* Shift operation that could overflow */
        accum_t doubled = fixed_val + fixed_val;
        
        /* Store with saturation check */
        if (doubled > (accum_t)0.999999k) {
            results[result_idx++] = (fract_t)1.0r;
        } else if (doubled < (accum_t)-0.999999k) {
            results[result_idx++] = (fract_t)-1.0r;
        } else {
            results[result_idx++] = (fract_t)doubled;
        }
    }
    
    /* Test 5: Long accum types with extreme values */
    laccum_t la = (laccum_t)0.999999999999k;
    laccum_t lb = (laccum_t)0.999999999999k;
    
    /* This multiplication mathematically exceeds the fixed-point range */
    laccum_t lprod = la * lb;
    
    /* Multiple shifts to force overflow analysis */
    laccum_t lshifted = lprod;
    for (int i = 0; i < 3; i++) {
        lshifted = lshifted + lshifted; /* << 1 each iteration */
    }
    
    /* Complex expression that uses the overflowed value */
    fract_t final_result;
    if (lshifted > (laccum_t)0.5k) {
        final_result = (fract_t)0.5r;
    } else {
        final_result = (fract_t)lshifted;
    }
    results[result_idx++] = final_result;
    
    /* Ensure we don't exceed array bounds */
    if (result_idx > 31) result_idx = 31;
    
    /* Consume results to prevent optimization */
    consume(results, sizeof(results));
    
    /* Return hash of results for verification */
    int hash = 0;
    for (int i = 0; i < result_idx; i++) {
        /* Access as bytes to compute hash */
        char *bytes = (char *)&results[i];
        for (size_t j = 0; j < sizeof(fract_t); j++) {
            hash = (hash * 31) + bytes[j];
        }
    }
    
    return hash & 0xFF;
}
