#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

// Global variables to prevent optimization
volatile int dummy = 0;
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with mixed normal values and NaNs
void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Every 4th element is NaN, others are normal values
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 2.5;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline))
void test_unordered(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test ORDERED condition code
__attribute__((noinline))
void test_ordered(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate ORDERED condition code (opposite of unordered)
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline))
void test_uneq(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNEQ condition code
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline))
void test_unge(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNGE condition code (unordered or a >= b)
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline))
void test_ungt(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNGT condition code (unordered or a > b)
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline))
void test_unle(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNLE condition code (unordered or a <= b)
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline))
void test_unlt(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate UNLT condition code (unordered or a < b)
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline))
void test_ltgt(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate LTGT condition code using builtin
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Alternative test using direct comparisons for LTGT
__attribute__((noinline))
void test_ltgt_direct(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        int idx = i % ARRAY_SIZE;
        double a = array1[idx];
        double b = array2[idx];
        
        // Generate LTGT condition code using direct comparisons
        int cmp_result = (a < b) || (a > b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

int main(void) {
    // Initialize arrays with mixed normal values and NaNs
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
    test_ltgt_direct();    // Alternative to generate "une"
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", dummy);
    
    return 0;
}
