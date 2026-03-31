#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define SIZE 1024
#define NAN_INTERVAL 4

/* Global dummy variable to prevent optimization */
volatile int dummy = 0;

/* Arrays with mixed normal values and NaNs */
double array1[SIZE];
double array2[SIZE];

/* Initialize arrays with pattern including NaNs */
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(SIZE - i) * 0.75;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Use __builtin_isunordered to generate UNORDERED condition */
        int cmp = __builtin_isunordered(a, b);
        
        /* Force condition code output through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Ordered is the opposite of unordered */
        int cmp = !__builtin_isunordered(a, b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Unordered or equal comparison */
        int cmp = __builtin_isunordered(a, b) || (a == b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNGE condition code (unordered or greater-or-equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Unordered or greater-or-equal: !(a < b) */
        int cmp = __builtin_isunordered(a, b) || !(a < b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNGT condition code (unordered or greater) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Unordered or greater: !(a <= b) */
        int cmp = __builtin_isunordered(a, b) || !(a <= b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNLE condition code (unordered or less-or-equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Unordered or less-or-equal: !(a > b) */
        int cmp = __builtin_isunordered(a, b) || !(a > b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test UNLT condition code (unordered or less) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Unordered or less: !(a >= b) */
        int cmp = __builtin_isunordered(a, b) || !(a >= b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Test LTGT condition code (less or greater, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Use __builtin_islessgreater for LTGT condition */
        int cmp = __builtin_islessgreater(a, b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

/* Alternative implementation using direct comparisons for some conditions */
__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Direct comparisons that may generate different condition codes */
        int cmp1 = (a != b);  /* May generate NE condition */
        int cmp2 = (a < b);   /* May generate LT condition */
        int cmp3 = (a > b);   /* May generate GT condition */
        int cmp4 = (a <= b);  /* May generate LE condition */
        int cmp5 = (a >= b);  /* May generate GE condition */
        
        /* Force condition code outputs */
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
    test_direct_comparisons(); /* Additional comparisons */
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    /* Use dummy to prevent optimization */
    checksum += dummy;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
