#include <stdio.h>
#include <math.h>

#define SIZE 1024

// Global arrays with mixed normal values and NaNs
double arr1[SIZE];
double arr2[SIZE];

// Dummy variable to prevent optimization
volatile int dummy = 0;

// Initialize arrays with pattern that includes NaNs
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (double)i * 1.5;
        // Every 4th element in arr2 is NaN, others are normal
        if (i % 4 == 0) {
            arr2[i] = __builtin_nan("");
        } else {
            arr2[i] = (double)i * 0.75;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline asm
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent loop optimization
        dummy += cmp_result;
    }
}

// Test ORDERED condition code
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Generate ORDERED condition code (not unordered)
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Generate UNEQ: unordered OR equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNGE condition code (unordered or greater-or-equal)
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Generate UNGE: unordered OR a >= b
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNGT condition code (unordered or greater)
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Generate UNGT: unordered OR a > b
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNLE condition code (unordered or less-or-equal)
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Generate UNLE: unordered OR a <= b
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNLT condition code (unordered or less)
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Generate UNLT: unordered OR a < b
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test LTGT condition code (less or greater, but not equal and not unordered)
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Generate LTGT: less OR greater (using builtin for unordered-aware comparison)
        int cmp_result = __builtin_islessgreater(a, b);
        
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Additional test with direct floating comparisons to generate various condition codes
__attribute__((noinline)) void test_mixed_comparisons() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Test various comparison operators that might generate different condition codes
        int cmp1 = (a < b);   // LT
        int cmp2 = (a > b);   // GT
        int cmp3 = (a <= b);  // LE
        int cmp4 = (a >= b);  // GE
        int cmp5 = (a == b);  // EQ
        int cmp6 = (a != b);  // NE
        
        // Force all condition codes through inline asm
        asm volatile("" : : "g"(cmp1), "g"(cmp2), "g"(cmp3), 
                              "g"(cmp4), "g"(cmp5), "g"(cmp6));
        
        dummy += cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6;
    }
}

int main() {
    // Call all test functions to generate the condition codes
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
    printf("Checksum: %d\n", dummy);
    
    return 0;
}
