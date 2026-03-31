/* fixed-value-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-value-test.c -o fixed-value-test
 */

#include <stdio.h>
#include <stdint.h>

/* Use volatile to prevent constant folding */
volatile short _Fract v_sf = 0.5r;
volatile _Accum v_acc = 0.5k;
volatile long _Fract v_lf = 0.9lr;

/* Function with fixed-point parameters to force range checking */
static _Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* Mixed precision multiplication */
    _Accum temp2 = temp1 << 3;      /* Left shift - FIXED_LSHIFT_EXPR */
    
    /* Narrower assignment forcing saturation check */
    short _Fract narrow = (_Fract)(c * b);
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return temp2 + (_Accum)narrow;
}

/* Another function focusing on overflow boundaries */
static void test_overflow_bounds(int iterations) {
    /* Array of fixed-point values */
    _Accum arr[4] = {0.1k, 0.5k, 0.9k, -0.5k};
    short _Fract results[4];
    
    for (int i = 0; i < iterations && i < 4; i++) {
        /* Operations that could exceed mode bounds */
        _Accum shifted = arr[i] << (i + 2);  /* Variable shift amount */
        
        /* Multiplication with potential wide intermediate */
        _Accum multiplied = shifted * (_Accum)v_lf;
        
        /* Assign to narrower type - may trigger saturation */
        results[i] = (_Fract)multiplied;
        
        /* Another memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    volatile short _Fract dummy = 0r;
    for (int i = 0; i < 4; i++) {
        dummy += results[i];
    }
}

/* Main test function with various fixed-point operations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop non-constant */
    int iterations = (argc > 1) ? (argv[1][0] - '0') : 3;
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;
    
    /* Test 1: Mixed-type multiplications */
    printf("Test 1: Mixed-type multiplications\n");
    for (int i = 0; i < iterations; i++) {
        /* Varying shift amounts to explore different i_f_bits */
        int shift = (i % 4) + 1;
        
        /* Start with volatile source */
        _Accum base = v_acc;
        
        /* Left shift operation - directly triggers FIXED_LSHIFT_EXPR */
        _Accum shifted = base << shift;
        
        /* Multiplication that could overflow */
        _Accum product = shifted * (_Accum)v_sf * (_Accum)v_lf;
        
        /* Assign to potentially narrower type */
        short _Fract narrowed = (_Fract)product;
        
        /* Memory barrier between operations */
        asm volatile ("" : : : "memory");
        
        /* Use result */
        volatile short _Fract sink = narrowed;
        (void)sink;
    }
    
    /* Test 2: Function calls with fixed-point args */
    printf("Test 2: Function calls\n");
    short _Fract sf_arg = 0.8r;
    _Accum acc_arg = 0.7k;
    long _Fract lf_arg = 0.95lr;
    
    for (int i = 0; i < iterations; i++) {
        /* Modify arguments slightly each iteration */
        sf_arg += 0.05r;
        acc_arg -= 0.1k;
        
        _Accum result = process_fixed_point(sf_arg, acc_arg, lf_arg);
        
        /* Left shift the result */
        result = result << 2;
        
        /* Another memory barrier */
        asm volatile ("" : : : "memory");
        
        volatile _Accum sink2 = result;
        (void)sink2;
    }
    
    /* Test 3: Array operations with bounds checking */
    printf("Test 3: Array operations\n");
    test_overflow_bounds(iterations);
    
    /* Test 4: Edge case - values near saturation boundaries */
    printf("Test 4: Edge cases\n");
    {
        /* Values that might trigger max_r/max_s comparison */
        _Accum near_max = 0.999k;
        long _Fract near_one = 0.999999lr;
        
        for (int i = 0; i < iterations; i++) {
            /* Multiplication that could exceed 1.0 */
            _Accum product = near_max * (_Accum)near_one;
            
            /* Multiple shifts */
            product = product << 1;
            product = product << 1;
            
            /* Assign to short _Fract - likely needs saturation */
            short _Fract final = (_Fract)product;
            
            asm volatile ("" : : : "memory");
            
            volatile short _Fract sink3 = final;
            (void)sink3;
            
            /* Modify values for next iteration */
            near_max *= 0.9k;
            near_one *= 0.95lr;
        }
    }
    
    /* Test 5: Unsigned fixed-point types */
    printf("Test 5: Unsigned types\n");
    {
        unsigned short _Fract usf1 = 0.8ur;
        unsigned short _Fract usf2 = 0.9ur;
        
        for (int i = 0; i < iterations; i++) {
            /* Multiplication that could saturate */
            unsigned short _Fract usf_prod = usf1 * usf2;
            
            /* Simulate left shift via multiplication by power of 2 */
            usf_prod = usf_prod * 2.0ur;
            
            asm volatile ("" : : : "memory");
            
            volatile unsigned short _Fract sink4 = usf_prod;
            (void)sink4;
            
            usf1 += 0.1ur;
            if (usf1 > 1.0ur) usf1 = 0.1ur;
        }
    }
    
    printf("Tests completed (iterations: %d)\n", iterations);
    return 0;
}
