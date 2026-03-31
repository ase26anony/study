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
void __attribute__((noinline)) init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        /* Every NAN_INTERVAL-th element in array2 is NaN */
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i) * 2.0;
        }
    }
}

/* Test UNORDERED condition code */
void __attribute__((noinline)) test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use __builtin_isunordered to generate UNORDERED condition */
        int cmp_result = __builtin_isunordered(a, b);
        /* Inline asm that uses condition code */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test ORDERED condition code */
void __attribute__((noinline)) test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* ORDERED is the opposite of UNORDERED */
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (unordered or equal) */
void __attribute__((noinline)) test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNEQ: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater than or equal) */
void __attribute__((noinline)) test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNGE: unordered OR a >= b */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater than) */
void __attribute__((noinline)) test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNGT: unordered OR a > b */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
void __attribute__((noinline)) test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNLE: unordered OR a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less than) */
void __attribute__((noinline)) test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNLT: unordered OR a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
void __attribute__((noinline)) test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* LTGT: a != b AND ordered (not unordered) */
        int cmp_result = !__builtin_isunordered(a, b) && (a != b);
        /* Alternative using __builtin_islessgreater */
        int cmp_result2 = __builtin_islessgreater(a, b);
        /* Use both to increase coverage chances */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

/* Additional test with float type for variety */
void __attribute__((noinline)) test_float_comparisons() {
    float fa[ARRAY_SIZE];
    float fb[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)(i + 1) * 1.5f;
        if (i % NAN_INTERVAL == 0) {
            fb[i] = __builtin_nanf("");
        } else {
            fb[i] = (float)(i) * 2.0f;
        }
    }
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float a = fa[i];
        float b = fb[i];
        
        /* Test various float comparisons */
        int cmp1 = __builtin_isunordered(a, b);
        int cmp2 = (a != b) && !__builtin_isunordered(a, b);
        int cmp3 = (a >= b) || __builtin_isunordered(a, b);
        int cmp4 = (a <= b) || __builtin_isunordered(a, b);
        
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
    }
}

int main() {
    /* Initialize arrays with mixed normal values and NaNs */
    init_arrays();
    
    /* Call all test functions to generate various condition codes */
    test_unordered();      /* Should generate "unord" */
    test_ordered();        /* Should generate "ord" */
    test_uneq();           /* Should generate "ueq" */
    test_unge();           /* Should generate "nlt" */
    test_ungt();           /* Should generate "nle" */
    test_unle();           /* Should generate "ule" */
    test_unlt();           /* Should generate "ult" */
    test_ltgt();           /* Should generate "une" */
    
    /* Additional float tests for more coverage */
    test_float_comparisons();
    
    /* Compute and print checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Dummy: %d\n", dummy);
    
    return 0;
}
