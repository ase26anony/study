#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1024

// Global variable to prevent optimization
volatile int dummy = 0;

// Arrays with mixed normal values and NaNs
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with pattern including NaNs
__attribute__((noinline))
void init_arrays() {
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
void test_unordered() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
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
void test_ordered() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate ORDERED condition code (not unordered)
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline))
void test_uneq() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNEQ condition code: unordered OR equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline))
void test_unge() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNGE condition code: unordered OR a >= b
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline))
void test_ungt() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNGT condition code: unordered OR a > b
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline))
void test_unle() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNLE condition code: unordered OR a <= b
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline))
void test_unlt() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNLT condition code: unordered OR a < b
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline))
void test_ltgt() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate LTGT condition code using builtin
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Alternative test for LTGT using direct comparisons
__attribute__((noinline))
void test_ltgt_alt() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate LTGT condition code: (a < b) || (a > b)
        // This excludes equal and unordered cases
        int cmp_result = (a < b) || (a > b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Prevent optimization
        dummy += cmp_result;
    }
}

// Test all condition codes with float type as well
__attribute__((noinline))
void test_float_comparisons() {
    float farray1[ARRAY_SIZE];
    float farray2[ARRAY_SIZE];
    
    // Initialize float arrays
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farray1[i] = (float)(i + 1) * 1.5f;
        if (i % 4 == 0) {
            farray2[i] = __builtin_nanf("");
        } else {
            farray2[i] = (float)(i + 1) * 2.5f;
        }
    }
    
    // Test various float comparisons
    for (int i = 0; i < ITERATIONS; i++) {
        float a = farray1[i % ARRAY_SIZE];
        float b = farray2[i % ARRAY_SIZE];
        
        // Test different condition codes with floats
        int cmp1 = __builtin_isunordered(a, b);
        int cmp2 = !__builtin_isunordered(a, b);
        int cmp3 = __builtin_islessgreater(a, b);
        int cmp4 = (a == b) || __builtin_isunordered(a, b);
        
        // Force condition code outputs
        asm volatile("" : : "g"(cmp1));
        asm volatile("" : : "g"(cmp2));
        asm volatile("" : : "g"(cmp3));
        asm volatile("" : : "g"(cmp4));
        
        dummy += cmp1 + cmp2 + cmp3 + cmp4;
    }
}

int main() {
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
    test_ltgt_alt();       // Alternative way to generate "une"
    
    // Test with float type as well
    test_float_comparisons();
    
    // Print dummy checksum to prevent dead code elimination
    printf("Checksum: %d\n", dummy);
    
    return 0;
}
