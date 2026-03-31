#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define SIZE 1024
#define DUMMY_INIT 0x12345678

// Global arrays with mixed NaN and normal values
double array1[SIZE];
double array2[SIZE];

// Dummy variable to prevent optimization
volatile uint64_t dummy = DUMMY_INIT;

// Initialize arrays with pattern including NaNs
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Every 4th element is NaN, others are normal
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i) * 2.0;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNORDERED condition code
        int cmp = __builtin_isunordered(a, b);
        
        // Force condition code output through inline asm
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

// Test ORDERED condition code  
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate ORDERED condition code (not unordered)
        int cmp = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNEQ: unordered OR equal
        int cmp = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

// Test UNGE condition code (unordered or greater or equal)
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGE: unordered OR a >= b
        // Using !(a < b) to get nlt (not less than)
        int cmp = __builtin_isunordered(a, b) || !(a < b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGT: unordered OR a > b
        // Using !(a <= b) to get nle (not less or equal)
        int cmp = __builtin_isunordered(a, b) || !(a <= b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

// Test UNLE condition code (unordered or less or equal)
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLE: unordered OR a <= b
        int cmp = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLT: unordered OR a < b
        int cmp = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

// Test LTGT condition code (less or greater, but not unordered and not equal)
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate LTGT: a != b and not unordered (using builtin)
        int cmp = __builtin_islessgreater(a, b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

// Alternative test using direct comparisons for LTGT
__attribute__((noinline)) void test_ltgt_direct() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate LTGT: (a < b) || (a > b) but not unordered
        // This should also generate une (unordered not equal)
        int cmp = (a < b) || (a > b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp));
    }
}

int main() {
    // Reset dummy
    dummy = DUMMY_INIT;
    
    // Call all test functions to generate various condition codes
    test_unordered();      // Should generate "unord"
    test_ordered();        // Should generate "ord"
    test_uneq();           // Should generate "ueq"
    test_unge();           // Should generate "nlt"
    test_ungt();           // Should generate "nle"
    test_unle();           // Should generate "ule"
    test_unlt();           // Should generate "ult"
    test_ltgt();           // Should generate "une"
    test_ltgt_direct();    // Alternative way to generate "une"
    
    // Compute checksum from dummy to prevent dead code elimination
    uint64_t checksum = dummy ^ (dummy >> 32);
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    return 0;
}
