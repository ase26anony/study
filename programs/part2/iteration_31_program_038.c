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
    
    /* Loop to generate multiple RTL instructions */
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
        if (__builtin_isunordered(x, y) || x >= y) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* UNGT condition: unordered or greater-than */
        if (__builtin_isunordered(x, y) || x > y) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* UNLE condition: unordered or less-or-equal */
        if (__builtin_isunordered(x, y) || x <= y) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* UNLT condition: unordered or less-than */
        if (__builtin_isunordered(x, y) || x < y) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* LTGT condition: less-than or greater-than (but not equal, not unordered) */
        if ((x < y) || (x > y)) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* Standard comparisons to mix with special conditions */
        if (x == z) counter++;
        if (x != z) counter++;
        if (x < z) counter++;
        if (x > z) counter++;
        if (x <= z) counter++;
        if (x >= z) counter++;
        
        /* Modify values slightly to create variation */
        x += 0.1;
        y -= 0.1;
        asm volatile("" : "+g"(x), "+g"(y) : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* Initialize with NaN, normal, and infinity values */
    volatile double nan_val = 0.0 / 0.0;          /* Generate a NaN */
    volatile double normal_val = 3.14159;
    volatile double inf_val = 1.0 / 0.0;          /* Generate infinity */
    
    /* Call the test function multiple times with different combinations */
    int result = 0;
    result += fp_test(nan_val, normal_val, inf_val);
    result += fp_test(normal_val, nan_val, normal_val);
    result += fp_test(inf_val, inf_val, nan_val);
    result += fp_test(__builtin_nan(""), 0.0, 1.0);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
