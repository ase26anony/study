#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

// Global variables to prevent optimization
volatile int dummy = 0;
volatile int checksum = 0;

// Arrays with mixed normal values and NaNs
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with pattern including NaNs
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

// Test UNORDERED condition code
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        checksum += dummy;
    }
}

// Test ORDERED condition code  
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate ORDERED condition code (opposite of unordered)
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        checksum += dummy;
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNEQ condition code
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        checksum += dummy;
    }
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGE condition code
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        checksum += dummy;
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGT condition code
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        checksum += dummy;
    }
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLE condition code
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        checksum += dummy;
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLT condition code
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        checksum += dummy;
    }
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate LTGT condition code using builtin
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        checksum += dummy;
    }
}

// Alternative implementation using direct comparisons for some codes
__attribute__((noinline)) void test_mixed_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Test various comparison operators that might generate different condition codes
        int cmp1 = (a != b);  // May generate UNE
        int cmp2 = (a < b);   // May generate LT
        int cmp3 = (a > b);   // May generate GT
        int cmp4 = (a <= b);  // May generate LE
        int cmp5 = (a >= b);  // May generate GE
        
        // Force condition code outputs
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        checksum += dummy;
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        checksum += dummy;
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        checksum += dummy;
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        checksum += dummy;
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
        checksum += dummy;
    }
}

int main() {
    // Call all test functions to generate various condition codes
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed_comparisons();
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
