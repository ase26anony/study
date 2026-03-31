#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

/* Global dummy variable to prevent optimization */
volatile int dummy = 0;

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
        int cmp_result = __builtin_isunordered(a, b);
        /* Force condition code output for UNORDERED */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = !__builtin_isunordered(a, b);
        /* Force condition code output for ORDERED */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        /* Force condition code output for UNEQ */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater or equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        /* Force condition code output for UNGE (nlt) */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i <ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        /* Force condition code output for UNGT (nle) */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less or equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        /* Force condition code output for UNLE */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        /* Force condition code output for UNLT */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less or greater, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = __builtin_islessgreater(a, b);
        /* Force condition code output for LTGT (une) */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Alternative implementation using direct comparisons for some codes */
__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Direct comparisons that may generate different condition codes */
        int cmp1 = (a != b);  /* May generate NE/UNE */
        int cmp2 = (a < b);   /* May generate LT/ULT */
        int cmp3 = (a > b);   /* May generate GT/UGT */
        int cmp4 = (a <= b);  /* May generate LE/ULE */
        int cmp5 = (a >= b);  /* May generate GE/UGE */
        
        /* Force multiple condition codes in one function */
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
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
    test_direct_comparisons();
    
    /* Use dummy to prevent dead code elimination */
    printf("Result checksum: %d\n", dummy);
    
    return 0;
}
