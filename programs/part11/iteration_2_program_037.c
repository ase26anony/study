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
        
        /* Use __builtin_isunordered to generate UNORDERED condition */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Force condition code output via inline assembly */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative using direct comparison with unordered semantics */
        int cmp_result2 = !(a == a) || !(b == b);
        asm volatile("" : : "g"(cmp_result2));
        
        dummy += cmp_result;
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate ORDERED condition */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: ordered comparison */
        int cmp_result2 = (a == a) && (b == b);
        asm volatile("" : : "g"(cmp_result2));
        
        dummy += cmp_result;
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNEQ: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test UNGE condition code (unordered or greater than or equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGE: unordered OR (a >= b) */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Force condition code output - should generate "nlt" */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGT: unordered OR (a > b) */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Force condition code output - should generate "nle" */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLE: unordered OR (a <= b) */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code output - should generate "ule" */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLT: unordered OR (a < b) */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Force condition code output - should generate "ult" */
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate LTGT using __builtin_islessgreater */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Force condition code output - should generate "une" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: (a < b) || (a > b) but not unordered */
        int cmp_result2 = (a < b) || (a > b);
        asm volatile("" : : "g"(cmp_result2));
        
        dummy += cmp_result;
    }
}

/* Additional test with float type for variety */
__attribute__((noinline)) void test_float_comparisons() {
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
        
        /* Test various float comparisons */
        int cmp1 = __builtin_isunordered(a, b);
        int cmp2 = !__builtin_isunordered(a, b);
        int cmp3 = __builtin_islessgreater(a, b);
        int cmp4 = __builtin_isunordered(a, b) || (a >= b);
        int cmp5 = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code outputs */
        asm volatile("" : : "g"(cmp1));
        asm volatile("" : : "g"(cmp2));
        asm volatile("" : : "g"(cmp3));
        asm volatile("" : : "g"(cmp4));
        asm volatile("" : : "g"(cmp5));
        
        dummy += cmp1 + cmp2 + cmp3 + cmp4 + cmp5;
    }
}

int main() {
    printf("Starting floating-point condition code tests...\n");
    
    /* Call all test functions in sequence */
    test_unordered();      /* Should trigger "unord" */
    test_ordered();        /* Should trigger "ord" */
    test_uneq();           /* Should trigger "ueq" */
    test_unge();           /* Should trigger "nlt" */
    test_ungt();           /* Should trigger "nle" */
    test_unle();           /* Should trigger "ule" */
    test_unlt();           /* Should trigger "ult" */
    test_ltgt();           /* Should trigger "une" */
    
    /* Additional tests with float type */
    test_float_comparisons();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %lu\n", (unsigned long)dummy);
    
    return 0;
}
