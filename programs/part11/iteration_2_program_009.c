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
            array2[i] = (double)(SIZE - i) * 0.75;
        }
    }
}

/* Test UNORDERED condition code (unord) */
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Using __builtin_isunordered to generate UNORDERED condition */
        int cmp = __builtin_isunordered(a, b);
        /* Inline asm to force condition code output */
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp; /* Prevent dead code elimination */
    }
}

/* Test ORDERED condition code (ord) */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* Ordered is the opposite of unordered */
        int cmp = !__builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

/* Test UNEQ condition code (ueq) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNEQ: unordered or equal */
        int cmp = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

/* Test UNGE condition code (nlt) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNGE: unordered or greater than or equal */
        int cmp = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

/* Test UNGT condition code (nle) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNGT: unordered or greater than */
        int cmp = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

/* Test UNLE condition code (ule) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNLE: unordered or less than or equal */
        int cmp = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

/* Test UNLT condition code (ult) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* UNLT: unordered or less than */
        int cmp = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

/* Test LTGT condition code (une) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        /* LTGT: less than or greater than (but not equal and not unordered) */
        int cmp = __builtin_islessgreater(a, b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

/* Alternative implementation using direct comparisons for some codes */
__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Direct comparisons that may generate different condition codes */
        int cmp1 = (a != a); /* Check for NaN (unordered) */
        int cmp2 = (a == b); /* Equality check */
        int cmp3 = (a < b);  /* Less than */
        int cmp4 = (a > b);  /* Greater than */
        int cmp5 = (a <= b); /* Less than or equal */
        int cmp6 = (a >= b); /* Greater than or equal */
        
        /* Force all comparisons through asm */
        asm volatile("" : : "g"(cmp1), "g"(cmp2), "g"(cmp3), 
                     "g"(cmp4), "g"(cmp5), "g"(cmp6) : "memory");
        
        dummy += cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6;
    }
}

int main() {
    /* Call all test functions to generate various condition codes */
    test_unordered();     /* Should generate "unord" */
    test_ordered();       /* Should generate "ord" */
    test_uneq();          /* Should generate "ueq" */
    test_unge();          /* Should generate "nlt" */
    test_ungt();          /* Should generate "nle" */
    test_unle();          /* Should generate "ule" */
    test_unlt();          /* Should generate "ult" */
    test_ltgt();          /* Should generate "une" */
    
    test_direct_comparisons(); /* Additional comparisons for coverage */
    
    /* Print dummy to prevent optimization */
    printf("Result: %d\n", dummy);
    
    return 0;
}
