/* fixed-point-test.c
 * Designed to trigger specific uncovered lines in GCC's fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -c fixed-point-test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Use volatile to prevent constant folding */
volatile short _Fract volatile_sf = 0.7r;
volatile long _Fract volatile_lf = 0.9lr;
volatile _Accum volatile_acc = 0.5k;
volatile long _Accum volatile_lacc = 0.8lk;

/* Function with fixed-point parameters to force analysis */
static _Accum process_fixed_point(short _Fract a, _Accum b, int shift) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;  /* FIXED_MULT_P with different types */
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift - FIXED_LSHIFT_EXPR */
    _Accum temp2 = temp1 << shift;
    
    /* Another multiplication with potential overflow */
    long _Accum wide_result = (_Accum)volatile_lacc * temp2;
    
    /* Convert to narrower type - may trigger saturation check */
    _Accum final_result = wide_result;
    
    return final_result;
}

/* Another function focusing on _Fract operations */
static short _Fract fract_operations(long _Fract a, long _Fract b, int iterations) {
    short _Fract result = 0.0r;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiplication that could exceed short _Fract range */
        long _Fract product = a * b;
        
        /* Left shift on fixed-point */
        long _Fract shifted = product << 1;
        
        /* Convert to narrower type - may trigger bounds checking */
        result += shifted;
        
        /* Modify values to prevent complete optimization */
        a = a * 0.95lr;
        b = b * 1.05lr;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Test unsigned fixed-point types as well */
static unsigned short _Fract unsigned_operations(unsigned long _Fract a, 
                                                unsigned short _Fract b) {
    /* Multiple operations to create complex expression */
    unsigned long _Fract temp = a * a * b;
    
    /* Left shift */
    temp = temp << 2;
    
    /* Convert to narrower unsigned type */
    unsigned short _Fract result = temp;
    
    return result;
}

/* Main function with various fixed-point operations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? 3 : 5;
    
    /* Initialize with volatile values to prevent compile-time evaluation */
    short _Fract sf1 = volatile_sf;
    short _Fract sf2 = 0.8r;
    
    _Accum acc1 = volatile_acc;
    _Accum acc2 = 0.6k;
    
    long _Fract lf1 = volatile_lf;
    long _Fract lf2 = 0.95lr;
    
    /* Array of fixed-point values for loop processing */
    _Accum accum_array[4] = {0.1k, 0.2k, 0.3k, 0.4k};
    short _Fract fract_array[4] = {0.5r, 0.6r, 0.7r, 0.8r};
    
    _Accum total_result = 0.0k;
    
    /* Loop with varying operations to prevent optimization */
    for (int i = 0; i < iterations; i++) {
        /* Mix of different fixed-point operations */
        
        /* 1. Multiplication with potential overflow */
        _Accum mult_result = acc1 * acc2;
        
        /* 2. Left shift operation */
        mult_result = mult_result << (i + 1);
        
        /* 3. Convert from wider type - may trigger saturation logic */
        short _Fract narrowed = mult_result;
        
        /* 4. Another multiplication chain */
        long _Accum wide_mult = (_Accum)volatile_lacc * acc1 * acc2;
        
        /* 5. Function call with fixed-point operations */
        _Accum func_result = process_fixed_point(sf1, acc1, i);
        
        /* 6. More complex expression with array elements */
        accum_array[i % 4] = accum_array[i % 4] * fract_array[i % 4] * 2.0k;
        
        /* 7. Left shift on array element */
        accum_array[i % 4] = accum_array[i % 4] << 1;
        
        /* 8. Convert to different fixed-point type */
        long _Fract temp_lf = accum_array[i % 4];
        
        /* 9. Call _Fract specific function */
        short _Fract fract_result = fract_operations(lf1, lf2, 2);
        
        /* Combine results */
        total_result += mult_result + func_result + accum_array[i % 4];
        
        /* Modify values for next iteration */
        acc1 = acc1 * 0.9k;
        acc2 = acc2 * 1.1k;
        sf1 = sf1 * 0.85r;
        
        /* Memory barrier to keep operations separate */
        asm volatile ("" : : : "memory");
    }
    
    /* Test unsigned fixed-point */
    unsigned long _Fract ulf1 = 0.9ulr;
    unsigned short _Fract usf1 = 0.7ur;
    unsigned short _Fract us_result = unsigned_operations(ulf1, usf1);
    
    /* Use results to prevent dead code elimination */
    volatile _Accum sink = total_result;
    volatile short _Fract sink2 = us_result;
    
    /* Print something to ensure code isn't optimized away entirely */
    if (sink != 0.0k) {
        printf("Result: %d\n", (int)(total_result * 100.0k));
    }
    
    return 0;
}
