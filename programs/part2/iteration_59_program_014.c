/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-point-test.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent constant folding with volatile variables */
volatile short _Fract volatile_sf = 0.5r;
volatile _Accum volatile_acc = 0.3k;
volatile long _Fract volatile_lf = 0.8lr;

/* Function with fixed-point parameters to force GIMPLE representation */
_Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* FIXED_MULT_P with different types */
    _Accum temp2 = temp1 << 2;      /* FIXED_LSHIFT_EXPR - triggers shift logic */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Narrower assignment forcing range check */
    short _Fract narrow_result = c * a;  /* Potential overflow to short _Fract */
    
    /* Another shift operation */
    _Accum temp3 = b << 3;
    
    /* Mixed-type multiplication with integer promotion */
    int multiplier = 4;
    _Accum temp4 = b * multiplier;  /* Integer promotion in fixed-point */
    
    /* Combine results - complex expression */
    return temp2 + (_Accum)narrow_result + temp3 + temp4;
}

/* Array processing with loop-variant values */
void process_fixed_array(int iterations) {
    /* Arrays of different fixed-point types */
    _Accum acc_array[10];
    short _Fract sf_array[10];
    long _Fract lf_array[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        acc_array[i] = (i % 10) * 0.1k;
        sf_array[i] = (i % 5) * 0.2r;
        lf_array[i] = (i % 8) * 0.125lr;
    }
    
    /* Process array with multiple fixed-point operations */
    for (int iter = 0; iter < iterations; iter++) {
        _Accum total = 0.0k;
        
        for (int i = 0; i < 9; i++) {  /* Stop at 9 to avoid overflow in shift */
            /* Complex expression with multiple operations */
            _Accum val1 = acc_array[i] * sf_array[i];  /* Mixed type mult */
            
            /* Left shift - directly triggers FIXED_LSHIFT_EXPR logic */
            _Accum val2 = val1 << (i % 3 + 1);  /* Variable shift amount */
            
            /* Memory barrier between operations */
            asm volatile ("" : : : "memory");
            
            /* Assignment to narrower type forcing saturation check */
            short _Fract narrow_val = lf_array[i] * sf_array[i+1];
            
            /* Another shift with different type */
            long _Fract val3 = lf_array[i] << 2;
            
            /* Integer promotion scenario */
            long int_promote = 100;
            _Accum val4 = acc_array[i] * int_promote;
            
            /* Combine with potential overflow */
            total += val2 + (_Accum)narrow_val + (_Accum)val3 + val4;
            
            /* Update array values to prevent dead code elimination */
            acc_array[i] = total * 0.01k;
        }
        
        /* Use result to prevent optimization */
        volatile _Accum sink = total;
        (void)sink;
    }
}

/* Test saturation on conversion paths */
void test_saturation_conversions(void) {
    /* Create values near bounds */
    long _Fract near_max = 0.99lr;
    long _Fract near_min = -0.99lr;
    
    /* Operations that could exceed short _Fract range */
    for (int i = 0; i < 5; i++) {
        /* Multiplication that could overflow when converted */
        long _Fract product = near_max * near_max;  /* ~0.98lr */
        
        /* Convert to narrower type with potential saturation */
        short _Fract converted = product;  /* Requires range check */
        
        /* Shift operation on the result */
        _Accum shifted = (_Accum)converted << 4;
        
        /* Another multiplication in narrower type */
        short _Fract sf1 = 0.9r;
        short _Fract sf2 = 0.95r;
        short _Fract narrow_product = sf1 * sf2;  /* Could overflow short _Fract */
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Use volatile to force computation */
        volatile short _Fract vol_result = narrow_product;
        (void)vol_result;
    }
}

/* Main function with argc-dependent iterations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop count non-constant */
    int iterations = (argc > 1) ? (argc % 5) + 2 : 3;
    
    printf("Running fixed-point tests with %d iterations\n", iterations);
    
    /* Test 1: Function with fixed-point parameters */
    short _Fract sf_param = volatile_sf;
    _Accum acc_param = volatile_acc;
    long _Fract lf_param = volatile_lf;
    
    _Accum result1 = process_fixed_point(sf_param, acc_param, lf_param);
    volatile _Accum vol_result1 = result1;
    (void)vol_result1;
    
    /* Test 2: Array processing */
    process_fixed_array(iterations);
    
    /* Test 3: Saturation conversions */
    test_saturation_conversions();
    
    /* Additional complex expression in main */
    _Accum complex_expr = 0.0k;
    for (int i = 0; i < iterations; i++) {
        /* Use volatile sources to prevent constant folding */
        short _Fract a = volatile_sf + (i * 0.1r);
        _Accum b = volatile_acc * (i + 1);
        
        /* Multiple operations in one expression */
        complex_expr += (a * b) << (i % 4) + (b * 256);  /* Large multiplier */
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Print something to ensure code isn't optimized away */
    printf("Final accumulator value: %ld\n", (long)(complex_expr * 1000k));
    
    return 0;
}
