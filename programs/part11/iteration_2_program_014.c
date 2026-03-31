#include <stdio.h>
#include <math.h>

#define SIZE 1024
#define NAN_INTERVAL 4

/* Global arrays with mixed normal values and NaNs */
double array1[SIZE];
double array2[SIZE];

/* Dummy variable to prevent optimization */
volatile int dummy = 0;

/* Initialize arrays with pattern including NaNs */
void init_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        /* Every NAN_INTERVAL-th element in array2 is NaN */
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 2.5;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline)) void test_unordered(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use __builtin_isunordered to generate UNORDERED condition code */
        int cmp_result = __builtin_isunordered(a, b);
        /* Inline asm to force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Ordered is the opposite of unordered */
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater than or equal) */
__attribute__((noinline)) void test_unge(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or a >= b */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline)) void test_ungt(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or a > b */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline)) void test_unle(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline)) void test_unlt(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Unordered or a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use __builtin_islessgreater for LTGT condition code */
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Alternative implementation using direct comparisons for some condition codes */
__attribute__((noinline)) void test_direct_comparisons(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Direct comparisons that should generate various condition codes */
        int cmp1 = (a != b);  /* May generate UNE or similar */
        int cmp2 = (a < b);   /* May generate LT or UNLT */
        int cmp3 = (a > b);   /* May generate GT or UNGT */
        int cmp4 = (a <= b);  /* May generate LE or UNLE */
        int cmp5 = (a >= b);  /* May generate GE or UNGE */
        
        /* Force all comparisons through asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
    }
}

int main(void) {
    init_arrays();
    
    /* Call all test functions to generate various condition codes */
    test_unordered();      /* Should generate UNORDERED condition code */
    test_ordered();        /* Should generate ORDERED condition code */
    test_uneq();           /* Should generate UNEQ condition code */
    test_unge();           /* Should generate UNGE condition code */
    test_ungt();           /* Should generate UNGT condition code */
    test_unle();           /* Should generate UNLE condition code */
    test_unlt();           /* Should generate UNLT condition code */
    test_ltgt();           /* Should generate LTGT condition code */
    test_direct_comparisons(); /* Additional comparisons for coverage */
    
    /* Print dummy to prevent dead code elimination */
    printf("Result: %d\n", dummy);
    
    return 0;
}
