/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-point-test.c -o fixed-point-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract volatile_sf = 0.7r;
volatile long _Fract volatile_lf = 0.9lr;
volatile _Accum volatile_acc = 0.5k;
volatile long _Accum volatile_lacc = 0.8lk;

/* Function with fixed-point parameters to force GIMPLE representation */
_Accum process_fixed_point(short _Fract a, _Accum b, int shift) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* FIXED_MULT_P */
    _Accum temp2 = temp1 << shift;  /* FIXED_LSHIFT_EXPR - triggers the uncovered logic */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Narrowing conversion that requires saturation check */
    short _Fract narrow = temp2;    /* Potential overflow */
    
    /* Another multiplication with different types */
    long _Fract lf_result = narrow * volatile_lf;
    
    return (_Accum)lf_result;
}

/* Another function focusing on _Fract operations */
long _Fract fract_operations(unsigned _Fract uf, _Fract sf, int iterations) {
    long _Fract result = 0.0lr;
    
    for (int i = 0; i < iterations; i++) {
        /* Operations that create wide intermediate results */
        long _Fract temp = uf * sf;      /* Multiplication */
        temp = temp << (i % 4);          /* Left shift - triggers shift logic */
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Assignment to narrower type requiring bounds check */
        _Fract narrowed = temp;
        
        result += narrowed * volatile_sf;
        
        /* Modify values to prevent complete optimization */
        uf = uf * 0.95ur;
        sf = sf * 0.9r;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Initialize arrays of fixed-point values */
    _Accum acc_array[10];
    short _Fract sf_array[10];
    long _Fract lf_array[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        acc_array[i] = (i + 1) * 0.1k;
        sf_array[i] = (i + 1) * 0.1r;
        lf_array[i] = (i + 1) * 0.1lr;
    }
    
    _Accum total_acc = 0.0k;
    long _Fract total_lf = 0.0lr;
    
    /* Loop with various fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Mix different fixed-point types */
        _Accum a = acc_array[i % 10];
        short _Fract b = sf_array[i % 10];
        long _Fract c = lf_array[i % 10];
        
        /* Operation 1: Multiplication with potential overflow */
        _Accum mult_result = a * b * volatile_acc;
        
        /* Operation 2: Left shift (FIXED_LSHIFT_EXPR) */
        int shift_amount = (i % 3) + 1;
        _Accum shifted = mult_result << shift_amount;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Operation 3: Narrowing conversion requiring saturation check */
        short _Fract narrowed = shifted;
        
        /* Operation 4: Another multiplication */
        long _Fract wide_result = narrowed * c * volatile_lf;
        
        /* Operation 5: More shifting */
        wide_result = wide_result << ((i % 2) + 1);
        
        /* Call functions that process fixed-point */
        _Accum func_result = process_fixed_point(b, a, shift_amount);
        long _Fract fract_result = fract_operations(0.8ur, 0.7r, i + 1);
        
        /* Accumulate results */
        total_acc += func_result + shifted;
        total_lf += wide_result + fract_result;
        
        /* Modify array values to prevent optimization */
        acc_array[i % 10] = acc_array[i % 10] * 0.98k;
        sf_array[i % 10] = sf_array[i % 10] * 0.95r;
    }
    
    /* Additional test cases specifically designed for overflow */
    
    /* Test 1: Multiplication that should approach max value */
    unsigned _Fract uf1 = 0.999ur;
    unsigned _Fract uf2 = 0.999ur;
    unsigned _Fract uf_product = uf1 * uf2;  /* Close to max */
    
    /* Test 2: Left shift that could overflow */
    _Accum large_acc = 0.9k;
    for (int shift = 1; shift <= 4; shift++) {
        _Accum shifted_acc = large_acc << shift;
        /* Assignment to narrower type */
        short _Fract test_narrow = shifted_acc;
        asm volatile ("" : : : "memory");
    }
    
    /* Test 3: Mixed integer and fixed-point */
    int int_val = 100;
    _Accum mixed_mult = int_val * volatile_acc;
    mixed_mult = mixed_mult << 2;
    
    /* Test 4: Chain of operations */
    long _Fract chain = 0.5lr;
    for (int j = 0; j < 3; j++) {
        chain = chain * 1.5lr;
        chain = chain << 1;
        _Fract intermediate = chain;  /* Narrowing */
        chain = intermediate * 0.8lr;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: %kd, %r, %lr\n", total_acc, uf_product, total_lf);
    
    return 0;
}
