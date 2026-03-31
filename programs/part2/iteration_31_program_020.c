/* test_i386_condition_codes.c
 * Generates RTL with various floating-point condition codes for i386 backend
 */

#include <stdio.h>
#include <math.h>

/* Prevent inlining to keep RTL structure intact */
__attribute__((noinline))
static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    
    /* Create multiple basic blocks with different FP comparisons */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED checks (x != x is true for NaN) */
        if (__builtin_isunordered(x, y)) {
            counter++;
            /* Use inline asm to prevent optimization */
            asm volatile("" : "+g"(counter) : : "memory");
        }
        
        /* ORDERED checks */
        if (!__builtin_isunordered(x, z)) {
            counter += 2;
            asm volatile("" : "+g"(counter) : : "memory");
        }
        
        /* UNEQ (unordered or equal) - using explicit check */
        if (__builtin_isunordered(x, y) || x == y) {
            counter += 3;
        }
        
        /* UNGE (not less than) - generates NLT condition */
        if (!(x < y)) {
            counter += 4;
        }
        
        /* UNGT (not less than or equal) - generates NLE condition */
        if (!(x <= y)) {
            counter += 5;
        }
        
        /* UNLE (unordered or less than or equal) */
        if (__builtin_isunordered(x, z) || x <= z) {
            counter += 6;
        }
        
        /* UNLT (unordered or less than) */
        if (__builtin_isunordered(y, z) || y < z) {
            counter += 7;
        }
        
        /* LTGT (less than or greater than) - generates UNE condition */
        if (x < z || x > z) {
            counter += 8;
        }
        
        /* Mix in some regular comparisons */
        if (x == y) counter--;
        if (x < y) counter--;
        if (x > y) counter--;
        if (x <= y) counter--;
        if (x >= y) counter--;
        
        /* Modify values to create data dependencies */
        asm volatile("" : "+g"(x), "+g"(y), "+g"(z));
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;  /* Quiet NaN */
    volatile double inf_val = 1.0 / 0.0;  /* Infinity */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    result += fp_test(nan_val, normal1, normal2);
    result += fp_test(normal1, nan_val, normal2);
    result += fp_test(normal1, normal2, nan_val);
    result += fp_test(inf_val, normal1, normal2);
    result += fp_test(normal1, inf_val, normal2);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
