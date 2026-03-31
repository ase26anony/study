/* Test program to generate RTL with various floating-point condition codes */
#include <stdio.h>
#include <math.h>

/* Prevent inlining to ensure separate RTL generation */
__attribute__((noinline))
static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create multiple RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks - using isnan and isunordered */
        if (__builtin_isnan(a)) {
            counter++;
            asm volatile("" : : : "memory");  /* Compiler barrier */
        }
        
        if (__builtin_isunordered(a, b)) {
            counter++;
        }
        
        /* ORDERED check - the opposite of unordered */
        if (!__builtin_isunordered(a, c)) {
            counter++;
        }
        
        /* Standard comparisons that may generate other condition codes */
        if (a < b) {
            counter++;
        }
        
        if (a > c) {
            counter++;
        }
        
        if (a == b) {
            counter++;
        }
        
        /* More complex unordered comparisons */
        if (a != a) {  /* Always false for non-NaN, but compiler may generate UNORDERED */
            counter++;
        }
        
        /* LTGT (unordered and not equal) - using explicit comparison */
        if (a < b || a > b) {  /* Equivalent to a != b but may generate LTGT */
            counter++;
        }
        
        /* Introduce data dependencies to prevent optimization */
        asm volatile("" : "+g"(a), "+g"(b) : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;      /* Generate a NaN */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Call the test function multiple times */
    int result = 0;
    result += fp_test(nan_val, normal1, normal2);
    result += fp_test(normal1, nan_val, normal2);
    result += fp_test(normal1, normal2, nan_val);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
