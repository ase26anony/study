/* fixed-value-coverage.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-value-coverage.c -o fixed-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract volatile_sf = 0.7r;
volatile _Accum volatile_ka = 0.5k;
volatile long _Fract volatile_lf = 0.9lr;

/* Function with fixed-point operations that may overflow */
_Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c, int shift) {
    /* Multiple operations that could trigger range checking */
    _Accum temp1 = a * b;           /* FIXED_MULT_P with different precisions */
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation - FIXED_LSHIFT_EXPR */
    _Accum temp2 = b << shift;      /* This directly triggers shift logic */
    
    asm volatile ("" : : : "memory");
    
    /* Multiplication that might overflow when converted */
    long _Fract temp3 = c * c;      /* Square could exceed range */
    
    asm volatile ("" : : : "memory");
    
    /* Narrow conversion that requires saturation check */
    short _Fract narrow_result = temp3;  /* Potential overflow here */
    
    asm volatile ("" : : : "memory");
    
    /* Mixed-type operation with integer promotion */
    _Accum temp4 = a * 256;         /* Integer promotion of fixed-point */
    
    /* Combine results */
    return temp1 + temp2 + (_Accum)narrow_result + temp4;
}

/* Another function focusing on shift operations */
void shift_operations(_Accum *arr, int size, int base_shift) {
    for (int i = 0; i < size; i++) {
        /* Varying shift amounts to prevent optimization */
        int shift = base_shift + (i % 4);
        
        /* Left shift with potential overflow */
        _Accum shifted = arr[i] << shift;
        
        /* Memory barrier between operations */
        asm volatile ("" : : : "memory");
        
        /* Store back, potentially to narrower type */
        short _Fract narrowed = shifted;
        
        asm volatile ("" : : : "memory");
        
        /* Use result to prevent dead code elimination */
        arr[i] = (_Accum)narrowed + 0.1k;
    }
}

/* Function that creates complex fixed-point expressions */
long _Fract complex_expression(unsigned _Fract uf, _Sat _Accum sat_acc) {
    /* Multiple operations in sequence */
    long _Fract lf1 = uf * 2.0lr;    /* Multiplication with constant */
    
    asm volatile ("" : : : "memory");
    
    /* Shift operation */
    _Accum acc_shifted = sat_acc << 3;  /* FIXED_LSHIFT_EXPR */
    
    asm volatile ("" : : : "memory");
    
    /* Mixed operations */
    long _Fract result = lf1 * (_Accum)acc_shifted;
    
    /* This multiplication might exceed the mode's bounds */
    return result * result;  /* Square could trigger max_r/max_s check */
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 2) iterations = 2;
    if (iterations > 10) iterations = 10;
    
    /* Initialize arrays with various fixed-point types */
    _Accum accum_array[10];
    short _Fract sf_array[10];
    long _Fract lf_array[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        accum_array[i] = (i % 10) * 0.1k;
        sf_array[i] = (i % 10) * 0.1r;
        lf_array[i] = (i % 10) * 0.1lr;
    }
    
    _Accum total_result = 0.0k;
    
    /* Main loop with various fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Read volatile values to prevent constant propagation */
        short _Fract sf = volatile_sf + (i * 0.01r);
        _Accum ka = volatile_ka + (i * 0.05k);
        long _Fract lf = volatile_lf - (i * 0.02lr);
        
        /* Operation 1: Process with potential overflow */
        _Accum result1 = process_fixed_point(sf, ka, lf, i % 4 + 1);
        
        asm volatile ("" : : : "memory");
        
        /* Operation 2: Shift operations on array */
        shift_operations(accum_array, 5, i % 3);
        
        /* Operation 3: Complex expression with saturation */
        unsigned _Fract uf = 0.8ur;
        _Sat _Accum sat_acc = 0.9k;
        long _Fract result3 = complex_expression(uf, sat_acc);
        
        /* Narrow conversion that might trigger saturation logic */
        short _Fract narrowed = result3;  /* This may need bounds checking */
        
        asm volatile ("" : : : "memory");
        
        /* Accumulate results to prevent optimization */
        total_result += result1 + (_Accum)narrowed + accum_array[i % 5];
        
        /* Modify array elements to create loop-carried dependencies */
        accum_array[i % 5] = total_result * 0.5k;
    }
    
    /* Additional test cases targeting specific scenarios */
    
    /* Test 1: Multiplication that could overflow max_r/max_s bounds */
    {
        long _Fract a = 0.99lr;
        long _Fract b = 0.99lr;
        /* This multiplication might approach or exceed 1.0 */
        long _Fract product = a * b;
        short _Fract narrowed_product = product;  /* Conversion check */
        
        asm volatile ("" : : : "memory");
        total_result += (_Accum)narrowed_product;
    }
    
    /* Test 2: Left shift that could overflow */
    {
        _Accum val = 0.75k;
        /* Shift by amount that could overflow the integer part */
        for (int shift = 1; shift <= 5; shift++) {
            _Accum shifted = val << shift;  /* Multiple FIXED_LSHIFT_EXPR */
            short _Fract narrowed = shifted;
            asm volatile ("" : : : "memory");
            total_result += (_Accum)narrowed;
        }
    }
    
    /* Test 3: Mixed-width operations */
    {
        short _Fract sf1 = 0.8r;
        short _Fract sf2 = 0.9r;
        long _Fract lf_result = sf1 * sf2;  /* Wider intermediate */
        
        /* Convert to narrower with potential saturation */
        _Sat short _Fract saturated = lf_result;
        
        asm volatile ("" : : : "memory");
        total_result += (_Accum)saturated;
    }
    
    /* Print result to ensure side effects */
    printf("Result: %k\n", total_result);
    
    return 0;
}
