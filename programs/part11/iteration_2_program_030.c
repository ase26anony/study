/* test_condition_codes.c
 * 
 * This program is designed to trigger the condition code printing logic
 * in GCC's i386.cc backend for unordered floating-point comparisons.
 * It generates inline assembly that forces the compiler to output
 * condition code names like "unord", "ord", "ueq", "nlt", etc.
 */

#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024

/* Global dummy variable to prevent optimization */
volatile int dummy = 0;

/* Arrays with mixed normal values and NaNs */
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Initialize arrays with pattern: array1 has sequential values,
 * array2 has NaN at regular intervals */
__attribute__((constructor))
static void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1);
        /* Every 4th element in array2 is NaN, others are normal */
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i * 2);
        }
    }
}

/* Test UNORDERED condition code (unord) */
__attribute__((noinline))
void test_unordered(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use __builtin_isunordered to generate UNORDERED condition */
        int cmp_result = __builtin_isunordered(a, b);
        /* Inline asm that uses the comparison result */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test ORDERED condition code (ord) */
__attribute__((noinline))
void test_ordered(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Ordered is the opposite of unordered */
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (ueq) - unordered or equal */
__attribute__((noinline))
void test_uneq(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (nlt) - unordered or greater-or-equal */
__attribute__((noinline))
void test_unge(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or a >= b (not less than) */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (nle) - unordered or greater */
__attribute__((noinline))
void test_ungt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or a > b (not less-or-equal) */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (ule) - unordered or less-or-equal */
__attribute__((noinline))
void test_unle(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (ult) - unordered or less than */
__attribute__((noinline))
void test_unlt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (une) - less or greater (ordered and not equal) */
__attribute__((noinline))
void test_ltgt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use __builtin_islessgreater for LTGT condition */
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

int main(void) {
    /* Call all test functions to generate various condition codes */
    test_unordered();    /* Should generate "unord" */
    test_ordered();      /* Should generate "ord" */
    test_uneq();         /* Should generate "ueq" */
    test_unge();         /* Should generate "nlt" */
    test_ungt();         /* Should generate "nle" */
    test_unle();         /* Should generate "ule" */
    test_unlt();         /* Should generate "ult" */
    test_ltgt();         /* Should generate "une" */
    
    /* Use dummy to compute checksum and prevent dead code elimination */
    int checksum = dummy;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
