/* test_condition_codes.c
 * 
 * This program is designed to trigger the condition code printing logic
 * in GCC's i386.cc backend for unordered floating-point comparisons.
 * It generates inline assembly that forces the compiler to output
 * the specific condition code names: unord, ord, ueq, nlt, nle, ule, ult, une.
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global dummy variable to prevent optimization */
volatile uint64_t dummy = 0;

/* Arrays for comparison */
#define SIZE 1024
static double array1[SIZE];
static double array2[SIZE];

/* Initialize arrays with normal values and NaNs */
__attribute__((constructor)) static void init_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1);  /* 1.0, 2.0, 3.0, ... */
        /* Every 4th element in array2 is NaN, others are normal */
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");  /* Quiet NaN */
        } else {
            array2[i] = (double)(i * 2);    /* 0.0, 4.0, 6.0, ... */
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline)) static void test_unordered(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use __builtin_isunordered to generate UNORDERED condition */
        int cmp = __builtin_isunordered(a, b);
        /* Inline asm that uses the condition code result */
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) static void test_ordered(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Ordered is the opposite of unordered */
        int cmp = !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) static void test_uneq(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use direct comparison to generate UNEQ */
        int cmp = (a != a) || (b != b) || (a == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNGE condition code (unordered or greater than or equal) */
__attribute__((noinline)) static void test_unge(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* This should generate "nlt" (not less than) */
        int cmp = !(a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline)) static void test_ungt(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* This should generate "nle" (not less than or equal) */
        int cmp = !(a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline)) static void test_unle(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* This should generate "ule" */
        int cmp = (a != a) || (b != b) || (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline)) static void test_unlt(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* This should generate "ult" */
        int cmp = (a != a) || (b != b) || (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline)) static void test_ltgt(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use __builtin_islessgreater to generate LTGT ("une") */
        int cmp = __builtin_islessgreater(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Main function that calls all test functions */
int main(void) {
    /* Call all test functions to generate different condition codes */
    test_unordered();    /* Should generate "unord" */
    test_ordered();      /* Should generate "ord" */
    test_uneq();         /* Should generate "ueq" */
    test_unge();         /* Should generate "nlt" */
    test_ungt();         /* Should generate "nle" */
    test_unle();         /* Should generate "ule" */
    test_unlt();         /* Should generate "ult" */
    test_ltgt();         /* Should generate "une" */
    
    /* Compute and print checksum to prevent dead code elimination */
    uint64_t checksum = dummy;
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
