#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define SIZE 1024
#define NAN_INTERVAL 4

/* Global arrays with mixed normal values and NaNs */
double array1[SIZE];
double array2[SIZE];

/* Dummy variable to prevent optimization */
volatile int dummy = 0;

/* Initialize arrays with pattern: array1 has sequential values,
   array2 has NaNs at regular intervals */
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1);
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(SIZE - i);
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
        
        /* Force condition code output via inline assembly */
        asm volatile("" : : "g"(cmp_result));
        
        /* Also use direct comparison with NaN to generate UNORDERED */
        int cmp_result2 = (a != a) || (b != b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

/* Test ORDERED condition code */
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Use !__builtin_isunordered to generate ORDERED condition code */
        int cmp_result = !__builtin_isunordered(a, b);
        
        /* Force condition code output via inline assembly */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: direct ordered check */
        int cmp_result2 = (a == a) && (b == b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

/* Test UNEQ condition code (unordered or equal) */
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNEQ: unordered or equal */
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        /* Force condition code output via inline assembly */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNGE condition code (unordered or greater than or equal) */
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGE: unordered or a >= b */
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        /* Force condition code output via inline assembly */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: not less than (nlt) */
        int cmp_result2 = !(a < b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

/* Test UNGT condition code (unordered or greater than) */
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNGT: unordered or a > b */
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        /* Force condition code output via inline assembly */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: not less than or equal (nle) */
        int cmp_result2 = !(a <= b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

/* Test UNLE condition code (unordered or less than or equal) */
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLE: unordered or a <= b */
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        /* Force condition code output via inline assembly */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test UNLT condition code (unordered or less than) */
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Generate UNLT: unordered or a < b */
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        /* Force condition code output via inline assembly */
        asm volatile("" : : "g"(cmp_result));
    }
}

/* Test LTGT condition code (less than or greater than, but not equal and not unordered) */
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        /* Use __builtin_islessgreater to generate LTGT condition code */
        int cmp_result = __builtin_islessgreater(a, b);
        
        /* Force condition code output via inline assembly */
        asm volatile("" : : "g"(cmp_result));
        
        /* Alternative: (a < b) || (a > b) but both ordered */
        int cmp_result2 = (a < b) || (a > b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

/* Main function that calls all test functions */
int main() {
    int checksum = 0;
    
    /* Call all test functions to generate various condition codes */
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    
    /* Use dummy variable in a way that prevents optimization */
    checksum = dummy;
    
    /* Print checksum to ensure code isn't optimized away */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
