/* fixed-value-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -c fixed-value-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract volatile_sf = 0.5r;
volatile long _Fract volatile_lf = 0.9lr;
volatile _Accum volatile_acc = 0.7k;

/* Function with fixed-point operations that should trigger range checking */
static _Accum process_fixed_point(short _Fract a, long _Fract b, _Accum c, int shift) {
    /* Multiple operations to create complex fixed-value analysis */
    _Accum temp1, temp2, result;
    
    /* Multiplication that may overflow intermediate representation */
    temp1 = (_Accum)a * (_Accum)b;
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift - this is FIXED_LSHIFT_EXPR in GCC */
    temp2 = c << shift;
    
    /* Another multiplication with potential overflow */
    result = temp1 * temp2;
    
    /* Convert to narrower type to force range checking */
    short _Fract narrow_result = (short _Fract)result;
    
    /* Return as _Accum to avoid constant folding */
    return (_Accum)narrow_result;
}

/* Another function focusing on _Fract operations */
static long _Fract fract_operations(unsigned short _Fract a, 
                                   signed short _Fract b,
                                   int iterations) {
    long _Fract accum = 0.0lr;
    signed short _Fract temp;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiplication that may exceed short _Fract range */
        temp = a * b;
        
        /* Left shift on fixed-point (triggers FIXED_LSHIFT_EXPR) */
        temp = temp << 1;
        
        /* Convert to long _Fract with potential overflow checking */
        accum += (long _Fract)temp;
        
        /* Modify values to prevent loop unrolling */
        a = a * 0.95ur;
        b = b * 0.85r;
        
        /* Memory barrier to keep operations separate */
        asm volatile ("" : : : "memory");
    }
    
    return accum;
}

/* Main test function with various fixed-point operations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 5 + 1 : 3;
    
    /* Initialize with volatile sources */
    short _Fract sf1 = volatile_sf;
    short _Fract sf2 = 0.8r;
    long _Fract lf1 = volatile_lf;
    _Accum acc1 = volatile_acc;
    
    /* Array of fixed-point values */
    _Accum accum_array[4];
    short _Fract fract_array[4];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 4; i++) {
        accum_array[i] = (_Accum)i * 0.25k;
        fract_array[i] = (short _Fract)(i * 0.2r);
    }
    
    _Accum total_result = 0.0k;
    
    /* Loop with fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Operation 1: Multiplication with potential overflow */
        _Accum temp_acc = acc1 * (_Accum)sf1;
        
        /* Operation 2: Left shift (triggers FIXED_LSHIFT_EXPR) */
        temp_acc = temp_acc << (i % 3 + 1);
        
        /* Operation 3: Convert to narrower type - forces range check */
        short _Fract narrow = (short _Fract)temp_acc;
        
        /* Operation 4: Another multiplication */
        long _Fract wide_result = (long _Fract)narrow * lf1;
        
        /* Operation 5: Left shift on _Fract */
        wide_result = wide_result << 2;
        
        /* Convert back with potential saturation */
        _Accum final_acc = (_Accum)wide_result;
        
        /* Mix with array values */
        final_acc = final_acc * accum_array[i % 4];
        
        total_result += final_acc;
        
        /* Modify values to prevent optimization */
        sf1 = sf1 * 0.9r;
        acc1 = acc1 * 0.95k;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Call functions that perform fixed-point operations */
    _Accum func_result = process_fixed_point(
        (short _Fract)0.7r,
        (long _Fract)0.8lr,
        (_Accum)0.6k,
        iterations
    );
    
    long _Fract fract_result = fract_operations(
        (unsigned short _Fract)0.9ur,
        (signed short _Fract)0.75r,
        iterations
    );
    
    /* Mix all results */
    total_result = total_result + func_result + (_Accum)fract_result;
    
    /* Use result to prevent dead code elimination */
    if (total_result > 0.0k) {
        printf("Result: %ld\n", (long)(total_result * 1000));
    }
    
    return 0;
}

/* Additional test cases in separate functions to increase coverage */

/* Test unsigned fixed-point types */
static unsigned _Accum test_unsigned_fixed(unsigned short _Fract a, 
                                          unsigned _Accum b) {
    /* Multiplication that may overflow */
    unsigned _Accum result = (unsigned _Accum)a * b;
    
    /* Left shift */
    result = result << 1;
    
    /* Convert to narrower type */
    unsigned short _Fract narrow = (unsigned short _Fract)result;
    
    return (unsigned _Accum)narrow;
}

/* Test saturation with explicit conversions */
static short _Fract test_saturation(long _Fract a, long _Fract b) {
    /* This multiplication may exceed short _Fract range */
    long _Fract product = a * b;
    
    /* Explicit conversion should trigger saturation logic */
    short _Fract result = (short _Fract)product;
    
    return result;
}

/* Wrapper function to ensure functions are called */
void call_test_functions(void) {
    unsigned short _Fract usf = 0.8ur;
    unsigned _Accum uacc = 0.9uk;
    
    unsigned _Accum uresult = test_unsigned_fixed(usf, uacc);
    
    long _Fract lf1 = 0.99lr;
    long _Fract lf2 = 0.98lr;
    
    short _Fract sresult = test_saturation(lf1, lf2);
    
    /* Use results */
    asm volatile ("" : : "r" (uresult), "r" (sresult) : "memory");
}
