#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

/* Global arrays with mixed normal values and NaNs */
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

/* Dummy variable to prevent optimization */
volatile int dummy = 0;

/* Initialize arrays with pattern including NaNs */
void __attribute__((noinline)) init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 0.75;
        }
    }
}

/* Test UNORDERED condition code */
void __attribute__((noinline)) test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using __builtin_isunordered to generate UNORDERED condition */
        int cmp_result = __builtin_isunordered(a, b);
        
        /* Force condition code output through inline assembly */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: direct comparison that might be unordered */
        int cmp_result2 = !(a == b) && !(a < b) && !(a > b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

/* Test ORDERED condition code */
void __attribute__((noinline)) test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Using negation of unordered to generate ORDERED condition */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: ordered comparison */
        int cmp_result2 = (a == b) || (a < b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

/* Test UNEQ condition code (unordered or equal) */
void __attribute__((noinline)) test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNEQ: unordered or equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Force condition code output */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative implementation */
        int cmp_result2 = !(a != b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

/* Test UNGE condition code (unordered or greater-or-equal) */
void __attribute__((noinline)) test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGE: unordered or a >= b */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Force condition code output - should generate "nlt" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: not less than */
        int cmp_result2 = !(a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

/* Test UNGT condition code (unordered or greater) */
void __attribute__((noinline)) test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGT: unordered or a > b */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Force condition code output - should generate "nle" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: not less-or-equal */
        int cmp_result2 = !(a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

/* Test UNLE condition code (unordered or less-or-equal) */
void __attribute__((noinline)) test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLE: unordered or a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code output - should generate "ule" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Direct comparison */
        int cmp_result2 = (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

/* Test UNLT condition code (unordered or less) */
void __attribute__((noinline)) test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLT: unordered or a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Force condition code output - should generate "ult" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Direct comparison */
        int cmp_result2 = (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

/* Test LTGT condition code (less or greater, but not equal and not unordered) */
void __attribute__((noinline)) test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate LTGT: less or greater (not equal, not unordered) */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Force condition code output - should generate "une" */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: not equal and ordered */
        int cmp_result2 = (a != b) && !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

int main() {
    /* Initialize arrays with mixed normal values and NaNs */
    init_arrays();
    
    /* Call all test functions to generate various condition codes */
    test_unordered();      /* Should trigger "unord" output */
    test_ordered();        /* Should trigger "ord" output */
    test_uneq();           /* Should trigger "ueq" output */
    test_unge();           /* Should trigger "nlt" output */
    test_ungt();           /* Should trigger "nle" output */
    test_unle();           /* Should trigger "ule" output */
    test_unlt();           /* Should trigger "ult" output */
    test_ltgt();           /* Should trigger "une" output */
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (int)array1[i] + (int)array2[i];
    }
    
    /* Use dummy to prevent optimization */
    checksum += dummy;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
