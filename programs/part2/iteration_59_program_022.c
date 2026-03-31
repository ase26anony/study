/* fixed-point-test.c
 * Designed to trigger GCC's fixed-value.cc logic for saturation checking
 * Specifically targets lines 264-277 in fixed-value.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract volatile_sf = 0.5r;
volatile _Accum volatile_acc = 0.5k;
volatile long _Fract volatile_lf = 0.9lr;

/* Function with fixed-point operations that may overflow */
static _Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c, int shift) {
    /* Multiple operations to create complex intermediate values */
    _Accum temp1 = a * b;           /* FIXED_MULT_P with different types */
    _Accum temp2 = temp1 << shift;  /* FIXED_LSHIFT_EXPR - triggers shift logic */
    
    /* Mix with integer promotion */
    long long int_prom = (long)temp2 * 256;
    _Accum temp3 = (_Accum)int_prom;
    
    /* Another multiplication that could overflow */
    _Accum result = temp3 * c;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Function that forces saturation checking */
static short _Fract narrow_conversion(_Accum wide_val) {
    /* Convert to narrower type - may trigger saturation */
    short _Fract narrow = (short _Fract)wide_val;
    
    /* Additional shift on the narrow type */
    narrow = narrow << 1;  /* FIXED_LSHIFT_EXPR on _Fract */
    
    asm volatile ("" : : : "memory");
    return narrow;
}

/* Process array with loop-variant operations */
static void process_array(_Accum *arr, int size, int shift) {
    for (int i = 0; i < size; i++) {
        /* Loop-variant shift amount */
        int dynamic_shift = (shift + i) % 8;
        
        /* Operation that creates wide intermediate */
        _Accum temp = arr[i] * 2.5k;
        
        /* Left shift - triggers FIXED_LSHIFT_EXPR logic */
        arr[i] = temp << dynamic_shift;
        
        /* Mix with volatile to prevent optimization */
        arr[i] = arr[i] * volatile_acc;
        
        /* Memory barrier every few iterations */
        if (i % 3 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 + 5 : 8;
    
    /* Initialize arrays with various fixed-point types */
    _Accum acc_array[20];
    short _Fract sf_array[20];
    long _Fract lf_array[20];
    
    /* Initialize with values that could cause overflow */
    for (int i = 0; i < 20; i++) {
        /* Use i to create varying values, prevent constant folding */
        acc_array[i] = (_Accum)(0.1k * i + 0.5k);
        sf_array[i] = (short _Fract)(0.05r * i + 0.5r);
        lf_array[i] = (long _Fract)(0.8lr - 0.01lr * i);
    }
    
    _Accum total_result = 0.0k;
    
    /* Main processing loop with multiple fixed-point operations */
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary shift amount based on iteration */
        int shift_amount = (iter * 3) % 7 + 1;
        
        /* Get volatile values to prevent compile-time computation */
        short _Fract sf_input = volatile_sf + (short _Fract)(0.01r * iter);
        _Accum acc_input = volatile_acc + (_Accum)(0.1k * iter);
        long _Fract lf_input = volatile_lf - (long _Fract)(0.01lr * iter);
        
        /* Complex fixed-point expression */
        _Accum intermediate = process_fixed_point(sf_input, acc_input, lf_input, shift_amount);
        
        /* Force potential overflow by converting to narrower type */
        short _Fract narrowed = narrow_conversion(intermediate);
        
        /* Accumulate result to prevent dead code elimination */
        total_result += (_Accum)narrowed * 0.5k;
        
        /* Process arrays with shifting operations */
        process_array(acc_array, 15, shift_amount);
        
        /* Additional multiplication that could overflow */
        for (int i = 0; i < 10; i++) {
            /* Multiplication between different fixed-point types */
            _Accum mixed = (_Accum)sf_array[i] * lf_array[i];
            
            /* Left shift operation */
            mixed = mixed << ((i + shift_amount) % 5);
            
            /* Assign to array element */
            acc_array[i] = mixed;
            
            /* Another memory barrier */
            if (i % 4 == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        /* Update volatile to change values for next iteration */
        asm volatile ("" : : "r"(iter) : "memory");
    }
    
    /* Use result to prevent optimization */
    printf("Result: %lld\n", (long long)(total_result * 1000k));
    
    /* Additional fixed-point operations in return calculation */
    _Accum final_calc = total_result;
    for (int i = 0; i < 5; i++) {
        final_calc = final_calc << 2;  /* Multiple shifts */
        final_calc = final_calc * 1.5k; /* Multiplication */
    }
    
    /* Final narrow conversion that may trigger saturation check */
    short _Fract final_narrow = (short _Fract)final_calc;
    
    return (int)(final_narrow * 100r);
}
