#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

// Global variables to prevent optimization
volatile int dummy = 0;
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with mixed normal values and NaNs
void __attribute__((noinline)) init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 0.7;
        }
    }
}

// Test UNORDERED condition code
void __attribute__((noinline)) test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result) : "memory");
    }
}

// Test ORDERED condition code
void __attribute__((noinline)) test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate ORDERED condition code (not unordered)
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
    }
}

// Test UNEQ condition code (unordered or equal)
void __attribute__((noinline)) test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNEQ: unordered OR equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
    }
}

// Test UNGE condition code (unordered or greater-or-equal)
void __attribute__((noinline)) test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGE: unordered OR (a >= b)
        // Using !(a < b) to get >= semantics
        int cmp_result = __builtin_isunordered(a, b) || !(a < b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
    }
}

// Test UNGT condition code (unordered or greater)
void __attribute__((noinline)) test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGT: unordered OR (a > b)
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
    }
}

// Test UNLE condition code (unordered or less-or-equal)
void __attribute__((noinline)) test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLE: unordered OR (a <= b)
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
    }
}

// Test UNLT condition code (unordered or less)
void __attribute__((noinline)) test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLT: unordered OR (a < b)
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
    }
}

// Test LTGT condition code (less or greater, but not equal and not unordered)
void __attribute__((noinline)) test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate LTGT: less or greater (using builtin)
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
    }
}

// Alternative test using direct comparisons for different condition codes
void __attribute__((noinline)) test_direct_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Test various comparison operators that might generate different condition codes
        int cmp1 = (a != b);  // May generate NE condition
        int cmp2 = (a < b);   // May generate LT condition
        int cmp3 = (a > b);   // May generate GT condition
        int cmp4 = (a <= b);  // May generate LE condition
        int cmp5 = (a >= b);  // May generate GE condition
        
        // Force all condition codes
        asm volatile("" : : "g"(cmp1), "g"(cmp2), "g"(cmp3), "g"(cmp4), "g"(cmp5) : "memory");
    }
}

int main() {
    // Initialize arrays with mixed values and NaNs
    init_arrays();
    
    // Call all test functions to generate various condition codes
    test_unordered();      // Should generate "unord"
    test_ordered();        // Should generate "ord"
    test_uneq();           // Should generate "ueq"
    test_unge();           // Should generate "nlt"
    test_ungt();           // Should generate "nle"
    test_unle();           // Should generate "ule"
    test_unlt();           // Should generate "ult"
    test_ltgt();           // Should generate "une"
    
    // Additional direct comparisons
    test_direct_comparisons();
    
    // Use dummy variable to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    // Print checksum to ensure code isn't optimized away
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
