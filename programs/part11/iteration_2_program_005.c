#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

/* Global arrays with mixed normal values and NaNs */
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Dummy variable to prevent optimization */
volatile int dummy = 0;

/* Initialize arrays with pattern: array1 has sequential values,
   array2 has NaNs at regular intervals */
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

/* Test UNORDERED condition code (unord) */
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using __builtin_isunordered to generate UNORDERED condition */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Inline asm to force condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test ORDERED condition code (ord) */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Ordered is the opposite of unordered */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Inline asm to force condition code output */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (ueq) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNEQ: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Inline asm to force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (nlt) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGE: unordered OR greater than or equal */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Inline asm to force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (nle) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGT: unordered OR greater than */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Inline asm to force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (ule) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLE: unordered OR less than or equal */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Inline asm to force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (ult) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLT: unordered OR less than */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Inline asm to force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (une) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* LTGT: less than OR greater than (using builtin) */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Inline asm to force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Alternative implementation using direct comparisons for some conditions */
__attribute__((noinline)) void test_mixed_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Direct comparisons that may generate different condition codes */
        int cmp1 = (a != a) || (b != b);  /* Check for NaN */
        int cmp2 = (a < b);
        int cmp3 = (a > b);
        int cmp4 = (a <= b);
        int cmp5 = (a >= b);
        int cmp6 = (a == b);
        
        /* Multiple asm statements with different comparisons */
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
        asm volatile("" : "+r"(dummy) : "g"(cmp6));
    }
}

int main() {
    /* Call all test functions to generate various condition codes */
    test_unordered();      /* Should generate UNORDERED (unord) */
    test_ordered();        /* Should generate ORDERED (ord) */
    test_uneq();           /* Should generate UNEQ (ueq) */
    test_unge();           /* Should generate UNGE (nlt) */
    test_ungt();           /* Should generate UNGT (nle) */
    test_unle();           /* Should generate UNLE (ule) */
    test_unlt();           /* Should generate UNLT (ult) */
    test_ltgt();           /* Should generate LTGT (une) */
    test_mixed_comparisons(); /* Additional comparisons for coverage */
    
    /* Use dummy variable to prevent dead code elimination */
    printf("Checksum: %d\n", dummy);
    
    return 0;
}
