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
            array2[i] = (double)(i + 1) * 2.5;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline)) void test_unordered() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNORDERED condition code */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Force condition code output through inline assembly */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate ORDERED condition code (not unordered) */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Force condition code output */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNEQ condition code: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Force condition code output */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNGE condition code (unordered or greater than or equal) */
__attribute__((noinline)) void test_unge() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGE condition code: unordered OR a >= b */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Force condition code output - should generate "nlt" */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline)) void test_ungt() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGT condition code: unordered OR a > b */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Force condition code output - should generate "nle" */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline)) void test_unle() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLE condition code: unordered OR a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code output - should generate "ule" */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline)) void test_unlt() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLT condition code: unordered OR a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Force condition code output - should generate "ult" */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate LTGT condition code using __builtin_islessgreater */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Force condition code output - should generate "une" */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Alternative test using direct floating-point comparisons */
__attribute__((noinline)) void test_direct_comparisons() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Test various direct comparisons that might generate different condition codes */
        int cmp1 = (a != b);  /* NE condition */
        int cmp2 = (a < b);   /* LT condition */
        int cmp3 = (a > b);   /* GT condition */
        int cmp4 = (a <= b);  /* LE condition */
        int cmp5 = (a >= b);  /* GE condition */
        
        /* Force multiple condition code outputs */
        asm volatile("" : "+r"(dummy) : "g"(cmp1) : "cc");
        asm volatile("" : "+r"(dummy) : "g"(cmp2) : "cc");
        asm volatile("" : "+r"(dummy) : "g"(cmp3) : "cc");
        asm volatile("" : "+r"(dummy) : "g"(cmp4) : "cc");
        asm volatile("" : "+r"(dummy) : "g"(cmp5) : "cc");
    }
    global_dummy += dummy;
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
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", global_dummy);
    
    return 0;
}
