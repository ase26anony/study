/* test_cc_coverage.c
 * 
 * This program is designed to trigger the condition code printing logic
 * in GCC's i386.cc for unordered floating-point comparisons.
 * Specifically targets lines 13992-14017 in i386.cc.
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global dummy variable to prevent optimization */
volatile int global_dummy = 0;

/* Arrays for comparison data */
#define ARRAY_SIZE 1024
static double array1[ARRAY_SIZE];
static double array2[ARRAY_SIZE];

/* Initialize arrays with mixed normal values and NaNs */
__attribute__((constructor)) 
static void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        
        /* Pattern: every 4th element is NaN, others are normal values */
        if ((i % 4) == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i % 10) * 0.7;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline))
void test_unordered(void) {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Use __builtin_isunordered to generate UNORDERED condition */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Inline asm that uses the condition code result */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test ORDERED condition code */
__attribute__((noinline))
void test_ordered(void) {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* ORDERED is the opposite of UNORDERED */
        int cmp_result = !__builtin_isunordered(a, b);
        
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline))
void test_uneq(void) {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNEQ: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNGE condition code (unordered or greater than or equal) */
__attribute__((noinline))
void test_unge(void) {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGE: unordered OR a >= b */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline))
void test_ungt(void) {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGT: unordered OR a > b */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline))
void test_unle(void) {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLE: unordered OR a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline))
void test_unlt(void) {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLT: unordered OR a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline))
void test_ltgt(void) {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* LTGT: not unordered AND not equal (i.e., strictly less or greater) */
        int cmp_result = !__builtin_isunordered(a, b) && (a != b);
        
        /* Alternative using __builtin_islessgreater */
        int cmp_result2 = __builtin_islessgreater(a, b);
        
        /* Use both to increase coverage chances */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp_result), "g"(cmp_result2)
                     : "cc");
    }
    global_dummy += dummy;
}

/* Additional test with direct floating comparisons that might generate specific condition codes */
__attribute__((noinline))
void test_direct_comparisons(void) {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile double a = array1[i];
        volatile double b = array2[i];
        
        /* Direct comparisons that might generate various condition codes */
        int cmp1 = (a < b);   /* LT */
        int cmp2 = (a > b);   /* GT */
        int cmp3 = (a <= b);  /* LE */
        int cmp4 = (a >= b);  /* GE */
        int cmp5 = (a == b);  /* EQ */
        int cmp6 = (a != b);  /* NEQ */
        
        /* Feed all comparisons to asm to trigger condition code generation */
        asm volatile("" 
                     : "+r"(dummy) 
                     : "g"(cmp1), "g"(cmp2), "g"(cmp3), 
                       "g"(cmp4), "g"(cmp5), "g"(cmp6)
                     : "cc");
    }
    global_dummy += dummy;
}

int main(void) {
    /* Call all test functions to generate condition codes */
    test_unordered();      /* Should trigger UNORDERED case */
    test_ordered();        /* Should trigger ORDERED case */
    test_uneq();           /* Should trigger UNEQ case */
    test_unge();           /* Should trigger UNGE case (prints "nlt") */
    test_ungt();           /* Should trigger UNGT case (prints "nle") */
    test_unle();           /* Should trigger UNLE case (prints "ule") */
    test_unlt();           /* Should trigger UNLT case (prints "ult") */
    test_ltgt();           /* Should trigger LTGT case (prints "une") */
    test_direct_comparisons();
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", global_dummy);
    
    return 0;
}
