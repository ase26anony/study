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
        /* Every NAN_INTERVAL-th element is NaN in array2 */
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
        
        /* Generate UNGE condition code: !(a < b) */
        int cmp_result = __builtin_isunordered(a, b) || !(a < b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGT condition code: !(a <= b) */
        int cmp_result = __builtin_isunordered(a, b) || !(a <= b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less-or-equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLE condition code: !(a > b) */
        int cmp_result = __builtin_isunordered(a, b) || !(a > b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLT condition code: !(a >= b) */
        int cmp_result = __builtin_isunordered(a, b) || !(a >= b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less or greater, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate LTGT condition code using builtin */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Alternative formulation that should also generate LTGT */
        int cmp_result2 = (a < b) || (a > b);
        
        /* Force condition code output through both formulations */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

/* Additional test with float type to ensure coverage */
__attribute__((noinline)) void test_float_comparisons() {
    float farray1[ARRAY_SIZE];
    float farray2[ARRAY_SIZE];
    
    /* Initialize float arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farray1[i] = (float)(i + 1) * 1.5f;
        if (i % NAN_INTERVAL == 0) {
            farray2[i] = __builtin_nanf("");
        } else {
            farray2[i] = (float)(i + 1) * 2.5f;
        }
    }
    
    /* Test various float comparisons */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float a = farray1[i];
        float b = farray2[i];
        
        /* Test multiple condition codes with floats */
        int cmp1 = __builtin_isunordered(a, b);
        int cmp2 = !__builtin_isunordered(a, b);
        int cmp3 = __builtin_islessgreater(a, b);
        int cmp4 = (a == b);
        int cmp5 = (a != b);
        
        /* Force condition code outputs */
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
    }
}

int main() {
    /* Call all test functions to generate condition codes */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_float_comparisons();
    
    /* Compute and print checksum to prevent dead code elimination */
    uint64_t checksum = dummy;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)array1[i];
        checksum += (uint64_t)array2[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    return 0;
}
