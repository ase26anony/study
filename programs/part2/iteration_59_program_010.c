/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Force GCC to process fixed-point arithmetic in middle-end */
volatile long _Fract volatile_source = 0x7FFFFFFFlr; /* Near maximum */

/* Function with fixed-point operations that may overflow */
static short _Fract process_fixed(short _Fract a, short _Fract b, int shift) {
    /* Intermediate multiplication with potential overflow */
    long _Fract temp = (long _Fract)a * (long _Fract)b;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    temp = temp << shift;
    
    /* Convert back with potential saturation */
    short _Fract result = (short _Fract)temp;
    
    return result;
}

/* Function using _Accum types with shifting */
static _Accum process_accum(_Accum val, int iterations) {
    _Accum result = val;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiplication that may exceed bounds */
        result = result * 1.5k;
        
        /* Left shift on fixed-point */
        result = result << 1;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Mixed-type operations to trigger promotions */
static long _Fract mixed_operations(unsigned short _Fract a, 
                                    long _Fract b, 
                                    int scale) {
    /* Integer promotion with fixed-point */
    long _Fract temp = a * 256;  /* Integer promotion */
    
    /* Complex expression */
    temp = (temp * b) << scale;
    
    /* Assignment to potentially narrower type */
    short _Fract narrowed = (short _Fract)temp;
    
    /* Return widened to avoid complete elimination */
    return (long _Fract)narrowed;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 5 + 1 : 3;
    
    /* Array of fixed-point values */
    short _Fract sf_array[8];
    _Accum accum_array[8];
    
    /* Initialize with pattern near bounds */
    for (int i = 0; i < 8; i++) {
        sf_array[i] = 0.8r + (i * 0.02r);
        accum_array[i] = 0.9k + (i * 0.1k);
    }
    
    long _Fract total = 0lr;
    
    /* Loop with varying operations to prevent compile-time evaluation */
    for (int i = 0; i < iterations; i++) {
        /* Read from volatile to prevent constant propagation */
        long _Fract base = volatile_source * 0.5lr;
        
        /* Operation 1: Multiplication with potential overflow */
        short _Fract sf_result = process_fixed(sf_array[i % 8], 
                                              sf_array[(i + 1) % 8], 
                                              i % 3);
        
        /* Operation 2: _Accum with shifting */
        _Accum accum_result = process_accum(accum_array[i % 8], i % 4);
        
        /* Operation 3: Mixed-type operations */
        long _Fract mixed_result = mixed_operations(
            (unsigned short _Fract)sf_array[i % 8],
            base,
            (i % 2) + 1
        );
        
        /* Combine results with another multiplication */
        total += (long _Fract)sf_result * (long _Fract)accum_result * mixed_result;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
        
        /* Modify array elements to create loop-carried dependencies */
        sf_array[i % 8] = sf_result * 0.95r;
        accum_array[i % 8] = accum_result * 0.8k;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", (double)total);
    
    /* Additional test cases in different scopes */
    {
        /* Test saturation with explicit narrower assignment */
        long _Fract large_val = 0.99lr;
        short _Fract narrowed = (short _Fract)(large_val * large_val);
        printf("Narrowed: %f\n", (double)narrowed);
    }
    
    {
        /* Test left shift boundary case */
        _Accum shift_test = 0.5k;
        for (int i = 0; i < 10; i++) {
            shift_test = shift_test << 1;
            asm volatile ("" : : : "memory");
        }
        printf("Shift test: %f\n", (double)shift_test);
    }
    
    return 0;
}
