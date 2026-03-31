/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Function with fixed-point operations that should trigger range checking */
static volatile short _Fract get_volatile_fract(void) {
    volatile short _Fract v = 0.7r;
    return v;
}

/* Function that performs fixed-point multiplication with potential overflow */
long _Fract process_fixed_mult(short _Fract a, short _Fract b) {
    /* These operations should create fixed-value objects that need bounds checking */
    long _Fract result;
    
    /* Force intermediate calculations by using volatile */
    volatile short _Fract va = a;
    volatile short _Fract vb = b;
    
    /* Multiplication that may exceed short _Fract range */
    result = (long _Fract)va * (long _Fract)vb;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Function with left shift operations on fixed-point types */
_Accum process_fixed_shift(_Accum val, int shift) {
    _Accum result = val;
    
    /* Multiple shift operations to trigger FIXED_LSHIFT_EXPR logic */
    for (int i = 0; i < shift; i++) {
        /* Left shift on fixed-point type */
        result = result << 1;
        
        /* Memory barrier between operations */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Function that mixes fixed-point with integer promotions */
_Accum mixed_operations(short _Fract f, int multiplier) {
    _Accum result;
    
    /* Mix fixed-point with integer - may trigger wider intermediate calculations */
    result = (_Accum)f * multiplier;
    
    /* Additional shift operation */
    result = result << 2;
    
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Main function with various fixed-point operations */
int main(int argc, char *argv[]) {
    /* Use argc to make loop count non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) % 10 : 5;
    if (iterations < 2) iterations = 2;
    
    /* Array of fixed-point values with different types */
    short _Fract sf_array[10];
    _Accum accum_array[10];
    long _Fract lf_array[10];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 10; i++) {
        /* Values close to 1.0 to increase overflow potential */
        sf_array[i] = (short _Fract)(0.9r - (i * 0.05r));
        accum_array[i] = (_Accum)(0.8k + (i * 0.02k));
        lf_array[i] = (long _Fract)(0.95lr - (i * 0.03lr));
    }
    
    /* Results accumulator */
    _Accum total_result = 0.0k;
    
    /* Perform series of fixed-point operations in loop */
    for (int i = 0; i < iterations; i++) {
        /* Operation 1: Multiplication with potential overflow */
        short _Fract a = get_volatile_fract();
        short _Fract b = sf_array[i % 10];
        long _Fract mult_result = process_fixed_mult(a, b);
        
        /* Convert to narrower type - may trigger saturation check */
        short _Fract narrow_result = (short _Fract)mult_result;
        
        /* Operation 2: Left shift operations */
        _Accum shift_val = accum_array[i % 10];
        _Accum shift_result = process_fixed_shift(shift_val, i + 1);
        
        /* Operation 3: Mixed operations */
        _Accum mixed_result = mixed_operations(sf_array[(i + 1) % 10], i + 2);
        
        /* Combine results with potential overflow */
        total_result = total_result + shift_result + mixed_result;
        
        /* Assign to array element to force storage */
        accum_array[i % 10] = narrow_result + shift_result;
        
        /* Memory barrier between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Additional complex expression that combines multiple operations */
    _Accum complex_expr = 0.0k;
    for (int i = 0; i < iterations; i++) {
        /* Chain of operations that should create wide intermediate results */
        complex_expr = (complex_expr << 1) * (_Accum)sf_array[i % 10];
        complex_expr = complex_expr + (_Accum)lf_array[i % 10] * 3;
        
        /* Force integer promotion */
        complex_expr = complex_expr * (i + 1);
    }
    
    /* Final assignment that might trigger saturation */
    short _Fract final_narrow = (short _Fract)(total_result + complex_expr);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", (double)final_narrow);
    
    return 0;
}

/* Additional function with saturation-prone operations */
void saturation_test(void) {
    /* Values that when multiplied exceed 1.0 */
    long _Fract a = 0.99lr;
    long _Fract b = 0.99lr;
    
    /* This multiplication should trigger overflow checking */
    long _Fract product = a * b;
    
    /* Assign to narrower type with -fsat-conversion */
    short _Fract narrow = (short _Fract)product;
    
    /* Use volatile to force the operation */
    volatile short _Fract v_narrow = narrow;
    
    /* Additional shift operations */
    _Accum acc = 0.5k;
    for (int i = 0; i < 4; i++) {
        acc = acc << 2;  /* This should trigger FIXED_LSHIFT_EXPR logic */
    }
    
    asm volatile ("" : : : "memory");
}
