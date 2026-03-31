/* Test program to generate RTL with various floating-point condition codes
   for i386 backend coverage */
#include <stdio.h>
#include <math.h>

/* Prevent inlining to ensure separate RTL generation */
__attribute__((noinline))
static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    
    /* Create data dependencies to prevent optimization */
    asm volatile("" : "+g"(x), "+g"(y), "+g"(z) : : "memory");
    
    /* Loop to generate more RTL instructions */
    for (int i = 0; i < 10; i++) {
        /* UNORDERED condition: check for NaN */
        if (__builtin_isunordered(x, y)) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* ORDERED condition: check both are not NaN */
        if (!__builtin_isunordered(x, z)) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* UNEQ condition: unordered or equal */
        if (__builtin_isunordered(x, y) || x == y) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* UNGE condition: unordered or greater-or-equal */
        if (__builtin_isunordered(x, z) || x >= z) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* UNGT condition: unordered or greater-than */
        if (__builtin_isunordered(y, z) || y > z) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* UNLE condition: unordered or less-or-equal */
        if (__builtin_isunordered(x, y) || x <= y) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* UNLT condition: unordered or less-than */
        if (__builtin_isunordered(x, z) || x < z) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* LTGT condition: less-than or greater-than (but not equal, not unordered) */
        if ((x < z) || (x > z)) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* Standard comparisons to ensure other condition codes are generated */
        if (x == y) counter++;
        if (x != z) counter++;
        if (x < y) counter++;
        if (x > z) counter++;
        if (x <= y) counter++;
        if (x >= z) counter++;
        
        /* Modify values to create varying conditions */
        x += 1.0;
        y -= 0.5;
        asm volatile("" : "+g"(x), "+g"(y) : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN and normal values */
    volatile double nan_val = 0.0 / 0.0;  /* Generate a NaN */
    volatile double inf_val = 1.0 / 0.0;  /* Generate infinity */
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    
    /* Prevent constant folding */
    asm volatile("" : "+g"(nan_val), "+g"(inf_val), 
                       "+g"(normal1), "+g"(normal2) : : "memory");
    
    /* Call test function with different combinations */
    int result = 0;
    result += fp_test(nan_val, normal1, normal2);   /* NaN vs normal */
    result += fp_test(inf_val, nan_val, normal1);   /* Inf vs NaN */
    result += fp_test(normal1, normal2, inf_val);   /* normal vs Inf */
    result += fp_test(nan_val, nan_val, nan_val);   /* NaN vs NaN */
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
