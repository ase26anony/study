/* fixed-point-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant folding */
volatile short _Fract vf1 = 0.5r;
volatile short _Fract vf2 = 0.8r;
volatile _Accum va1 = 0.5k;
volatile _Accum va2 = 1.2k;

/* Function with fixed-point operations that may overflow */
short _Fract process_fract(short _Fract a, short _Fract b) {
    /* Multiplication that may exceed range */
    short _Fract result = a * b;
    
    /* Insert memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Left shift on fixed-point (FIXED_LSHIFT_EXPR) */
    _Accum temp = (_Accum)result;
    temp = temp << 2;  /* This should trigger shift logic */
    
    asm volatile ("" : : : "memory");
    
    /* Convert back with potential overflow */
    return (short _Fract)temp;
}

/* Function with accum operations */
_Accum process_accum(_Accum a, _Accum b) {
    /* Multiplication with wider intermediate */
    _Accum prod = a * b;
    
    asm volatile ("" : : : "memory");
    
    /* Left shift that may overflow */
    prod = prod << 3;
    
    asm volatile ("" : : : "memory");
    
    /* Mix with integer promotion */
    long temp = (long)prod * 256;
    return (_Accum)temp;
}

/* Function that forces saturation checking */
void test_saturation(int iterations) {
    long _Fract lf1 = 0.9lr;
    long _Fract lf2 = 0.95lr;
    
    for (int i = 0; i < iterations; i++) {
        /* Operation that may overflow into narrower type */
        short _Fract narrow_result = (short _Fract)(lf1 * lf2);
        
        /* Modify values slightly each iteration */
        lf1 = lf1 * 0.99lr;
        lf2 = lf2 * 1.01lr;
        
        asm volatile ("" : : : "memory");
        
        /* Use result to prevent dead code elimination */
        volatile short _Fract sink = narrow_result;
    }
}

/* Array-based operations to prevent optimization */
void array_operations(int size) {
    _Accum arr[10];
    short _Fract results[10];
    
    /* Initialize array with pattern */
    for (int i = 0; i < size && i < 10; i++) {
        arr[i] = (_Accum)(i * 0.1k);
    }
    
    /* Perform operations that may trigger bounds checking */
    for (int i = 0; i < size - 1 && i < 9; i++) {
        /* Multiplication with potential overflow */
        _Accum temp = arr[i] * arr[i + 1];
        
        /* Left shift */
        temp = temp << (i % 4);
        
        /* Convert to narrower type */
        results[i] = (short _Fract)temp;
        
        asm volatile ("" : : : "memory");
    }
}

/* Mixed-type operations */
void mixed_operations(int count) {
    for (int i = 0; i < count; i++) {
        /* Mix _Fract with integer */
        int multiplier = i + 1;
        short _Fract f = (short _Fract)(0.5r + i * 0.01r);
        
        /* This should trigger integer promotion logic */
        _Accum result = (_Accum)f * multiplier;
        
        /* Left shift */
        result = result << (multiplier % 3);
        
        /* Store to volatile to prevent optimization */
        volatile _Accum sink = result;
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 2) iterations = 2;
    if (iterations > 100) iterations = 100;
    
    printf("Testing fixed-point operations with %d iterations\n", iterations);
    
    /* Test 1: Basic fixed-point multiplication with volatile */
    short _Fract f1 = vf1;
    short _Fract f2 = vf2;
    short _Fract f_result = process_fract(f1, f2);
    
    /* Test 2: Accum operations */
    _Accum a1 = va1;
    _Accum a2 = va2;
    _Accum a_result = process_accum(a1, a2);
    
    /* Test 3: Saturation checking */
    test_saturation(iterations);
    
    /* Test 4: Array operations */
    array_operations(iterations);
    
    /* Test 5: Mixed operations */
    mixed_operations(iterations);
    
    /* Use results to prevent dead code elimination */
    volatile short _Fract final_f = f_result;
    volatile _Accum final_a = a_result;
    
    printf("Fixed-point test completed\n");
    
    return 0;
}
