#include <stdio.h>
#include <stdint.h>
#include <math.h>

// Global variable to prevent optimization
volatile int dummy = 0;

// Arrays for testing
#define SIZE 1024
double array1[SIZE];
double array2[SIZE];

// Initialize arrays with mixed normal values and NaNs
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Every 4th element in array2 is NaN, others are normal
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 0.75;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Also use direct comparison with unordered semantics
        cmp_result = (a != a) || (b != b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test ORDERED condition code
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate ORDERED condition code
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative ordered check
        cmp_result = (a == a) && (b == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNEQ condition code
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Direct unordered-or-equal comparison
        cmp_result = !(a < b || a > b);  // Not less and not greater
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNGE condition code (unordered or greater-or-equal)
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGE condition code (nlt in assembly)
        int cmp_result = __builtin_isunordered(a, b) || !(a < b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative: not less-than (including unordered)
        cmp_result = !(a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNGT condition code (unordered or greater)
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGT condition code (nle in assembly)
        int cmp_result = __builtin_isunordered(a, b) || !(a <= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative: not less-or-equal (including unordered)
        cmp_result = !(a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNLE condition code (unordered or less-or-equal)
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLE condition code
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Direct unordered-or-less-or-equal
        cmp_result = !(a > b);  // Not greater (includes unordered)
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNLT condition code (unordered or less)
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLT condition code
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Direct unordered-or-less-than
        cmp_result = !(a >= b);  // Not greater-or-equal (includes unordered)
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test LTGT condition code (less or greater, but not equal and not unordered)
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate LTGT condition code (une in assembly)
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative: less or greater (excluding equal and unordered)
        cmp_result = (a < b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Additional test with float type to ensure different code generation
__attribute__((noinline)) void test_float_comparisons() {
    float farray1[SIZE];
    float farray2[SIZE];
    
    // Initialize float arrays
    for (int i = 0; i < SIZE; i++) {
        farray1[i] = (float)(i + 1) * 1.25f;
        farray2[i] = (i % 3 == 0) ? __builtin_nanf("") : (float)(i + 1) * 0.5f;
    }
    
    // Test various float comparisons
    for (int i = 0; i < SIZE; i++) {
        float a = farray1[i];
        float b = farray2[i];
        
        // Test all condition codes with floats
        int cmp1 = __builtin_isunordered(a, b);
        int cmp2 = !__builtin_isunordered(a, b);
        int cmp3 = __builtin_islessgreater(a, b);
        int cmp4 = (a != b);
        int cmp5 = (a == b);
        
        // Force condition code outputs
        asm volatile("" : : "g"(cmp1));
        asm volatile("" : : "g"(cmp2));
        asm volatile("" : : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
    }
}

int main() {
    // Call all test functions
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_float_comparisons();
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    // Use dummy to prevent optimization
    checksum += dummy;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
