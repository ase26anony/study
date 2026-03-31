#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

/* Global arrays with mixed normal values and NaNs */
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Dummy variable to prevent optimization */
volatile int dummy = 0;

/* Initialize arrays with pattern including NaNs */
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        /* Insert NaN at regular intervals in array2 */
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 0.75;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using __builtin_isunordered to generate UNORDERED condition */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using !__builtin_isunordered to generate ORDERED condition */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using == operator with potential NaNs to generate UNEQ */
        int cmp_result = (a == b) || __builtin_isunordered(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater than or equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using >= operator with potential NaNs to generate UNGE */
        int cmp_result = (a >= b) || __builtin_isunordered(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using > operator with potential NaNs to generate UNGT */
        int cmp_result = (a > b) || __builtin_isunordered(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using <= operator with potential NaNs to generate UNLE */
        int cmp_result = (a <= b) || __builtin_isunordered(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using < operator with potential NaNs to generate UNLT */
        int cmp_result = (a < b) || __builtin_isunordered(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using __builtin_islessgreater to generate LTGT condition */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Alternative implementation using direct comparisons for some conditions */
__attribute__((noinline)) void test_mixed_conditions() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Test various comparison operators that might generate different condition codes */
        int cmp1 = (a != b);  /* May generate NE or UNE */
        int cmp2 = !(a < b);  /* May generate NLT for UNGE */
        int cmp3 = !(a <= b); /* May generate NLE for UNGT */
        
        /* Force condition code output for each comparison */
        asm volatile("" : : "g"(cmp1));
        asm volatile("" : : "g"(cmp2));
        asm volatile("" : : "g"(cmp3));
    }
}

int main() {
    /* Call all test functions to generate various condition codes */
    test_unordered();      /* Should generate "unord" */
    test_ordered();        /* Should generate "ord" */
    test_uneq();           /* Should generate "ueq" */
    test_unge();           /* Should generate "nlt" */
    test_ungt();           /* Should generate "nle" */
    test_unle();           /* Should generate "ule" */
    test_unlt();           /* Should generate "ult" */
    test_ltgt();           /* Should generate "une" */
    test_mixed_conditions(); /* Additional coverage */
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    /* Use dummy variable to prevent optimization */
    checksum += dummy;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
