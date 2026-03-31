/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Function with fixed-point parameters to prevent constant folding */
volatile long _Fract get_input(int idx) {
    static volatile long _Fract values[] = {
        0.1lr, 0.5lr, 0.7lr, 0.9lr, 0.99lr,
        -0.1lr, -0.5lr, -0.7lr, -0.9lr, -0.99lr
    };
    return values[idx % 10];
}

/* Function that performs fixed-point operations likely to trigger range checking */
short _Fract process_fixed_point(long _Fract a, long _Fract b, int shift) {
    volatile long _Fract temp;
    
    /* Multiplication that may overflow when converted to narrower type */
    temp = a * b;
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    temp = temp << shift;
    
    /* Another multiplication to create wider intermediate */
    temp = temp * (long _Fract)0.8lr;
    
    /* Convert to narrower type - may trigger saturation/overflow checks */
    short _Fract result = (short _Fract)temp;
    
    return result;
}

/* Another function using _Accum types */
_Accum process_accum(_Accum a, _Accum b, int shift) {
    volatile _Accum temp;
    
    /* Multiple operations to create complex expression */
    temp = a * b;
    asm volatile ("" : : : "memory");
    
    /* Left shift that should trigger FIXED_LSHIFT_EXPR logic */
    temp = temp << shift;
    
    /* Additional multiplication */
    temp = temp * (_Accum)1.5k;
    
    /* Operation with integer promotion */
    temp = temp * 2;
    
    return temp;
}

/* Function with array operations */
void process_fixed_array(short _Fract *output, int size) {
    volatile long _Fract accum = 0.5lr;
    
    for (int i = 0; i < size; i++) {
        /* Get non-constant input */
        long _Fract input = get_input(i);
        
        /* Complex fixed-point expression */
        long _Fract temp = input * accum;
        
        /* Left shift with variable amount */
        temp = temp << (i % 4);
        
        /* Multiply by another fixed-point value */
        temp = temp * (long _Fract)0.9lr;
        
        /* Convert to narrower type - potential overflow */
        output[i] = (short _Fract)temp;
        
        /* Update accumulator with non-trivial value */
        accum = accum * (long _Fract)0.95lr;
        
        /* Memory barrier to preserve operations */
        asm volatile ("" : : : "memory");
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 + 5 : 8;
    
    /* Array of fixed-point values */
    short _Fract results[20];
    _Accum accum_results[10];
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile long _Fract init_val = 0.8lr;
    volatile _Accum init_accum = 0.6k;
    
    printf("Starting fixed-point tests with %d iterations\n", iterations);
    
    /* Test 1: Process array of fixed-point values */
    process_fixed_array(results, iterations < 20 ? iterations : 20);
    
    /* Test 2: Individual fixed-point operations */
    for (int i = 0; i < iterations && i < 10; i++) {
        /* Get non-constant inputs */
        long _Fract a = get_input(i);
        long _Fract b = get_input(i + 3);
        int shift = i % 5;
        
        /* Process with potential overflow */
        results[i] = process_fixed_point(a, b, shift);
        
        /* Also test _Accum operations */
        _Accum acc_a = (_Accum)a;
        _Accum acc_b = (_Accum)b * 2.0k;
        accum_results[i] = process_accum(acc_a, acc_b, shift);
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Test 3: Mixed-type operations */
    volatile short _Fract sf1 = 0.7r;
    volatile short _Fract sf2 = 0.8r;
    volatile _Accum acc1 = 0.5k;
    
    /* Multiplication that may need range checking */
    _Accum mixed_result = acc1 * (_Accum)(sf1 * sf2);
    
    /* Left shift on the result */
    mixed_result = mixed_result << 3;
    
    /* Convert to narrower type */
    short _Fract final_result = (short _Fract)mixed_result;
    
    /* Test 4: Edge case with near-maximum values */
    volatile long _Fract near_max = 0.999lr;
    volatile long _Fract near_max2 = 0.998lr;
    
    /* This multiplication approaches 1.0, may trigger max bound checks */
    long _Fract product = near_max * near_max2;
    
    /* Left shift that could overflow */
    product = product << 2;
    
    /* Force conversion to narrower type */
    short _Fract narrowed = (short _Fract)product;
    
    /* Use results to prevent dead code elimination */
    short _Fract sum = 0r;
    for (int i = 0; i < iterations && i < 20; i++) {
        sum += results[i % 20];
    }
    
    printf("Final sum: %f\n", (double)sum);
    printf("Mixed result: %f\n", (double)mixed_result);
    printf("Narrowed product: %f\n", (double)narrowed);
    
    return 0;
}
