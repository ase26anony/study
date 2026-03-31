/* fixed-value-coverage.c
 * Designed to trigger uncovered lines 264-277 in fixed-value.cc
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-value-coverage.c -o fixed-value-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding with volatile variables */
volatile short _Fract volatile_sf = 0.5r;
volatile _Accum volatile_acc = 0.5k;
volatile long _Fract volatile_lf = 0.9lr;

/* Function with fixed-point parameters to force analysis */
_Accum process_fixed_point(short _Fract a, _Accum b, long _Fract c) {
    /* Multiple operations that could overflow */
    _Accum temp1 = a * b;           /* Mixed precision multiplication */
    _Accum temp2 = temp1 << 3;      /* Left shift - FIXED_LSHIFT_EXPR */
    
    /* Narrowing conversion with potential overflow */
    short _Fract narrow = c;        /* From long _Fract to short _Fract */
    
    /* More operations */
    _Accum temp3 = narrow * b;
    temp3 = temp3 << 2;             /* Another left shift */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return temp2 + temp3;
}

/* Another function focusing on saturation paths */
short _Fract saturate_operations(int iterations) {
    /* Array of fixed-point values */
    _Accum arr[10];
    short _Fract result = 0.0r;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 10; i++) {
        arr[i] = (i % 3 == 0) ? 0.8k : 0.3k;
    }
    
    /* Loop with fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Operations that could exceed bounds */
        _Accum temp = arr[i % 10];
        
        /* Left shift that could overflow */
        temp = temp << (i % 4 + 1);  /* Variable shift amount */
        
        /* Multiplication with widening */
        long _Fract wide = temp * volatile_lf;
        
        /* Narrowing assignment that requires range check */
        short _Fract narrowed = wide;
        
        result = result + narrowed;
        
        /* Update array element */
        arr[i % 10] = temp * 0.9k;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Function to test unsigned fixed-point types */
unsigned short _Fract test_unsigned_fixed(int seed) {
    unsigned short _Fract a = 0.8ur;
    unsigned _Accum b = 0.9uk;
    
    /* Operations that might hit max bounds */
    for (int i = 0; i < seed % 5 + 1; i++) {
        b = b << 1;                 /* Left shift unsigned */
        a = a * b;                  /* Multiplication */
        
        /* Mix with signed types */
        short _Fract signed_val = 0.7r;
        a = a * signed_val;
    }
    
    return a;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 + 1 : 5;
    
    /* Initialize with volatile to prevent compile-time evaluation */
    short _Fract sf1 = volatile_sf;
    short _Fract sf2 = 0.8r;
    _Accum acc1 = volatile_acc;
    long _Fract lf1 = volatile_lf;
    
    printf("Starting fixed-point coverage test with %d iterations\n", iterations);
    
    /* Test 1: Basic multiplication with potential overflow */
    short _Fract result1 = 0.0r;
    for (int i = 0; i < iterations; i++) {
        /* Multiplication that could exceed short _Fract range */
        long _Fract temp = lf1 * sf1;
        
        /* Narrowing conversion - triggers range checking */
        short _Fract narrowed = temp;
        
        result1 = result1 + narrowed;
        
        /* Modify values to prevent optimization */
        sf1 = sf1 * 0.9r;
        lf1 = lf1 * 0.95lr;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Test 2: Left shift operations */
    _Accum result2 = 0.0k;
    for (int i = 0; i < iterations; i++) {
        /* Variable left shift */
        int shift = (i % 3) + 1;
        _Accum shifted = acc1 << shift;  /* FIXED_LSHIFT_EXPR */
        
        /* Multiplication after shift */
        shifted = shifted * 1.5k;
        
        result2 = result2 + shifted;
        
        /* Update accumulator */
        acc1 = acc1 * 0.8k;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Test 3: Function calls with mixed types */
    _Accum result3 = process_fixed_point(sf2, acc1, lf1);
    
    /* Test 4: Saturation-focused operations */
    short _Fract result4 = saturate_operations(iterations);
    
    /* Test 5: Unsigned fixed-point */
    unsigned short _Fract result5 = test_unsigned_fixed(iterations);
    
    /* Combine results to create observable side effect */
    _Accum final_result = result1 + result2 + result3 + result4 + result5;
    
    /* Print something to prevent dead code elimination */
    printf("Final accumulated result (simplified): %d\n", (int)(final_result * 1000));
    
    return 0;
}
