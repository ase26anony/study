#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

/* Global variables to prevent optimization */
volatile int global_dummy = 0;
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Initialize arrays with mixed normal values and NaNs */
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i) * 2.0;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNORDERED condition code */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate ORDERED condition code (opposite of unordered) */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNEQ condition code using multiple comparisons */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater-or-equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGE condition code - maps to "nlt" (not less than) */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGT condition code - maps to "nle" (not less-or-equal) */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less-or-equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLE condition code - maps to "ule" */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLT condition code - maps to "ult" */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less or greater, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate LTGT condition code - maps to "une" (unordered or not equal) */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Additional test with direct comparisons to ensure all paths are hit */
__attribute__((noinline)) void test_mixed_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Test various comparison operators that might generate different condition codes */
        int cmp1 = (a != b);  /* NE */
        int cmp2 = (a < b);   /* LT */
        int cmp3 = (a > b);   /* GT */
        int cmp4 = (a <= b);  /* LE */
        int cmp5 = (a >= b);  /* GE */
        
        /* Force all condition codes through inline asm */
        asm volatile("" : : "g"(cmp1), "g"(cmp2), "g"(cmp3), "g"(cmp4), "g"(cmp5));
    }
}

int main() {
    /* Call all test functions to generate the condition codes */
    test_unordered();      /* Should generate "unord" */
    test_ordered();        /* Should generate "ord" */
    test_uneq();           /* Should generate "ueq" */
    test_unge();           /* Should generate "nlt" */
    test_ungt();           /* Should generate "nle" */
    test_unle();           /* Should generate "ule" */
    test_unlt();           /* Should generate "ult" */
    test_ltgt();           /* Should generate "une" */
    test_mixed_comparisons();
    
    /* Use the results to prevent dead code elimination */
    int checksum = global_dummy;
    
    /* Print something to ensure the program runs */
    printf("Condition code test completed. Checksum: %d\n", checksum);
    
    return 0;
}
