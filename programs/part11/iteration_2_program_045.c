#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

/* Global dummy variable to prevent optimization */
volatile uint64_t dummy = 0;

/* Arrays with mixed normal values and NaNs */
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Initialize arrays with pattern including NaNs */
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 2.5;
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
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate ORDERED condition code */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
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
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater-or-equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGE condition code - maps to "nlt" in output */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGT condition code - maps to "nle" in output */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less-or-equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLE condition code - maps to "ule" in output */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLT condition code - maps to "ult" in output */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less or greater, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate LTGT condition code - maps to "une" in output */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Additional test with float type for variety */
__attribute__((noinline)) void test_mixed_types() {
    float farray1[ARRAY_SIZE];
    float farray2[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farray1[i] = (float)(i + 1) * 1.5f;
        if (i % NAN_INTERVAL == 0) {
            farray2[i] = __builtin_nanf("");
        } else {
            farray2[i] = (float)(i + 1) * 2.5f;
        }
    }
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float a = farray1[i];
        float b = farray2[i];
        
        /* Test various condition codes with float */
        int cmp1 = __builtin_isunordered(a, b);
        int cmp2 = (a != b);
        int cmp3 = (a < b);
        int cmp4 = (a > b);
        
        /* Force multiple condition code outputs */
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
    }
}

int main() {
    printf("Starting floating-point condition code tests...\n");
    
    /* Call all test functions in sequence */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed_types();
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)array1[i];
        checksum += (uint64_t)array2[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    printf("Dummy value: %lu\n", dummy);
    
    return 0;
}
