#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024

/* Global arrays with mixed NaN and normal values */
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Dummy variable to prevent optimization */
volatile int dummy = 0;

/* Initialize arrays with pattern including NaNs */
void __attribute__((noinline)) init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        /* Every 4th element is NaN, others are normal */
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 2.5;
        }
    }
}

/* Test UNORDERED condition code */
void __attribute__((noinline)) test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNORDERED condition code */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : : "g"(cmp_result));
        
        /* Also use direct comparison for unordered */
        if (a != a || b != b) {
            asm volatile("" : : "g"(1));
        }
    }
}

/* Test ORDERED condition code */
void __attribute__((noinline)) test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate ORDERED condition code (opposite of unordered) */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        /* Alternative ordered check */
        if (a == a && b == b) {
            asm volatile("" : : "g"(1));
        }
    }
}

/* Test UNEQ condition code (unordered or equal) */
void __attribute__((noinline)) test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNEQ: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        /* Direct comparison that might generate UNEQ */
        if (!(a > b) && !(a < b)) {
            asm volatile("" : "+r"(dummy) : "g"(1));
        }
    }
}

/* Test UNGE condition code (unordered or greater-or-equal) */
void __attribute__((noinline)) test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGE: unordered OR a >= b */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Force condition code output - should print "nlt" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: not less than */
        if (!(a < b)) {
            asm volatile("" : "+r"(dummy) : "g"(1));
        }
    }
}

/* Test UNGT condition code (unordered or greater) */
void __attribute__((noinline)) test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGT: unordered OR a > b */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Force condition code output - should print "nle" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: not less-or-equal */
        if (!(a <= b)) {
            asm volatile("" : "+r"(dummy) : "g"(1));
        }
    }
}

/* Test UNLE condition code (unordered or less-or-equal) */
void __attribute__((noinline)) test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLE: unordered OR a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code output - should print "ule" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Direct comparison */
        if (a <= b) {
            asm volatile("" : "+r"(dummy) : "g"(1));
        }
    }
}

/* Test UNLT condition code (unordered or less) */
void __attribute__((noinline)) test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLT: unordered OR a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Force condition code output - should print "ult" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Direct comparison */
        if (a < b) {
            asm volatile("" : "+r"(dummy) : "g"(1));
        }
    }
}

/* Test LTGT condition code (less or greater, but not equal and not unordered) */
void __attribute__((noinline)) test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate LTGT: less OR greater (but not equal and not unordered) */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Force condition code output - should print "une" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: not equal and both ordered */
        if ((a != b) && !__builtin_isunordered(a, b)) {
            asm volatile("" : "+r"(dummy) : "g"(1));
        }
    }
}

int main() {
    /* Initialize arrays with NaN pattern */
    init_arrays();
    
    /* Call all test functions to generate condition codes */
    test_unordered();    /* Should generate "unord" */
    test_ordered();      /* Should generate "ord" */
    test_uneq();         /* Should generate "ueq" */
    test_unge();         /* Should generate "nlt" */
    test_ungt();         /* Should generate "nle" */
    test_unle();         /* Should generate "ule" */
    test_unlt();         /* Should generate "ult" */
    test_ltgt();         /* Should generate "une" */
    
    /* Use dummy variable to prevent dead code elimination */
    printf("Result: %d\n", dummy);
    
    return 0;
}
