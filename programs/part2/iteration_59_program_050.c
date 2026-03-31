/* fixed-point-test.c
 * Designed to trigger GCC's fixed-value.cc lines 264-277
 * Compile with: gcc -O1 -fsat-conversion -std=c11 -ffixed-point fixed-point-test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Memory barrier to prevent optimization */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function with fixed-point operations that should trigger range checking */
static short _Fract process_fract(short _Fract a, short _Fract b, int shift) {
    /* Multiplication that may overflow short _Fract range */
    short _Fract prod = a * b;
    MEMORY_BARRIER();
    
    /* Left shift - FIXED_LSHIFT_EXPR */
    short _Fract shifted = prod << shift;
    MEMORY_BARRIER();
    
    /* Mix with integer promotion */
    long _Fract widened = (long _Fract)shifted * 256L;
    MEMORY_BARRIER();
    
    /* Convert back with potential overflow */
    short _Fract result = (short _Fract)widened;
    MEMORY_BARRIER();
    
    return result;
}

/* Function using _Accum types with shifting */
static _Accum process_accum(_Accum a, _Accum b, int shift) {
    /* Multiplication with _Accum types */
    _Accum prod = a * b;
    MEMORY_BARRIER();
    
    /* Left shift operation */
    _Accum shifted = prod << shift;
    MEMORY_BARRIER();
    
    /* Operation that promotes to wider type */
    long _Accum temp = (long _Accum)shifted * 65536lk;
    MEMORY_BARRIER();
    
    /* Narrow conversion that requires range checking */
    _Accum result = (_Accum)temp;
    MEMORY_BARRIER();
    
    return result;
}

/* Process array with mixed fixed-point operations */
static void process_array(short _Fract *arr, int size, int shift) {
    volatile short _Fract v1 = 0.7r;  /* Prevent constant folding */
    volatile short _Fract v2 = 0.9r;
    
    for (int i = 0; i < size; i++) {
        /* Use volatile values to prevent compile-time evaluation */
        short _Fract a = v1;
        short _Fract b = v2;
        
        /* Complex expression that should trigger fixed-value analysis */
        arr[i] = process_fract(a, b, shift + (i & 3));
        
        /* Update volatiles slightly */
        v1 = v1 * 0.98r;
        v2 = v2 * 0.99r;
        MEMORY_BARRIER();
    }
}

/* Test unsigned fixed-point types as well */
static unsigned short _Fract process_ufract(unsigned short _Fract a, 
                                          unsigned short _Fract b,
                                          int shift) {
    /* Operations on unsigned fixed-point */
    unsigned short _Fract prod = a * b;
    MEMORY_BARRIER();
    
    unsigned short _Fract shifted = prod << shift;
    MEMORY_BARRIER();
    
    /* Mix with signed to create complex type interactions */
    short _Fract mixed = (short _Fract)shifted * 0.5r;
    MEMORY_BARRIER();
    
    return (unsigned short _Fract)mixed;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0 || iterations > 100) iterations = 10;
    
    /* Array of fixed-point values */
    short _Fract fract_array[100];
    _Accum accum_array[50];
    
    /* Initialize with pattern */
    for (int i = 0; i < 100; i++) {
        fract_array[i] = (short _Fract)((i % 10) * 0.1r);
    }
    
    for (int i = 0; i < 50; i++) {
        accum_array[i] = (_Accum)((i % 20) * 0.05k);
    }
    
    /* Process arrays multiple times with different shifts */
    for (int iter = 0; iter < iterations; iter++) {
        int shift = (iter % 4) + 1;  /* Vary shift amount */
        
        /* Process fract array */
        process_array(fract_array, 50, shift);
        
        /* Process accum array with similar pattern */
        volatile _Accum acc1 = 0.5k;
        volatile _Accum acc2 = 0.8k;
        
        for (int i = 0; i < 25; i++) {
            _Accum a = acc1;
            _Accum b = acc2;
            
            /* Complex accum operations */
            accum_array[i] = process_accum(a, b, shift);
            
            /* Update volatiles */
            acc1 = acc1 * 0.95k;
            acc2 = acc2 * 0.97k;
            MEMORY_BARRIER();
        }
        
        /* Test unsigned fixed-point */
        volatile unsigned short _Fract uf1 = 0.8ur;
        volatile unsigned short _Fract uf2 = 0.9ur;
        
        for (int i = 25; i < 50; i++) {
            unsigned short _Fract a = uf1;
            unsigned short _Fract b = uf2;
            
            /* This assignment to narrower type may trigger overflow check */
            fract_array[i] = (short _Fract)process_ufract(a, b, shift);
            
            uf1 = uf1 * 0.96ur;
            uf2 = uf2 * 0.94ur;
            MEMORY_BARRIER();
        }
    }
    
    /* Compute sum to prevent dead code elimination */
    _Accum total = 0.0k;
    for (int i = 0; i < 100; i++) {
        total += (_Accum)fract_array[i];
    }
    for (int i = 0; i < 50; i++) {
        total += accum_array[i];
    }
    
    /* Print something to ensure code isn't optimized away */
    printf("Result: %k\n", total);
    
    return 0;
}
