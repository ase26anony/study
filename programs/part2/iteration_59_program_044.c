/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile long _Fract volatile_source = 0x0.8lr;

/* Function with fixed-point parameters to force analysis */
short _Fract process_fixed(short _Fract a, short _Fract b, int shift) {
    /* Intermediate multiplication with potential overflow */
    short _Fract prod = a * b;
    
    /* Left shift operation (FIXED_LSHIFT_EXPR) */
    prod = prod << shift;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return prod;
}

/* Another function with _Accum type */
_Accum process_accum(_Accum a, _Accum b, int shift) {
    /* Multiplication that may overflow */
    _Accum result = a * b;
    
    /* Left shift with saturation checking */
    result = result << shift;
    
    /* Assign to narrower type to force range check */
    short _Accum narrowed = result;
    
    asm volatile ("" : : : "memory");
    return narrowed;
}

/* Function mixing integer and fixed-point */
long _Fract mixed_operation(long _Fract f, int i) {
    /* Integer promotion in multiplication */
    long _Fract result = f * i;
    
    /* Multiple shifts to trigger bounds checking */
    result = result << 1;
    result = result << 2;
    
    asm volatile ("" : : : "memory");
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop count non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations > 10) iterations = 10;
    
    /* Array of fixed-point values */
    short _Fract sf_array[10];
    _Accum accum_array[10];
    
    /* Initialize with pattern */
    for (int i = 0; i < 10; i++) {
        sf_array[i] = (short _Fract)(i * 0.1r);
        accum_array[i] = (_Accum)(i * 0.05k);
    }
    
    short _Fract total_sf = 0.0r;
    _Accum total_accum = 0.0k;
    
    /* Main loop with various fixed-point operations */
    for (int i = 0; i < iterations; i++) {
        /* Operation 1: Multiplication with potential overflow */
        short _Fract a = sf_array[i];
        short _Fract b = (short _Fract)volatile_source; /* Volatile source */
        short _Fract c = a * b;
        
        /* Operation 2: Left shift (FIXED_LSHIFT_EXPR) */
        c = c << (i % 3 + 1);
        
        /* Operation 3: Assign to potentially narrower context */
        total_sf += c;
        
        /* Operation 4: _Accum multiplication and shift */
        _Accum acc1 = accum_array[i];
        _Accum acc2 = (_Accum)((i + 1) * 0.1k);
        _Accum acc_result = acc1 * acc2;
        
        /* Multiple shifts to increase chance of overflow */
        acc_result = acc_result << 1;
        acc_result = acc_result << 2;
        
        total_accum += acc_result;
        
        /* Operation 5: Mixed integer/fixed-point */
        long _Fract lf = (long _Fract)(i * 0.05lr);
        long _Fract mixed = mixed_operation(lf, i + 2);
        
        /* Convert and add to total */
        total_accum += (_Accum)mixed;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Additional saturation-provoking operations */
    /* Multiply values close to 1.0 to potentially exceed range */
    short _Fract near_one = 0.99r;
    short _Fract another_near_one = 0.98r;
    short _Fract product = process_fixed(near_one, another_near_one, 2);
    
    /* Process with _Accum near limits */
    _Accum large_accum = 0.999k;
    _Accum result_accum = process_accum(large_accum, large_accum, 3);
    
    /* Print results to prevent dead code elimination */
    printf("Result SF: %f\n", (double)total_sf);
    printf("Result Accum: %f\n", (double)total_accum);
    printf("Product: %f\n", (double)product);
    printf("Result Accum2: %f\n", (double)result_accum);
    
    return 0;
}
