/* test_i386_condition_codes.c
 * This program generates RTL with various floating-point condition codes
 * to cover the UNORDERED, ORDERED, and related case statements in i386.cc
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
            counter += 1;  /* UNORDERED condition */
        }
        
        /* ORDERED check */
        if (!__builtin_isunordered(a, c)) {
            counter += 2;  /* ORDERED condition */
        }
        
        /* UNEQ (unordered or equal) - a != a is true for NaN */
        if (a != a || a == b) {
            counter += 3;  /* UNEQ condition */
        }
        
        /* UNGE (not less than) - !(a < b) including unordered */
        if (!(a < b)) {
            counter += 4;  /* UNGE condition (nlt) */
        }
        
        /* UNGT (not less than or equal) - !(a <= b) including unordered */
        if (!(a <= b)) {
            counter += 5;  /* UNGT condition (nle) */
        }
        
        /* UNLE (unordered or less than or equal) */
        if (a != a || a <= b) {
            counter += 6;  /* UNLE condition (ule) */
        }
        
        /* UNLT (unordered or less than) */
        if (a != a || a < b) {
            counter += 7;  /* UNLT condition (ult) */
        }
        
        /* LTGT (less than or greater than, but not equal and not unordered) */
        if ((a < b) || (a > b)) {
            counter += 8;  /* LTGT condition (une) */
        }
        
        /* Standard comparisons that might generate different condition codes */
        if (a == b) {
            counter += 9;
        }
        if (a < b) {
            counter += 10;
        }
        if (a > b) {
            counter += 11;
        }
        if (a <= b) {
            counter += 12;
        }
        if (a >= b) {
            counter += 13;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly to prevent complete optimization */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c));
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;  /* Generate a quiet NaN */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double inf_val = 1.0 / 0.0;   /* Infinity */
    
    /* Call the test function multiple times with different combinations */
    int result = 0;
    result += fp_test(nan_val, normal1, normal2);
    result += fp_test(normal1, nan_val, inf_val);
    result += fp_test(normal1, normal2, nan_val);
    result += fp_test(inf_val, -inf_val, nan_val);
    
    /* Use __builtin_nan for another NaN source */
    volatile double builtin_nan = __builtin_nan("");
    result += fp_test(builtin_nan, normal1, normal2);
    
    /* Test with signaling NaN if supported */
    #ifdef __FAST_MATH__
    volatile double signaling_nan = __builtin_nans("");
    result += fp_test(signaling_nan, normal1, normal2);
    #endif
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
