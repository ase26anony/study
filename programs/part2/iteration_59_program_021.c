/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Function with fixed-point parameters to prevent constant folding */
static short _Fract process_fract(short _Fract a, short _Fract b) {
    /* Multiple fixed-point operations that could overflow */
    short _Fract temp1 = a * b;           /* FIXED_MULT_P */
    short _Fract temp2 = temp1 << 1;      /* FIXED_LSHIFT_EXPR - triggers shift logic */
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Another multiplication with potential overflow */
    short _Fract result = temp2 * a;
    
    return result;
}

/* Function using _Accum types with wider intermediate results */
static _Accum process_accum(_Accum x, _Accum y) {
    /* Left shift operation - directly triggers FIXED_LSHIFT_EXPR logic */
    _Accum shifted = x << 2;              /* This should trigger the uncovered shift logic */
    
    /* Multiplication that might overflow */
    _Accum product = shifted * y;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Assign to narrower type to force range checking */
    short _Accum narrow_result = product; /* Potential overflow check */
    
    return (_Accum)narrow_result;
}

/* Function mixing fixed-point with integer promotions */
static long _Fract mixed_operations(short _Fract a, int multiplier) {
    /* Integer promotion in multiplication */
    long _Fract temp = a * multiplier;    /* Wider intermediate */
    
    /* Left shift on fixed-point */
    temp = temp << 1;                     /* FIXED_LSHIFT_EXPR */
    
    /* Convert to narrower type with potential overflow */
    short _Fract result = temp;           /* May trigger saturation check */
    
    return (long _Fract)result;
}

/* Main function with loops and volatile variables */
int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations <= 0) iterations = 5;
    if (iterations > 100) iterations = 100; /* Limit for safety */
    
    /* Volatile variables to prevent compile-time evaluation */
    volatile short _Fract vol_fract = 0.7r;
    volatile _Accum vol_accum = 0.8k;
    
    /* Arrays of fixed-point values */
    short _Fract fract_array[10];
    _Accum accum_array[10];
    
    /* Initialize arrays with a pattern */
    for (int i = 0; i < 10; i++) {
        fract_array[i] = (short _Fract)(i * 0.1r);
        accum_array[i] = (_Accum)(i * 0.2k);
    }
    
    short _Fract total_fract = 0.0r;
    _Accum total_accum = 0.0k;
    
    /* Main processing loop with various fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Read from volatile to get unknown-at-compile-time values */
        short _Fract a = vol_fract;
        _Accum b = vol_accum;
        
        /* Index with some variation */
        int idx = i % 10;
        
        /* Operation 1: Multiplication with potential overflow */
        short _Fract f1 = fract_array[idx];
        short _Fract f2 = fract_array[(idx + 1) % 10];
        short _Fract mult_result = f1 * f2;  /* FIXED_MULT_P */
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
        
        /* Operation 2: Left shift (triggers FIXED_LSHIFT_EXPR) */
        short _Fract shifted = mult_result << 2;  /* Directly triggers uncovered logic */
        
        /* Operation 3: Process with function call */
        short _Fract processed = process_fract(shifted, a);
        
        /* Operation 4: _Accum operations */
        _Accum acc1 = accum_array[idx];
        _Accum acc2 = accum_array[(idx + 3) % 10];
        _Accum acc_result = process_accum(acc1, acc2);
        
        /* Operation 5: Mixed operations with integer promotion */
        long _Fract mixed = mixed_operations(processed, i + 1);
        
        /* Accumulate results (prevents dead code elimination) */
        total_fract += processed;
        total_accum += acc_result + (_Accum)mixed;
        
        /* Modify volatile to change values for next iteration */
        asm volatile ("" : "+m" (vol_fract), "+m" (vol_accum));
    }
    
    /* Use results to prevent optimization */
    printf("Result: fract = %f, accum = %f\n", 
           (double)total_fract, (double)total_accum);
    
    /* Additional test cases targeting specific scenarios */
    
    /* Test 1: Narrowing conversion that might overflow */
    {
        long _Fract large_fract = 0.99lr;
        long _Fract another_large = 0.98lr;
        long _Fract big_product = large_fract * another_large;
        short _Fract narrow_target = big_product;  /* Should trigger range check */
        
        asm volatile ("" : : "r" (narrow_target) : "memory");
    }
    
    /* Test 2: Left shift near boundaries */
    {
        _Accum boundary_val = 0.5k;
        /* Multiple shifts to potentially exceed bounds */
        for (int shift = 1; shift <= 4; shift++) {
            _Accum shifted = boundary_val << shift;  /* FIXED_LSHIFT_EXPR */
            asm volatile ("" : : "r" (shifted) : "memory");
        }
    }
    
    /* Test 3: Saturation conversion with -fsat-conversion */
    {
        unsigned short _Fract uf1 = 0.8ur;
        unsigned short _Fract uf2 = 0.9ur;
        unsigned short _Fract uresult = uf1 * uf2;
        /* Convert to signed with potential overflow */
        short _Fract sresult = (short _Fract)uresult;
        
        asm volatile ("" : : "r" (sresult) : "memory");
    }
    
    return 0;
}
