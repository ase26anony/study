/* Test program to generate RTL with UNORDERED/ORDERED condition codes */
#include <stdio.h>
#include <math.h>

/* Prevent inlining to keep RTL structure */
__attribute__((noinline))
static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    
    /* Create multiple basic blocks with different FP comparisons */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED checks - using various methods */
        if (__builtin_isunordered(a, b)) {
            counter++;  /* Side effect to keep branch */
        }
        
        if (a != a) {  /* Classic NaN check */
            counter++;
        }
        
        /* ORDERED checks */
        if (!__builtin_isunordered(a, c)) {
            counter++;
        }
        
        /* Standard comparisons that might generate other condition codes */
        if (a < b) {
            counter++;
        }
        
        if (a > c) {
            counter++;
        }
        
        if (a == b) {
            counter++;
        }
        
        /* UNEQ: unordered or equal */
        if (!(a < b) && !(a > b)) {
            counter++;
        }
        
        /* LTGT: less than or greater than (ordered and not equal) */
        if ((a < b) || (a > b)) {
            counter++;
        }
        
        /* Compiler barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* Modify values slightly */
        asm volatile("" : "+g"(a), "+g"(b), "+g"(c));
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;      /* Generate a quiet NaN */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Call test function multiple times */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += fp_test(nan_val, normal1, normal2);
        
        /* Mix up the arguments */
        total += fp_test(normal1, nan_val, normal2);
        total += fp_test(normal1, normal2, nan_val);
        
        /* Use __builtin_nan for another NaN source */
        volatile double nan2 = __builtin_nan("");
        total += fp_test(nan2, normal1, normal2);
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", total);
    
    return 0;
}
