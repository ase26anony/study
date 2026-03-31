/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Function to prevent constant folding */
static volatile int get_seed(void) {
    return 1;
}

/* Function with fixed-point operations that may overflow */
static short _Fract process_fract(short _Fract a, short _Fract b) {
    /* Multiplication that might exceed range */
    short _Fract result = a * b;
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift on fixed-point (FIXED_LSHIFT_EXPR) */
    /* Cast to _Accum for wider shift operation */
    _Accum temp = (_Accum)result;
    temp = temp << 2;
    
    asm volatile ("" : : : "memory");
    
    /* Convert back with potential overflow */
    return (short _Fract)temp;
}

/* Function with _Accum operations */
static _Accum process_accum(_Accum a, _Accum b) {
    /* Multiplication with wide intermediate */
    _Accum prod = a * b;
    
    asm volatile ("" : : : "memory");
    
    /* Left shift that may overflow */
    _Accum shifted = prod << 3;
    
    asm volatile ("" : : : "memory");
    
    /* Additional multiplication to create complex expression */
    return shifted * (_Accum)0.5k;
}

/* Main test function with loops and arrays */
int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations <= 0) iterations = 5;
    
    /* Initialize arrays of different fixed-point types */
    short _Fract sf_array[10];
    _Accum accum_array[10];
    long _Fract lf_array[10];
    
    /* Initialize with values that may cause overflow */
    for (int i = 0; i < 10; i++) {
        /* Use volatile to prevent compile-time evaluation */
        volatile int seed = get_seed();
        float frac = 0.5f + (seed * 0.1f * i);
        
        sf_array[i] = (short _Fract)frac;
        accum_array[i] = (_Accum)(frac * 2.0f);
        lf_array[i] = (long _Fract)frac;
    }
    
    /* Variables to accumulate results */
    short _Fract total_sf = 0.0r;
    _Accum total_accum = 0.0k;
    
    /* Perform fixed-point operations in loop */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 10;
        
        /* Fixed-point multiplication with potential overflow */
        short _Fract sf_result = sf_array[idx] * sf_array[(idx + 1) % 10];
        
        /* Assign to narrower type to force range checking */
        short _Fract narrowed = sf_result;
        
        /* Additional operation to create complex expression tree */
        narrowed = process_fract(narrowed, sf_array[(idx + 2) % 10]);
        
        /* Mix with integer promotion */
        int multiplier = (i + 1);
        _Accum promoted = (_Accum)narrowed * multiplier;
        
        asm volatile ("" : : : "memory");
        
        /* Left shift on fixed-point */
        promoted = promoted << (i % 4 + 1);
        
        /* Process _Accum operations */
        _Accum accum_result = process_accum(accum_array[idx], 
                                           accum_array[(idx + 3) % 10]);
        
        /* Convert between different fixed-point types */
        long _Fract wide_result = (long _Fract)accum_result * lf_array[idx];
        
        /* Narrow conversion that may overflow */
        short _Fract final_narrow = (short _Fract)wide_result;
        
        /* Accumulate results */
        total_sf += final_narrow;
        total_accum += accum_result;
        
        asm volatile ("" : : : "memory");
    }
    
    /* Additional test case specifically for saturation */
    {
        /* Create values near extremes */
        long _Fract a = 0.999999lr;
        long _Fract b = 0.999999lr;
        
        /* Multiplication that may overflow when converted to short _Fract */
        long _Fract product = a * b;
        
        /* Narrow conversion with potential saturation */
        short _Fract narrow_product = (short _Fract)product;
        
        /* Use the result to prevent dead code elimination */
        total_sf += narrow_product;
    }
    
    /* Test left shift on _Fract types directly */
    {
        _Accum val = 0.75k;
        
        /* Multiple shifts to create complex pattern */
        for (int i = 0; i < 3; i++) {
            val = val << 2;
            asm volatile ("" : : : "memory");
        }
        
        total_accum += val;
    }
    
    /* Print results to ensure code isn't optimized away */
    printf("Result sf: %f\n", (double)total_sf);
    printf("Result accum: %f\n", (double)total_accum);
    
    return 0;
}
