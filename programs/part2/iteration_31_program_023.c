/* test_i386_condition_codes.c
 * Designed to generate RTL with UNORDERED, ORDERED, and other FP condition codes
 * for i386 backend coverage testing.
 */

#include <stdio.h>
#include <math.h>

/* Prevent inlining to ensure separate RTL generation */
__attribute__((noinline))
static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    
    /* Loop to create multiple RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED checks - using various methods */
        if (__builtin_isunordered(a, b)) {
            counter++;  /* Side effect to keep branch */
        }
        
        if (a != a) {  /* NaN check - often generates unordered */
            counter++;
        }
        
        /* ORDERED checks */
        if (!__builtin_isunordered(a, c)) {
            counter++;
        }
        
        /* UNEQ (unordered or equal) */
        if (__builtin_isunordered(a, b) || a == b) {
            counter++;
        }
        
        /* Standard comparisons that may generate UNGE/UNGT/UNLE/UNLT/LTGT */
        if (a >= b) {  /* May generate UNGE */
            counter++;
        }
        
        if (a > b) {   /* May generate UNGT */
            counter++;
        }
        
        if (a <= c) {  /* May generate UNLE */
            counter++;
        }
        
        if (a < c) {   /* May generate UNLT */
            counter++;
        }
        
        if (a != b) {  /* May generate LTGT when both are NaN */
            counter++;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent constant folding */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c));
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;      /* Quiet NaN */
    volatile double inf_val = 1.0 / 0.0;      /* Infinity */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    result += fp_test(nan_val, normal1, normal2);
    result += fp_test(inf_val, nan_val, normal1);
    result += fp_test(normal1, normal2, nan_val);
    result += fp_test(__builtin_nan(""), normal1, inf_val);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
