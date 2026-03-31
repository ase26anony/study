/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x0.8p-7lr;  /* 0.0078125 in hex */

/* Function with fixed-point parameters to force analysis */
static short _Fract process_fixed(short _Fract a, _Accum b, int shift) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;                    /* Mixed-type multiplication */
    _Accum temp2 = temp1 << shift;           /* Left shift - FIXED_LSHIFT_EXPR */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Convert to narrower type with potential overflow */
    short _Fract result = (_Fract)temp2;
    
    /* Another shift on fract type */
    result = result << 1;                    /* Another FIXED_LSHIFT_EXPR */
    
    return result;
}

/* Another function focusing on fract operations */
static long _Fract fract_chain(long _Fract x, long _Fract y, unsigned int iterations) {
    long _Fract acc = 0x0.8p-1lr;  /* 0.5 */
    
    for (unsigned int i = 0; i < iterations; i++) {
        /* Operations that could exceed range */
        long _Fract prod = x * y;
        
        /* Left shift on fixed-point */
        prod = prod << (i % 4);
        
        /* Accumulate with potential overflow */
        acc = acc + prod;
        
        /* Modify inputs slightly */
        x = x * 0x0.Cp-1lr;  /* 0.75 */
        y = y * 0x0.Ap-1lr;  /* 0.625 */
        
        /* Memory barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return acc;
}

/* Test saturation with conversion */
static short _Fract test_saturation(_Accum wide_val) {
    /* This conversion should trigger saturation checks */
    short _Fract narrowed = wide_val;
    
    /* Additional operation on narrowed value */
    narrowed = narrowed * 0x0.8p-1r;  /* 0.5 */
    
    return narrowed;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 8 + 2 : 4;
    
    /* Initialize various fixed-point arrays */
    short _Fract sf_array[8];
    _Accum accum_array[8];
    long _Fract lf_array[8];
    
    /* Initialize with values that could cause overflow */
    for (int i = 0; i < 8; i++) {
        /* Use volatile source to prevent compile-time computation */
        sf_array[i] = (short _Fract)volatile_source * i;
        accum_array[i] = (_Accum)(i * 0x0.4p-1k);  /* 0.25 */
        lf_array[i] = 0x0.Fp-1lr * i;  /* 0.9375 */
    }
    
    short _Fract total_sf = 0x0.0p-1r;
    _Accum total_accum = 0x0.0p-1k;
    
    /* Main loop with various fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Test 1: Multiplication with potential overflow */
        short _Fract prod1 = sf_array[idx] * sf_array[(idx + 1) % 8];
        
        /* Test 2: Left shift on fixed-point */
        _Accum shifted = accum_array[idx] << (idx + 1);
        
        /* Test 3: Mixed-type operation */
        _Accum mixed = (_Accum)sf_array[idx] * lf_array[idx];
        
        /* Test 4: Function call with complex operations */
        short _Fract processed = process_fixed(
            sf_array[idx], 
            accum_array[idx], 
            idx
        );
        
        /* Test 5: Chain of fract operations */
        long _Fract chained = fract_chain(
            lf_array[idx],
            lf_array[(idx + 3) % 8],
            3
        );
        
        /* Test 6: Saturation conversion */
        short _Fract saturated = test_saturation(mixed << 2);
        
        /* Accumulate results (could overflow) */
        total_sf = total_sf + prod1 + processed + saturated;
        total_accum = total_accum + shifted + mixed;
        
        /* Modify array values for next iteration */
        sf_array[idx] = sf_array[idx] * 0x0.Cp-1r;  /* 0.75 */
        accum_array[idx] = accum_array[idx] << 1;   /* Left shift */
        
        /* Memory barrier to separate iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    volatile short _Fract dummy = total_sf;
    volatile _Accum dummy2 = total_accum;
    
    /* Print something to ensure execution */
    printf("Result: %hd (sf), %d (accum)\n", 
           (short)(dummy * 1000), 
           (int)(dummy2 * 1000));
    
    return 0;
}
