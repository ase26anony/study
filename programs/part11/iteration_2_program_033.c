#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024

/* Global arrays with mixed normal values and NaNs */
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Dummy variable to prevent optimization */
volatile int dummy = 0;

/* Initialize arrays with pattern: array1 has sequential values,
   array2 has NaN every 4th element */
void __attribute__((noinline)) init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1);
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i * 2);
        }
    }
}

/* Test UNORDERED condition code */
void __attribute__((noinline)) test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Use __builtin_isunordered to generate UNORDERED condition code */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Force condition code output through inline assembly */
        asm volatile("" : : "g"(cmp_result));
        
        /* Also use direct comparison with NaN to generate UNORDERED */
        int cmp_result2 = (a != a) || (b != b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

/* Test ORDERED condition code */
void __attribute__((noinline)) test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* ORDERED is the opposite of UNORDERED */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        /* Alternative using ordered comparison */
        int cmp_result2 = (a == a) && (b == b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

/* Test UNEQ condition code (unordered or equal) */
void __attribute__((noinline)) test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNEQ: unordered OR equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Force condition code output */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater than or equal) */
void __attribute__((noinline)) test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGE: unordered OR a >= b */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Force condition code output - this should generate "nlt" */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNGT condition code (unordered or greater than) */
void __attribute__((noinline)) test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNGT: unordered OR a > b */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Force condition code output - this should generate "nle" */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
void __attribute__((noinline)) test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLE: unordered OR a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code output - this should generate "ule" */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less than) */
void __attribute__((noinline)) test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* UNLT: unordered OR a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Force condition code output - this should generate "ult" */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
void __attribute__((noinline)) test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* LTGT: less than OR greater than (but not equal, not unordered) */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Force condition code output - this should generate "une" */
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        /* Alternative implementation */
        int cmp_result2 = (a < b) || (a > b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

int main() {
    /* Initialize arrays with mixed normal values and NaNs */
    init_arrays();
    
    /* Call all test functions to generate various condition codes */
    test_unordered();    /* Should generate "unord" */
    test_ordered();      /* Should generate "ord" */
    test_uneq();         /* Should generate "ueq" */
    test_unge();         /* Should generate "nlt" */
    test_ungt();         /* Should generate "nle" */
    test_unle();         /* Should generate "ule" */
    test_unlt();         /* Should generate "ult" */
    test_ltgt();         /* Should generate "une" */
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = dummy;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (int)array1[i] + (int)array2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
