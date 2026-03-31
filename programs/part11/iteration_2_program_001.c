#include <stdio.h>
#include <math.h>

#define SIZE 1024
#define NAN_INTERVAL 4

/* Global arrays with mixed normal values and NaNs */
double array1[SIZE];
double array2[SIZE];

/* Dummy variable to prevent optimization */
volatile int dummy = 0;

/* Initialize arrays with sequential values and NaNs */
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        /* Every NAN_INTERVAL-th element in array2 is NaN */
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 2.5;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Use __builtin_isunordered to generate UNORDERED condition code */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Force condition code output through inline assembly */
        asm volatile("" : : "g"(cmp_result));
        
        /* Also use direct comparison with unordered semantics */
        int cmp_result2 = (a != a) || (b != b);
        asm volatile("" : : "g"(cmp_result2));
        
        dummy += cmp_result;
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* ORDERED is the opposite of UNORDERED */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNEQ: unordered or equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test UNGE condition code (unordered or greater-or-equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGE: unordered or greater-or-equal (maps to "nlt" in output) */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test UNGT condition code (unordered or greater) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGT: unordered or greater (maps to "nle" in output) */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test UNLE condition code (unordered or less-or-equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLE: unordered or less-or-equal (maps to "ule" in output) */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test UNLT condition code (unordered or less) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLT: unordered or less (maps to "ult" in output) */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test LTGT condition code (less or greater, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* LTGT: less or greater, but not equal and not unordered (maps to "une" in output) */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative implementation using direct comparisons */
        int cmp_result2 = (a < b) || (a > b);
        asm volatile("" : : "g"(cmp_result2));
        
        dummy += cmp_result;
    }
}

/* Main function that calls all test functions */
int main() {
    printf("Starting condition code coverage test...\n");
    
    /* Call all test functions to generate various condition codes */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    
    /* Use dummy to prevent dead code elimination */
    printf("Dummy checksum: %d\n", dummy);
    
    return 0;
}
