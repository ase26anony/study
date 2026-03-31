/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Function with fixed-point parameters to prevent constant folding */
volatile _Accum external_source = 0.5k;

/* Array operations to create loop-variant values */
void process_fixed_array(short _Fract *arr, int size) {
    for (int i = 0; i < size; i++) {
        /* Force non-constant values */
        short _Fract val = (short _Fract)(external_source * i);
        
        /* Multiplication that could overflow */
        short _Fract a = 0.8r;
        short _Fract b = 0.9r;
        short _Fract c = a * b;
        
        /* Left shift operation (FIXED_LSHIFT_EXPR) */
        _Accum accum_val = (_Accum)val;
        accum_val = accum_val << 2;
        
        /* Store with potential overflow */
        arr[i] = c + (short _Fract)accum_val;
        
        /* Memory barrier to prevent optimization */
        asm volatile ("" : : : "memory");
    }
}

/* Function to trigger saturation checks with mixed types */
long _Fract complex_multiplication(long _Fract a, long _Fract b) {
    /* Multiplication with wide intermediate */
    long _Fract result = a * b;
    
    /* Left shift to trigger FIXED_LSHIFT_EXPR logic */
    result = result << 1;
    
    /* Assign to narrower type to force range checking */
    short _Fract narrow_result = (short _Fract)result;
    
    asm volatile ("" : : : "memory");
    return result;
}

/* Function with integer promotions */
_Accum mixed_operations(int scale) {
    _Accum base = 0.7k;
    
    /* Multiplication with integer promotion */
    _Accum scaled = base * scale;
    
    /* Left shift operation */
    scaled = scaled << 3;
    
    /* Another multiplication that could overflow */
    _Accum multiplier = 1.5k;
    _Accum result = scaled * multiplier;
    
    /* Force saturation check by assigning to narrower type */
    short _Fract narrowed = (short _Fract)result;
    
    asm volatile ("" : : : "memory");
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    if (iterations > 100) iterations = 100;
    
    /* Array of fixed-point values */
    short _Fract arr[100];
    
    /* Initialize with pattern */
    for (int i = 0; i < 100; i++) {
        arr[i] = (short _Fract)(i * 0.01r);
    }
    
    /* Process array with fixed-point operations */
    process_fixed_array(arr, iterations);
    
    /* Perform complex multiplications */
    long _Fract total = 0.0lr;
    for (int i = 0; i < iterations; i++) {
        long _Fract a = (long _Fract)(external_source * i);
        long _Fract b = 0.95lr;
        
        /* This multiplication can overflow */
        total = total + complex_multiplication(a, b);
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Mixed operations with integer promotion */
    _Accum accum_result = 0.0k;
    for (int i = 0; i < iterations; i++) {
        accum_result = accum_result + mixed_operations(i);
        
        /* Prevent optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Additional test cases for specific scenarios */
    
    /* Test 1: Multiplication near saturation bounds */
    short _Fract sf1 = 0.99r;
    short _Fract sf2 = 0.99r;
    short _Fract sf3 = sf1 * sf2;  /* Should be close to max */
    
    /* Test 2: Left shift that could overflow */
    _Accum acc1 = 0.8k;
    acc1 = acc1 << 4;  /* Shift by significant amount */
    
    /* Test 3: Narrowing conversion with potential overflow */
    long _Fract lf1 = 0.999lr;
    long _Fract lf2 = 0.999lr;
    short _Fract narrow = (short _Fract)(lf1 * lf2);
    
    /* Test 4: Multiple operations in sequence */
    _Accum chain = 0.5k;
    for (int i = 0; i < 5; i++) {
        chain = chain * 1.2k;
        chain = chain << 1;
        asm volatile ("" : : : "memory");
    }
    
    /* Print results to prevent dead code elimination */
    printf("Result: %d %d %d\n", 
           (int)(sf3 * 1000), 
           (int)(accum_result * 1000),
           (int)(total * 1000));
    
    return 0;
}
