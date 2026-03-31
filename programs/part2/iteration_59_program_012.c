/* fixed-point-test.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-point-test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract vf1 = 0.8r;
volatile short _Fract vf2 = 0.9r;
volatile _Accum vacc = 0.5k;

/* Function with fixed-point parameters to force analysis */
_Accum process_fixed_point(short _Fract a, _Accum b, int shift) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* FIXED_MULT_P */
    _Accum temp2 = temp1 << shift;  /* FIXED_LSHIFT_EXPR - triggers shift logic */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Convert to narrower type - may trigger saturation */
    short _Fract narrow = temp2;
    
    /* Another multiplication with potential overflow */
    _Accum temp3 = narrow * b;
    
    return temp3;
}

/* Another function with different fixed-point types */
long _Fract complex_operation(unsigned short _Fract a, long _Fract b, int iterations) {
    long _Fract result = 0.0lr;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix with integer promotion */
        result = result + (a * i);
        
        /* Left shift on fixed-point */
        if (i % 2 == 0) {
            result = result << 1;  /* FIXED_LSHIFT_EXPR */
        }
        
        /* Multiplication that could overflow */
        result = result * b;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations > 100) iterations = 100; /* Safety limit */
    
    /* Array of fixed-point values */
    _Accum accum_array[10];
    short _Fract fract_array[10];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 10; i++) {
        accum_array[i] = i * 0.1k;
        fract_array[i] = i * 0.1r;
    }
    
    _Accum total = 0.0k;
    
    /* Loop with various fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Read volatile variables to get unknown values */
        short _Fract f1 = vf1;
        short _Fract f2 = vf2;
        _Accum acc = vacc;
        
        /* Operation 1: Multiplication with potential overflow */
        _Accum temp1 = f1 * f2 * acc;
        
        /* Operation 2: Left shift (triggers FIXED_LSHIFT_EXPR logic) */
        int shift_amount = i % 4;
        _Accum temp2 = temp1 << shift_amount;
        
        /* Operation 3: Assign to narrower type - may trigger saturation check */
        short _Fract narrow_result = temp2;
        
        /* Operation 4: Mix with array values */
        _Accum temp3 = narrow_result * accum_array[i % 10];
        
        /* Operation 5: Another left shift */
        temp3 = temp3 << 1;
        
        /* Operation 6: Convert and multiply */
        long _Fract lf1 = temp3;
        long _Fract lf2 = 0.95lr;
        long _Fract temp4 = lf1 * lf2;
        
        /* Operation 7: Shift on long fract */
        temp4 = temp4 << 2;
        
        /* Convert back and accumulate */
        total += temp4;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Additional test cases with function calls */
    short _Fract sf_param = 0.7r;
    _Accum acc_param = 0.6k;
    
    for (int i = 0; i < 3; i++) {
        /* Call function that does fixed-point operations */
        _Accum result1 = process_fixed_point(sf_param, acc_param, i);
        
        /* Call function with different fixed-point types */
        unsigned short _Fract usf = 0.8ur;
        long _Fract lf_param = 0.9lr;
        long _Fract result2 = complex_operation(usf, lf_param, i + 1);
        
        total += result1 + result2;
    }
    
    /* Test with unsigned fixed-point types */
    unsigned _Accum uacc1 = 0.8uk;
    unsigned _Accum uacc2 = 0.9uk;
    
    /* Multiplication that could saturate */
    unsigned _Accum uresult = uacc1 * uacc2;
    
    /* Left shift on unsigned */
    uresult = uresult << 1;
    
    total += uresult;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", (double)total);
    
    return 0;
}
