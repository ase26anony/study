/* Test program to cover i386 condition code printing for UNORDERED, ORDERED, etc. */
#include <stdio.h>
#include <math.h>

/* Prevent inlining to ensure separate RTL generation */
__attribute__((noinline))
static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    
    /* Create multiple basic blocks with different FP comparisons */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED checks - using isnan and isunordered */
        if (__builtin_isnan(x)) {
            counter++;
        }
        
        if (__builtin_isunordered(x, y)) {
            counter += 2;
        }
        
        /* ORDERED check - the opposite of unordered */
        if (!__builtin_isunordered(x, y)) {
            counter += 3;
        }
        
        /* UNEQ: unordered or equal */
        if (__builtin_isunordered(x, z) || x == z) {
            counter += 4;
        }
        
        /* UNGE: unordered or greater-or-equal (not less than) */
        if (__builtin_isunordered(x, y) || x >= y) {
            counter += 5;
        }
        
        /* UNGT: unordered or greater-than (not less-or-equal) */
        if (__builtin_isunordered(x, y) || x > y) {
            counter += 6;
        }
        
        /* UNLE: unordered or less-or-equal */
        if (__builtin_isunordered(x, y) || x <= y) {
            counter += 7;
        }
        
        /* UNLT: unordered or less-than */
        if (__builtin_isunordered(x, y) || x < y) {
            counter += 8;
        }
        
        /* LTGT: less-than or greater-than (not equal and not unordered) */
        if (x < y || x > y) {
            counter += 9;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values to create data dependencies */
        x = y + 1.0;
        y = z * 2.0;
        z = x / 2.0;
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;  /* Generate a NaN */
    volatile double inf_val = 1.0 / 0.0;  /* Generate infinity */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Call test function multiple times with different combinations */
    int result = 0;
    result += fp_test(nan_val, normal1, normal2);
    result += fp_test(inf_val, nan_val, normal1);
    result += fp_test(normal1, normal2, nan_val);
    result += fp_test(__builtin_nan(""), __builtin_inf(), 0.0);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
