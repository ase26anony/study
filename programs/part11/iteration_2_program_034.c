/* test_condition_codes.c
 * 
 * This program is designed to trigger the condition code printing logic
 * in GCC's i386.cc backend for unordered floating-point comparisons.
 * It generates inline assembly with condition code operands that should
 * cause the compiler to output the specific condition code names:
 * unord, ord, ueq, nlt, nle, ule, ult, une
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global dummy variable to prevent optimization */
volatile int dummy = 0;

/* Arrays for comparison */
#define ARRAY_SIZE 1024
static double array1[ARRAY_SIZE];
static double array2[ARRAY_SIZE];

/* Initialize arrays with mixed normal values and NaNs */
__attribute__((constructor)) 
static void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        /* Every 4th element in array2 is NaN, others are normal */
        if ((i % 4) == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i) * 1.2;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline))
static void test_unordered(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use __builtin_isunordered to generate UNORDERED condition */
        int cmp_result = __builtin_isunordered(a, b);
        /* Inline assembly that uses the condition code */
        asm volatile ("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline))
static void test_ordered(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* ORDERED is the opposite of UNORDERED */
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile ("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline))
static void test_uneq(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNEQ: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile ("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or not less than) */
__attribute__((noinline))
static void test_unge(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNGE: unordered OR (a >= b) */
        int cmp_result = __builtin_isunordered(a, b) || !(a < b);
        asm volatile ("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline))
static void test_ungt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNGT: unordered OR (a > b) */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile ("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline))
static void test_unle(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNLE: unordered OR (a <= b) */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile ("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline))
static void test_unlt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNLT: unordered OR (a < b) */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile ("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline))
static void test_ltgt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* LTGT: !__builtin_isunordered(a, b) && (a != b) */
        /* Use __builtin_islessgreater which maps directly to LTGT */
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile ("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Alternative test using direct comparison operators to generate condition codes */
__attribute__((noinline))
static void test_direct_comparisons(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate various condition codes through direct comparisons */
        int cmp1 = (a < b);   /* LT */
        int cmp2 = (a > b);   /* GT */
        int cmp3 = (a == b);  /* EQ */
        int cmp4 = (a != b);  /* NE */
        
        /* Feed all comparisons to asm to ensure they're used */
        asm volatile ("" : "+r"(dummy) : "g"(cmp1), "g"(cmp2), "g"(cmp3), "g"(cmp4));
    }
}

int main(void) {
    /* Call all test functions to generate condition codes */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_direct_comparisons();
    
    /* Use dummy in a visible way to prevent dead code elimination */
    printf("Result: %d\n", dummy);
    
    return 0;
}
