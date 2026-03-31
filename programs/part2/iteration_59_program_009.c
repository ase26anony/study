/* fixed-point-test.c - Designed to trigger uncovered lines 264-277 in fixed-value.cc */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract volatile_sf = 0.5r;
volatile _Accum volatile_acc = 0.5k;
volatile long _Fract volatile_lf = 0.7lr;

/* Function with fixed-point parameters to force analysis */
_Accum fixed_point_operations(short _Fract a, _Accum b, long _Fract c) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* FIXED_MULT_P with different types */
    short _Fract temp2 = b * c;     /* Potential overflow to narrower type */
    
    /* Left shift operations - FIXED_LSHIFT_EXPR */
    _Accum shifted1 = temp1 << 3;   /* Shift that could overflow */
    _Accum shifted2 = b << 5;       /* Another shift */
    
    /* Mixed operations with integer promotion */
    int multiplier = 100;
    _Accum mixed = temp1 * multiplier;  /* Integer promotion */
    
    /* Chain operations to create complex expression */
    _Accum result = (shifted1 + shifted2) * mixed;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Another function focusing on saturation scenarios */
short _Fract saturation_test(long _Fract a, long _Fract b) {
    /* Multiplication that could overflow when assigned to short _Fract */
    long _Fract product = a * b;
    
    /* Explicit assignment to narrower type - should trigger range check */
    short _Fract narrowed = product;  /* This should check bounds */
    
    /* More operations */
    narrowed = narrowed << 2;         /* Left shift on fixed-point */
    
    asm volatile ("" : : : "memory");
    return narrowed;
}

/* Array-based operations to create loop-variant patterns */
void array_operations(int iterations) {
    /* Arrays of different fixed-point types */
    _Accum acc_array[10];
    short _Fract sf_array[10];
    long _Fract lf_array[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        acc_array[i] = (i * 0.1k);
        sf_array[i] = (i % 5) * 0.2r;
        lf_array[i] = (i * 0.05lr);
    }
    
    /* Perform operations in loop with variant indices */
    _Accum accumulator = 0.0k;
    for (int i = 0; i < iterations; i++) {
        int idx = i % 10;
        
        /* Multiple fixed-point multiplications */
        _Accum temp1 = acc_array[idx] * sf_array[idx];
        long _Fract temp2 = lf_array[idx] * sf_array[idx];
        
        /* Left shifts that could overflow */
        _Accum shifted = temp1 << (idx % 4 + 1);
        
        /* Assignment to narrower type - potential overflow */
        short _Fract narrowed = temp2;  /* Should trigger bounds check */
        
        /* More operations */
        narrowed = narrowed << 2;
        
        /* Accumulate with mixed types */
        accumulator += shifted * narrowed;
        
        /* Memory barrier to keep operations separate */
        asm volatile ("" : : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    volatile _Accum sink = accumulator;
    (void)sink;
}

/* Main function with argc-dependent iterations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop count non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    /* Call functions with volatile sources to prevent constant folding */
    short _Fract sf1 = volatile_sf;
    _Accum acc1 = volatile_acc;
    long _Fract lf1 = volatile_lf;
    
    /* Test case 1: Basic fixed-point operations */
    _Accum result1 = fixed_point_operations(sf1, acc1, lf1);
    
    /* Test case 2: Saturation scenarios */
    /* Use values close to 1.0 to increase overflow probability */
    long _Fract a = 0.9lr;
    long _Fract b = 0.95lr;
    short _Fract result2 = saturation_test(a, b);
    
    /* Test case 3: Array-based operations */
    array_operations(iterations);
    
    /* Test case 4: Direct overflow-inducing operations */
    /* These values when multiplied exceed short _Fract range */
    short _Fract sf_max = 0.99r;
    short _Fract sf_near_max = 0.98r;
    short _Fract product = sf_max * sf_near_max;  /* Should be ~0.9702 */
    
    /* Left shift that could overflow */
    _Accum acc_val = 0.8k;
    for (int i = 0; i < 3; i++) {
        acc_val = acc_val << 2;  /* FIXED_LSHIFT_EXPR */
        asm volatile ("" : : : "memory");
    }
    
    /* Test case 5: Mixed integer/fixed-point operations */
    int int_val = 256;
    _Accum mixed_result = acc_val * int_val;  /* Integer promotion */
    
    /* Test case 6: Complex expression tree */
    /* Create a complex expression that needs range analysis */
    _Accum complex_expr = ((sf1 * acc1) << 2) * (lf1 * 100);
    
    /* Use results to prevent optimization */
    volatile _Accum sink1 = result1;
    volatile short _Fract sink2 = result2;
    volatile short _Fract sink3 = product;
    volatile _Accum sink4 = mixed_result;
    volatile _Accum sink5 = complex_expr;
    
    (void)sink1; (void)sink2; (void)sink3; (void)sink4; (void)sink5;
    
    /* Print something to show the program ran */
    printf("Fixed-point test completed with %d iterations\n", iterations);
    
    return 0;
}
