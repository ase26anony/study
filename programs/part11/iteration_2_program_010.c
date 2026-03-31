#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024

// Global arrays with mixed normal values and NaNs
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Dummy variable to prevent optimization
volatile int dummy = 0;

// Initialize arrays with pattern including NaNs
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Every 4th element in array2 is NaN, others are normal values
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i) * 2.0;
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
        
        // Force condition code output through inline asm
        asm volatile("" : : "g"(cmp_result) : "memory");
        
        // Also use direct comparison with unordered semantics
        if (a != a || b != b) {
            asm volatile("" : : "g"(1) : "memory");
        }
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
        asm volatile("" : : "g"(cmp_result) : "memory");
        
        // Alternative: ordered comparison
        if (a == a && b == b) {
            asm volatile("" : : "g"(1) : "memory");
        }
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNEQ: unordered OR equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
        
        // Alternative formulation
        if (!(a < b || b < a)) {
            asm volatile("" : : "g"(1) : "memory");
        }
    }
}

// Test UNGE condition code (unordered or greater-or-equal)
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGE: unordered OR a >= b
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
        
        // Alternative: not less than
        if (!(a < b)) {
            asm volatile("" : : "g"(1) : "memory");
        }
    }
}

// Test UNGT condition code (unordered or greater)
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGT: unordered OR a > b
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
        
        // Alternative: not less-or-equal
        if (!(a <= b)) {
            asm volatile("" : : "g"(1) : "memory");
        }
    }
}

// Test UNLE condition code (unordered or less-or-equal)
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLE: unordered OR a <= b
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
        
        // Alternative formulation
        if (a <= b) {
            asm volatile("" : : "g"(1) : "memory");
        }
    }
}

// Test UNLT condition code (unordered or less)
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLT: unordered OR a < b
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
        
        // Direct comparison
        if (a < b) {
            asm volatile("" : : "g"(1) : "memory");
        }
    }
}

// Test LTGT condition code (less or greater, but not equal and not unordered)
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate LTGT: less OR greater (but not equal, not unordered)
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result) : "memory");
        
        // Alternative: not equal and ordered
        if ((a < b || b < a) && !__builtin_isunordered(a, b)) {
            asm volatile("" : : "g"(1) : "memory");
        }
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
    
    // Use dummy variable to prevent dead code elimination
    printf("Dummy checksum: %d\n", dummy);
    
    return 0;
}
