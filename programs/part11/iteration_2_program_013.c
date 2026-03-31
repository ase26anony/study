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
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        /* Every NAN_INTERVAL-th element in array2 is NaN */
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 0.75;
        }
    }
}

/* Test UNORDERED condition code */
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Use __builtin_isunordered to generate UNORDERED condition */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* ORDERED is the opposite of UNORDERED */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNEQ: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater than or equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGE: unordered OR a >= b */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGT: unordered OR a > b */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLE: unordered OR a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLT: unordered OR a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* LTGT: a < b OR a > b (but not equal and not unordered) */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Inline asm that forces condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Alternative implementation using direct comparisons for some condition codes */
__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Direct comparisons that may generate different condition codes */
        int cmp1 = (a != b);  /* May generate NE condition */
        int cmp2 = (a < b);   /* May generate LT condition */
        int cmp3 = (a > b);   /* May generate GT condition */
        int cmp4 = (a <= b);  /* May generate LE condition */
        int cmp5 = (a >= b);  /* May generate GE condition */
        
        /* Force all comparisons through inline asm */
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
    }
}

int main() {
    /* Call all test functions to generate various condition codes */
    test_unordered();      /* Should generate UNORDERED condition code */
    test_ordered();        /* Should generate ORDERED condition code */
    test_uneq();           /* Should generate UNEQ condition code */
    test_unge();           /* Should generate UNGE condition code */
    test_ungt();           /* Should generate UNGT condition code */
    test_unle();           /* Should generate UNLE condition code */
    test_unlt();           /* Should generate UNLT condition code */
    test_ltgt();           /* Should generate LTGT condition code */
    test_direct_comparisons(); /* Additional comparisons for completeness */
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    /* Use dummy to prevent optimization */
    checksum += dummy;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
