/* fixed-value-coverage.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-value-coverage.c -o fixed-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding with volatile variables */
volatile short _Fract v_sf1 = 0.5r;
volatile short _Fract v_sf2 = 0.8r;
volatile _Accum v_acc1 = 0.5k;
volatile _Accum v_acc2 = 0.7k;
volatile long _Fract v_lf1 = 0.9lr;
volatile long _Fract v_lf2 = 0.95lr;

/* Function to perform fixed-point operations that may trigger overflow checks */
static _Accum process_fixed_ops(short _Fract a, _Accum b, long _Fract c, int shift) {
    /* Multiple operations to create wide intermediate results */
    _Accum temp1 = a * b;           /* Mixed-type multiplication */
    _Accum temp2 = temp1 << shift;  /* Left shift - FIXED_LSHIFT_EXPR */
    
    /* Force integer promotion */
    long long big_int = 1000;
    _Accum temp3 = temp2 * big_int; /* Integer promotion */
    
    /* Narrowing conversion with potential overflow */
    short _Fract narrow = c;        /* Potential overflow check */
    
    /* Another multiplication that may exceed bounds */
    _Accum temp4 = temp3 * narrow;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return temp4;
}

/* Another function focusing on saturation boundaries */
static short _Fract test_saturation_boundary(long _Fract a, long _Fract b, int iterations) {
    /* This multiplication can overflow short _Fract range */
    long _Fract wide_result = a * b;
    
    /* Narrowing conversion that requires saturation check */
    short _Fract result = wide_result;
    
    /* Multiple shifts to trigger the uncovered initialization */
    for (int i = 0; i < iterations; i++) {
        /* Left shift operation - directly triggers FIXED_LSHIFT_EXPR logic */
        result = result << 1;
        
        /* Memory barrier between operations */
        asm volatile ("" : : : "memory");
        
        /* Another multiplication to create complex expression */
        result = result * 0.5r;
    }
    
    return result;
}

/* Process array of fixed-point values */
static _Accum process_fixed_array(short _Fract arr[], int size, int shift_amount) {
    _Accum total = 0.0k;
    
    for (int i = 0; i < size; i++) {
        /* Complex expression with multiple operations */
        _Accum temp = arr[i];
        temp = temp << shift_amount;      /* Left shift */
        temp = temp * v_acc1;             /* Multiplication with volatile */
        temp = temp * (i + 1);            /* Integer promotion */
        
        /* Assignment to narrower type - may trigger overflow check */
        short _Fract narrowed = temp;
        
        total = total + (_Accum)narrowed;
        
        /* Prevent loop optimization */
        asm volatile ("" : : : "memory");
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 + 1 : 5;
    
    printf("Running fixed-point coverage test with %d iterations\n", iterations);
    
    /* Initialize array with pattern */
    short _Fract sf_array[10];
    for (int i = 0; i < 10; i++) {
        sf_array[i] = (short _Fract)(i * 0.1r);
    }
    
    /* Test 1: Process array with shifting */
    _Accum result1 = process_fixed_array(sf_array, iterations, 2);
    printf("Result 1: %k\n", result1);
    
    /* Test 2: Saturation boundary test */
    short _Fract result2 = test_saturation_boundary(v_lf1, v_lf2, iterations);
    printf("Result 2: %r\n", result2);
    
    /* Test 3: Mixed operations with volatile operands */
    _Accum result3 = 0.0k;
    for (int i = 0; i < iterations; i++) {
        /* Complex expression designed to trigger wide intermediate calculations */
        _Accum temp = v_acc1;
        
        /* Multiple left shifts */
        temp = temp << i;                     /* Variable shift amount */
        temp = temp * v_sf1;                  /* Mixed-type multiplication */
        temp = temp << 3;                     /* Constant shift */
        
        /* Multiplication that may exceed mode bounds */
        long _Fract wide_mult = v_lf1 * v_lf2;
        
        /* Narrowing assignment - may trigger overflow/saturation check */
        _Accum narrowed = wide_mult;
        
        temp = temp * narrowed;
        
        result3 = result3 + temp;
        
        /* Memory barrier to keep operations separate */
        asm volatile ("" : : : "memory");
    }
    printf("Result 3: %k\n", result3);
    
    /* Test 4: Direct calls to trigger the specific comparison */
    _Accum final_result = process_fixed_ops(v_sf1, v_acc1, v_lf1, 4);
    printf("Final result: %k\n", final_result);
    
    /* Ensure all results are used */
    volatile _Accum dummy __attribute__((unused)) = result1 + result2 + result3 + final_result;
    
    return 0;
}
