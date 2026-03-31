#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define SIZE 1024
#define ITERATIONS 1000

// Global variables to prevent optimization
volatile int dummy = 0;
double arr1[SIZE];
double arr2[SIZE];

// Initialize arrays with mixed values including NaNs
__attribute__((noinline, constructor))
void init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (double)(i + 1) * 1.5;
        // Every 4th element is NaN, others are normal numbers
        if (i % 4 == 0) {
            arr2[i] = __builtin_nan("");
        } else {
            arr2[i] = (double)(i + 1) * 2.5;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline))
void test_unordered() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % SIZE;
        double a = arr1[idx];
        double b = arr2[idx];
        
        // Generate UNORDERED condition code
        int cmp = __builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp));
    }
}

// Test ORDERED condition code  
__attribute__((noinline))
void test_ordered() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % SIZE;
        double a = arr1[idx];
        double b = arr2[idx];
        
        // Generate ORDERED condition code
        int cmp = !__builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp));
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline))
void test_uneq() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % SIZE;
        double a = arr1[idx];
        double b = arr2[idx];
        
        // Generate UNEQ condition code
        int cmp = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : : "g"(cmp));
    }
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline))
void test_unge() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % SIZE;
        double a = arr1[idx];
        double b = arr2[idx];
        
        // Generate UNGE condition code - maps to "nlt" in output
        int cmp = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : : "g"(cmp));
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline))
void test_ungt() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % SIZE;
        double a = arr1[idx];
        double b = arr2[idx];
        
        // Generate UNGT condition code - maps to "nle" in output
        int cmp = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : : "g"(cmp));
    }
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline))
void test_unle() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % SIZE;
        double a = arr1[idx];
        double b = arr2[idx];
        
        // Generate UNLE condition code - maps to "ule" in output
        int cmp = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : : "g"(cmp));
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline))
void test_unlt() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % SIZE;
        double a = arr1[idx];
        double b = arr2[idx];
        
        // Generate UNLT condition code - maps to "ult" in output
        int cmp = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : : "g"(cmp));
    }
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline))
void test_ltgt() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % SIZE;
        double a = arr1[idx];
        double b = arr2[idx];
        
        // Generate LTGT condition code - maps to "une" in output
        int cmp = __builtin_islessgreater(a, b);
        asm volatile("" : : "g"(cmp));
    }
}

// Alternative implementation using direct comparisons with volatile
__attribute__((noinline))
void test_direct_comparisons() {
    volatile double a = __builtin_nan("");
    volatile double b = 1.0;
    volatile double c = 2.0;
    
    // Force generation of various condition codes through direct comparisons
    asm volatile("" : : "g"(a == b));  // Should generate UNEQ or similar
    asm volatile("" : : "g"(a != b));  // Should generate UNORDERED or similar
    asm volatile("" : : "g"(a < b));   // Should generate UNLT or similar
    asm volatile("" : : "g"(a > b));   // Should generate UNGT or similar
    asm volatile("" : : "g"(a <= b));  // Should generate UNLE or similar
    asm volatile("" : : "g"(a >= b));  // Should generate UNGE or similar
    
    // Ordered comparisons
    asm volatile("" : : "g"(c == b));  // Ordered equal
    asm volatile("" : : "g"(c != b));  // Ordered not equal (LTGT)
    asm volatile("" : : "g"(c < b));   // Ordered less than
    asm volatile("" : : "g"(c > b));   // Ordered greater than
}

int main() {
    // Initialize arrays
    init_arrays();
    
    // Call all test functions to generate various condition codes
    test_unordered();      // Should trigger "unord" output
    test_ordered();        // Should trigger "ord" output
    test_uneq();           // Should trigger "ueq" output
    test_unge();           // Should trigger "nlt" output
    test_ungt();           // Should trigger "nle" output
    test_unle();           // Should trigger "ule" output
    test_unlt();           // Should trigger "ult" output
    test_ltgt();           // Should trigger "une" output
    
    // Additional direct comparisons
    test_direct_comparisons();
    
    // Use dummy variable to prevent dead code elimination
    printf("Result: %d\n", dummy);
    
    return 0;
}
