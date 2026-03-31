#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

// Global variables to prevent optimization
volatile int dummy = 0;
volatile double checksum = 0.0;

// Arrays with mixed normal values and NaNs
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with pattern including NaNs
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Every 4th element in array2 is NaN, others are normal
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i) * 2.0;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Use result to prevent optimization
        dummy += cmp_result;
    }
}

// Test ORDERED condition code  
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate ORDERED condition code (not unordered)
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNEQ: unordered OR equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNGE condition code (unordered or greater or equal)
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNGE: unordered OR a >= b
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNGT: unordered OR a > b
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNLE condition code (unordered or less or equal)
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNLE: unordered OR a <= b
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNLT: unordered OR a < b
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Test LTGT condition code (less or greater, but not equal and not unordered)
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate LTGT: not equal AND not unordered
        // Using __builtin_islessgreater for direct mapping
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        dummy += cmp_result;
    }
}

// Additional test with direct floating comparisons to ensure
// the compiler generates the specific condition codes
__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Direct comparisons that should generate specific condition codes
        int cmp1 = (a < b);   // Should generate LT (or UNLT with NaN)
        int cmp2 = (a > b);   // Should generate GT (or UNGT with NaN)
        int cmp3 = (a <= b);  // Should generate LE (or UNLE with NaN)
        int cmp4 = (a >= b);  // Should generate GE (or UNGE with NaN)
        int cmp5 = (a == b);  // Should generate EQ
        int cmp6 = (a != b);  // Should generate NEQ (or UNEQ with NaN)
        
        // Force all condition codes to be output
        asm volatile("" : : "g"(cmp1), "g"(cmp2), "g"(cmp3), 
                              "g"(cmp4), "g"(cmp5), "g"(cmp6));
        
        dummy += cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6;
    }
}

int main() {
    // Initialize arrays (already done via constructor)
    
    // Call all test functions
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_direct_comparisons();
    
    // Compute checksum from global variable to prevent optimization
    checksum = (double)dummy;
    
    // Print checksum to ensure code isn't optimized away
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
