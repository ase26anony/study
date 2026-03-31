#include <stdio.h>
#include <stdint.h>
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
            array2[i] = (double)(i + 1) * 0.75;
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

/* Alternative test using direct comparison operators to generate different patterns */
__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Test various direct comparisons that might generate different condition codes */
        int cmp1 = (a < b);   /* Might generate different CC */
        int cmp2 = (a > b);   /* Might generate different CC */
        int cmp3 = (a <= b);  /* Might generate different CC */
        int cmp4 = (a >= b);  /* Might generate different CC */
        int cmp5 = (a == b);  /* Might generate different CC */
        int cmp6 = (a != b);  /* Might generate different CC */
        
        /* Force all condition codes through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
        asm volatile("" : "+r"(dummy) : "g"(cmp6));
    }
}

int main() {
    /* Call all test functions to generate various condition codes */
    test_unordered();      /* Should trigger "unord" output */
    test_ordered();        /* Should trigger "ord" output */
    test_uneq();           /* Should trigger "ueq" output */
    test_unge();           /* Should trigger "nlt" output */
    test_ungt();           /* Should trigger "nle" output */
    test_unle();           /* Should trigger "ule" output */
    test_unlt();           /* Should trigger "ult" output */
    test_ltgt();           /* Should trigger "une" output */
    test_direct_comparisons(); /* Additional comparisons for coverage */
    
    /* Compute checksum from dummy to prevent dead code elimination */
    int checksum = dummy;
    
    /* Print checksum to ensure code isn't optimized away */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
