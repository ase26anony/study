/* Test program to generate RTL with various floating-point condition codes
   for i386 backend coverage */
#include <stdio.h>
#include <math.h>

/* Prevent inlining to keep RTL structure intact */
__attribute__((noinline))
static int fp_test(volatile double a, volatile double b, volatile double c) {
    volatile int counter = 0;
    volatile double x = a;
    volatile double y = b;
    volatile double z = c;
    
    /* Use inline assembly to prevent optimization */
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
        if (__builtin_isunordered(y, z) || y < z) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* LTGT condition: less-than or greater-than (but not equal/unordered) */
        if ((x < y) || (x > y)) {
            counter++;
            asm volatile("" : : : "memory");
        }
        
        /* Standard comparisons mixed in */
        if (x == z) counter++;
        if (x != y) counter++;
        if (x < y) counter++;
        if (x > z) counter++;
        if (x <= y) counter++;
        if (x >= z) counter++;
        
        /* Modify values to create data dependencies */
        x = y + 1.0;
        y = z * 2.0;
        z = x / 2.0;
        asm volatile("" : "+g"(x), "+g"(y), "+g"(z) : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* Create NaN values to trigger unordered comparisons */
    volatile double nan_val = 0.0 / 0.0;
    volatile double inf_val = 1.0 / 0.0;
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Call with various combinations including NaN */
    int result = 0;
    result += fp_test(nan_val, normal1, normal2);
    result += fp_test(normal1, nan_val, inf_val);
    result += fp_test(inf_val, normal2, nan_val);
    result += fp_test(zero, inf_val, normal1);
    result += fp_test(normal1, normal2, zero);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
