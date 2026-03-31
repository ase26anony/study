/* fixed-value-coverage.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -c fixed-value-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding with volatile */
volatile short _Fract v_sf = 0.5r;
volatile long _Fract v_lf = 0.9lr;
volatile _Accum v_acc = 0.5k;
volatile long _Accum v_lacc = 0.7lk;

/* Function to force fixed-point arithmetic with unknown values */
static _Accum process_fixed_point(short _Fract a, _Accum b, int shift) {
    /* Multiple operations to create wide intermediate results */
    _Accum temp1 = a * b;                    /* FIXED_MULT_P */
    _Accum temp2 = temp1 << shift;           /* FIXED_LSHIFT_EXPR - triggers bounds setup */
    
    /* Mix with integer promotion */
    long long int_prom = 256;
    _Accum temp3 = temp2 * int_prom;         /* Integer promotion */
    
    /* Force potential overflow by assigning to narrower type */
    short _Fract narrow_result = temp3;      /* May trigger saturation check */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return (_Accum)narrow_result;
}

/* Another function focusing on _Fract operations */
static long _Fract fract_operations(long _Fract x, long _Fract y, int iterations) {
    long _Fract result = 0.0lr;
    
    for (int i = 0; i < iterations; i++) {
        /* Operations that can overflow */
        long _Fract prod = x * y;            /* Multiplication */
        prod = prod << 1;                    /* Left shift - triggers the uncovered code */
        
        /* Chain operations for wider intermediate */
        prod = prod * prod;                  /* Square - can overflow */
        
        /* Assign to potentially overflowing narrower type */
        short _Fract narrowed = prod;        /* May trigger bounds checking */
        
        result += (long _Fract)narrowed;
        
        /* Slightly modify values to prevent complete optimization */
        x = x * 0.99lr;
        y = y * 1.01lr;
        
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Main function with loop-variant operations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop count non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Initialize arrays of fixed-point values */
    short _Fract sf_array[10];
    _Accum acc_array[10];
    long _Fract lf_array[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        sf_array[i] = (short _Fract)(i * 0.1r);
        acc_array[i] = (_Accum)(i * 0.2k);
        lf_array[i] = (long _Fract)(i * 0.05lr);
    }
    
    _Accum total_acc = 0.0k;
    long _Fract total_lf = 0.0lr;
    
    /* Main processing loop with various fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Read volatile to prevent compile-time evaluation */
        short _Fract sf1 = v_sf;
        _Accum acc1 = v_acc;
        
        /* Operation 1: Multiplication with potential overflow */
        _Accum mult_result = sf1 * acc1;     /* Mixed types */
        mult_result = mult_result << (i % 4); /* Left shift - key trigger */
        
        /* Operation 2: Process through function */
        _Accum func_result = process_fixed_point(sf_array[i % 10], 
                                                acc_array[i % 10], 
                                                i % 3);
        
        /* Operation 3: Fract-specific operations */
        long _Fract lf1 = v_lf;
        long _Fract lf2 = lf_array[i % 10];
        long _Fract fract_result = fract_operations(lf1, lf2, 2);
        
        /* Accumulate results with narrowing assignments */
        short _Fract narrowed = mult_result + func_result;
        total_acc += (_Accum)narrowed;       /* May trigger saturation logic */
        total_lf += fract_result;
        
        /* Modify array values slightly */
        sf_array[i % 10] = sf_array[i % 10] * 0.95r;
        acc_array[i % 10] = acc_array[i % 10] * 1.05k;
        
        asm volatile ("" : : : "memory");
    }
    
    /* Additional specific test cases targeting the uncovered condition */
    
    /* Case 1: Large left shift that could exceed bounds */
    {
        _Accum large_val = 0.9k;
        int large_shift = 10;  /* Enough to potentially overflow */
        _Accum shifted = large_val << large_shift;  /* Should trigger bounds check */
        total_acc += shifted;
    }
    
    /* Case 2: Multiplication near overflow boundaries */
    {
        long _Fract near_max1 = 0.999lr;
        long _Fract near_max2 = 0.999lr;
        long _Fract product = near_max1 * near_max2;  /* Close to 1.0 */
        product = product << 1;  /* Push it over the boundary */
        short _Fract narrowed_product = product;  /* Should trigger saturation */
        total_lf += (long _Fract)narrowed_product;
    }
    
    /* Case 3: Chain of operations creating wide intermediate */
    {
        _Accum chain = 0.5k;
        for (int j = 0; j < 4; j++) {
            chain = chain * chain;      /* Square repeatedly */
            chain = chain << 1;         /* Shift after each square */
            asm volatile ("" : : : "memory");
        }
        total_acc += chain;
    }
    
    /* Make results observable */
    printf("Result: %ld %ld\n", 
           (long)(total_acc * 1000k), 
           (long)(total_lf * 1000lr));
    
    return 0;
}
