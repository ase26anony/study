/* test_fixed_point_range.c
 * Designed to trigger fixed-point range analysis overflow checks
 * Compile with: gcc -O3 -ffixed-point -fwrapv -ftree-vrp -c test_fixed_point_range.c
 */

#include <stdint.h>

/* Prevent constant folding with volatile inputs */
static volatile int vi1 = 1;
static volatile int vi2 = -1;
static volatile int vi3 = 100;
static volatile int vi4 = -100;

/* Dummy function to prevent dead code elimination */
__attribute__((noinline, noipa))
void consume(void *p, int size) {
    volatile char sink;
    char *cp = (char *)p;
    for (int i = 0; i < size; i++) {
        sink = cp[i];
    }
}

/* Hash function to return non-deterministic result */
__attribute__((noinline))
int hash_result(const void *data, int size) {
    const unsigned char *p = (const unsigned char *)data;
    int h = 0;
    for (int i = 0; i < size; i++) {
        h = (h * 31) + p[i];
    }
    return h;
}

int main(void) {
    /* Array to accumulate results */
    _Accum results[16];
    int result_idx = 0;
    
    /* Test various fixed-point types near their boundaries */
    
    /* 1. Signed accumulative types - push against max/min */
    _Accum a1 = 0.999999999k;  /* Very close to max */
    _Accum a2 = -0.999999999k; /* Very close to min */
    _Accum a3 = 0.5k;
    
    /* Force range analysis with data-dependent expressions */
    for (int i = 0; i < 4; i++) {
        volatile int scale = vi1 + i;
        
        /* Operations that could overflow */
        _Accum temp1 = a1 * (_Accum)scale;
        _Accum temp2 = a2 * (_Accum)(vi2 + i);
        
        /* Left shift simulation through multiplication */
        _Accum shifted1 = temp1 * 2.0k;  /* Effectively << 1 */
        _Accum shifted2 = temp2 * 4.0k;  /* Effectively << 2 */
        
        /* Store results */
        if (result_idx < 16) results[result_idx++] = shifted1;
        if (result_idx < 16) results[result_idx++] = shifted2;
    }
    
    /* 2. Long accum with extreme values */
    long _Accum la1 = 0.999999999999999k;
    long _Accum la2 = -0.999999999999999k;
    
    /* Multiplication that could overflow the range */
    for (int i = 0; i < 2; i++) {
        volatile long factor = vi3 + i * 50;
        long _Accum product = la1 * (long _Accum)factor;
        
        /* Additional shift-like operation */
        long _Accum shifted = product * 8.0k;  /* << 3 equivalent */
        
        if (result_idx < 16) results[result_idx++] = (_Accum)shifted;
    }
    
    /* 3. Unsigned fractional types - near 1.0 */
    unsigned _Fract uf1 = 0.9999999ur;
    unsigned _Fract uf2 = 0.0000001ur;
    
    /* Operations that could wrap around */
    for (int i = 0; i < 3; i++) {
        volatile unsigned increment = vi4 + 100 + i * 10;
        unsigned _Fract uf_temp = uf1;
        
        /* Multiple additions that could exceed 1.0 */
        for (int j = 0; j < (increment % 5) + 1; j++) {
            uf_temp = uf_temp + uf2;
        }
        
        /* Multiplication near overflow boundary */
        unsigned _Fract uf_prod = uf_temp * 0.9999998ur;
        
        if (result_idx < 16) results[result_idx++] = (_Accum)uf_prod;
    }
    
    /* 4. Signed fractional types - near -1.0 and 1.0 */
    _Fract sf1 = 0.9999999r;
    _Fract sf2 = -0.9999999r;
    
    /* Complex expression with conditional */
    for (int i = 0; i < 3; i++) {
        volatile int selector = vi1 + i;
        
        _Fract sf_result = (selector > 0) ? sf1 : sf2;
        
        /* Chain of operations */
        sf_result = sf_result * 0.5r;
        sf_result = sf_result + ((i % 2) ? 0.25r : -0.25r);
        sf_result = sf_result * 2.0r;  /* Could overflow */
        
        if (result_idx < 16) results[result_idx++] = (_Accum)sf_result;
    }
    
    /* 5. Short fixed-point types */
    short _Fract ssf1 = 0.9999r;
    short _Accum ssa1 = 0.99999k;
    
    /* Mixed-type operations with promotion */
    for (int i = 0; i < 2; i++) {
        volatile short scale = vi2 + i * 2;
        
        /* Convert to larger type, operate, convert back */
        _Accum temp = (_Accum)ssa1 * (_Accum)scale;
        short _Accum ssa_result = (short _Accum)temp;
        
        /* Additional operation that might overflow short range */
        ssa_result = ssa_result * 2.0k;
        
        if (result_idx < 16) results[result_idx++] = (_Accum)ssa_result;
    }
    
    /* 6. Explicit overflow check simulation */
    /* This should trigger the specific comparison logic */
    for (int i = 0; i < 2; i++) {
        volatile _Accum v1 = (vi3 > 50) ? 0.999999999k : 0.5k;
        volatile _Accum v2 = (vi4 < -50) ? -0.999999999k : -0.5k;
        
        /* Operations designed to hit max/min boundaries */
        _Accum test1 = v1 * 1.5k;  /* Could exceed 1.0 */
        _Accum test2 = v2 * 3.0k;  /* Could exceed -1.0 in magnitude */
        
        /* Chain operations to create complex range */
        _Accum combined = test1 + test2;
        combined = combined * 0.75k;
        
        /* Final operation that might trigger overflow check */
        _Accum final_op = combined * 2.0k;
        
        if (result_idx < 16) results[result_idx++] = final_op;
    }
    
    /* Fill remaining slots */
    while (result_idx < 16) {
        results[result_idx++] = 0.0k;
    }
    
    /* Prevent optimization */
    consume(results, sizeof(results));
    
    /* Return hash to make result observable */
    return hash_result(results, sizeof(results)) & 0xFF;
}
