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
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        /* Every NAN_INTERVAL-th element in array2 is NaN */
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(SIZE - i) * 0.7;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Use __builtin_isunordered to generate UNORDERED condition code */
        int cmp_result = __builtin_isunordered(a, b);
        /* Inline asm that uses the condition code result */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* ORDERED is the opposite of UNORDERED */
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNEQ: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater than or equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNGE: unordered OR greater than or equal */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNGT: unordered OR greater than */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNLE: unordered OR less than or equal */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNLT: unordered OR less than */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* LTGT: less than OR greater than (but not equal, not unordered) */
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Additional test with direct comparisons to ensure coverage */
__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Direct comparisons that should generate various condition codes */
        int cmp1 = (a < b);   /* Should generate LT */
        int cmp2 = (a > b);   /* Should generate GT */
        int cmp3 = (a == b);  /* Should generate EQ */
        int cmp4 = (a != b);  /* Should generate NE */
        
        /* Use all comparisons in asm to ensure they're not optimized away */
        asm volatile("" : "+r"(dummy) : "g"(cmp1), "g"(cmp2), "g"(cmp3), "g"(cmp4));
    }
}

int main() {
    /* Call all test functions in sequence */
    test_unordered();      /* Should trigger UNORDERED case */
    test_ordered();        /* Should trigger ORDERED case */
    test_uneq();           /* Should trigger UNEQ case */
    test_unge();           /* Should trigger UNGE case (prints "nlt") */
    test_ungt();           /* Should trigger UNGT case (prints "nle") */
    test_unle();           /* Should trigger UNLE case (prints "ule") */
    test_unlt();           /* Should trigger UNLT case (prints "ult") */
    test_ltgt();           /* Should trigger LTGT case (prints "une") */
    test_direct_comparisons();
    
    /* Compute and print checksum to prevent dead code elimination */
    int checksum = dummy;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
