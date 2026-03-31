/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force unordered comparisons by using NaN values */
#define SET_NAN(d) (d) = __builtin_nan("")

/* Inline assembly with %C constraint to directly trigger condition code printing */
static inline void emit_condition_code(int cond, int *dest, int src) {
    /* Use %C to print condition code name */
    asm volatile ("cmov%C0 %2, %0\n\t"
                  : "+r" (*dest)
                  : "i" (cond), "r" (src)
                  : "cc");
}

/* Another variant with different constraint */
static inline void test_cc_printing(int cond) {
    int a = 1, b = 2;
    asm volatile ("# Condition code test: %C0\n\t"
                  : : "i" (cond), "r" (a), "r" (b)
                  : "cc");
}

int main(void) {
    volatile int cc_accumulator = 0;
    double arr1[256], arr2[256];
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = i * 2.0;
        
        /* Insert NaN at specific indices to force unordered comparisons */
        if (i % 7 == 0) {
            SET_NAN(arr1[i]);
        }
        if (i % 11 == 0) {
            SET_NAN(arr2[i]);
        }
        if (i % 13 == 0) {
            SET_NAN(arr1[i]);
            SET_NAN(arr2[i]);
        }
    }
    
    /* Perform various floating-point comparisons to generate different condition codes */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        int temp;
        
        /* Generate UNORDERED condition (NaN comparison) */
        if (a < b) {
            temp = 1;
        } else {
            temp = 0;
        }
        cc_accumulator += temp;
        
        /* Generate ORDERED condition */
        if (!isunordered(a, b)) {
            temp = 1;
        } else {
            temp = 0;
        }
        cc_accumulator += temp;
        
        /* Generate UNEQ (unordered or equal) */
        if (!(a > b) && !(a < b)) {
            temp = 1;
        } else {
            temp = 0;
        }
        cc_accumulator += temp;
        
        /* Generate UNGE (unordered or greater or equal) */
        if (!(a < b)) {
            temp = 1;
        } else {
            temp = 0;
        }
        cc_accumulator += temp;
        
        /* Generate UNGT (unordered or greater) */
        if (!(a <= b)) {
            temp = 1;
        } else {
            temp = 0;
        }
        cc_accumulator += temp;
        
        /* Generate UNLE (unordered or less or equal) */
        if (!(a > b)) {
            temp = 1;
        } else {
            temp = 0;
        }
        cc_accumulator += temp;
        
        /* Generate UNLT (unordered or less) */
        if (!(a >= b)) {
            temp = 1;
        } else {
            temp = 0;
        }
        cc_accumulator += temp;
        
        /* Generate LTGT (less or greater) */
        if (a != b) {
            temp = 1;
        } else {
            temp = 0;
        }
        cc_accumulator += temp;
    }
    
    /* Direct inline assembly calls with different condition codes */
    /* These should directly trigger the printing logic via %C */
    int x = 0, y = 100;
    
    /* Test all condition codes from the uncovered block */
    emit_condition_code(0, &x, y);  /* UNORDERED */
    test_cc_printing(0);
    
    emit_condition_code(7, &x, y);  /* ORDERED */
    test_cc_printing(7);
    
    emit_condition_code(8, &x, y);  /* UNEQ */
    test_cc_printing(8);
    
    emit_condition_code(13, &x, y); /* UNGE */
    test_cc_printing(13);
    
    emit_condition_code(14, &x, y); /* UNGT */
    test_cc_printing(14);
    
    emit_condition_code(15, &x, y); /* UNLE */
    test_cc_printing(15);
    
    emit_condition_code(16, &x, y); /* UNLT */
    test_cc_printing(16);
    
    emit_condition_code(17, &x, y); /* LTGT */
    test_cc_printing(17);
    
    /* Additional tests with explicit floating-point comparisons in inline asm */
    double d1 = __builtin_nan(""), d2 = 3.14;
    int result;
    
    /* FCOMI comparison that sets condition codes */
    asm volatile ("fcomi %%st(1), %%st(0)\n\t"
                  "setp %%al\n\t"
                  : "=a" (result)
                  : "t" (d1), "u" (d2)
                  : "cc");
    
    /* More complex floating-point comparison sequence */
    asm volatile ("# Complex FP comparison sequence\n\t"
                  "fldl %1\n\t"
                  "fldl %2\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "fstp %%st(0)\n\t"
                  : : "m" (d1), "m" (d2)
                  : "cc");
    
    printf("Condition code accumulator: %d\n", cc_accumulator);
    printf("Final x value: %d\n", x);
    
    return 0;
}
