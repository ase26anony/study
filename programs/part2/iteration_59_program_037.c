/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-point-test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract volatile_sf = 0.5r;
volatile long _Fract volatile_lf = 0.8lr;
volatile _Accum volatile_acc = 0.5k;
volatile long _Accum volatile_lacc = 0.7lk;

/* Function with fixed-point parameters to force analysis */
static _Accum process_fixed_point(short _Fract a, _Accum b, int shift) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* FIXED_MULT_P */
    _Accum temp2 = temp1 << shift;  /* FIXED_LSHIFT_EXPR - should trigger the uncovered code */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return temp2;
}

/* Another function with narrower target type to force saturation checks */
static short _Fract narrow_conversion(long _Fract a, long _Fract b) {
    /* Multiplication that might exceed short _Fract range */
    long _Fract wide_result = a * b;
    
    /* Explicit cast to narrower type - should trigger range checking */
    short _Fract narrow_result = (short _Fract)wide_result;
    
    asm volatile ("" : : : "memory");
    
    return narrow_result;
}

/* Function with mixed-type operations */
static _Accum mixed_operations(_Accum base, int multiplier, int shift) {
    /* Mix fixed-point with integer promotion */
    _Accum result = base * multiplier;  /* Integer promotion in multiplication */
    
    /* Left shift that could overflow */
    if (shift > 0) {
        result = result << shift;  /* FIXED_LSHIFT_EXPR */
    }
    
    asm volatile ("" : : : "memory");
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations <= 0) iterations = 5;
    if (iterations > 100) iterations = 100; /* Limit for safety */
    
    /* Array of fixed-point values for loop processing */
    _Accum accum_array[10];
    short _Fract fract_array[10];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 10; i++) {
        accum_array[i] = (i % 10) * 0.1k;
        fract_array[i] = (i % 10) * 0.1r;
    }
    
    _Accum total_accum = 0k;
    short _Fract total_fract = 0r;
    
    /* Main processing loop with various fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Operation 1: Multiplication with potential overflow */
        long _Fract lf1 = 0.9lr;
        long _Fract lf2 = 0.95lr;
        short _Fract sf_result = narrow_conversion(lf1, lf2);
        total_fract += sf_result;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Operation 2: Left shift of _Accum (FIXED_LSHIFT_EXPR) */
        _Accum acc_val = 0.5k + (i % 3) * 0.1k;
        int shift_amount = (i % 4) + 1;  /* Shift 1-4 bits */
        _Accum shifted = acc_val << shift_amount;  /* This should trigger the uncovered code */
        total_accum += shifted;
        
        /* Operation 3: Process with function call */
        short _Fract sf_param = volatile_sf + (i % 5) * 0.05r;
        _Accum acc_param = volatile_acc + (i % 3) * 0.1k;
        _Accum processed = process_fixed_point(sf_param, acc_param, (i % 3) + 1);
        total_accum += processed;
        
        /* Operation 4: Mixed operations with integer promotion */
        _Accum mixed_result = mixed_operations(volatile_lacc, (i % 7) + 1, (i % 3));
        total_accum += mixed_result;
        
        /* Operation 5: Array-based operations */
        int idx = i % 10;
        _Accum array_result = accum_array[idx] * fract_array[idx];  /* Mixed type multiplication */
        total_accum += array_result;
        
        /* Operation 6: Chain of operations */
        _Accum chain = 0.25k;
        chain = chain << 1;      /* FIXED_LSHIFT_EXPR */
        chain = chain * 2.0k;    /* FIXED_MULT_P */
        chain = chain << 2;      /* Another FIXED_LSHIFT_EXPR */
        total_accum += chain;
        
        /* Operation 7: Near-boundary values to trigger saturation checks */
        short _Fract near_max = 0.99r;
        short _Fract near_min = -0.99r;
        short _Fract boundary_result = near_max * near_max;  /* Could saturate */
        total_fract += boundary_result;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    volatile _Accum final_accum = total_accum;
    volatile short _Fract final_fract = total_fract;
    
    /* Print something to ensure code isn't optimized away entirely */
    if (final_accum != 0k || final_fract != 0r) {
        printf("Processed %d iterations\n", iterations);
    }
    
    return 0;
}
