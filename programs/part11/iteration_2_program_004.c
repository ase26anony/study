#include <stdio.h>
#include <math.h>

#define SIZE 1024

// Global variables to prevent optimization
volatile int dummy = 0;
double arr1[SIZE];
double arr2[SIZE];

// Initialize arrays with mixed normal values and NaNs
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (double)(i + 1) * 1.5;
        // Every 4th element in arr2 is NaN, others are normal
        if (i % 4 == 0) {
            arr2[i] = __builtin_nan("");
        } else {
            arr2[i] = (double)(i + 1) * 0.75;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        // UNORDERED: true if either operand is NaN
        int cmp_result = __builtin_isunordered(a, b);
        // Force condition code output through inline asm
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test ORDERED condition code
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        // ORDERED: true if neither operand is NaN
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        // UNEQ: true if unordered OR equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        // UNGE: true if unordered OR a >= b
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        // UNGT: true if unordered OR a > b
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        // UNLE: true if unordered OR a <= b
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        // UNLT: true if unordered OR a < b
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        // LTGT: true if a < b OR a > b (but not equal and not unordered)
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Alternative implementation using direct comparisons for some codes
__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < SIZE; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        // Direct comparisons that may generate different condition codes
        int cmp1 = (a != b);  // May generate UNE
        int cmp2 = (a < b);   // May generate LT
        int cmp3 = (a > b);   // May generate GT
        int cmp4 = (a <= b);  // May generate LE
        int cmp5 = (a >= b);  // May generate GE
        
        // Force all condition codes through asm
        asm volatile("" : : "g"(cmp1), "g"(cmp2), "g"(cmp3), "g"(cmp4), "g"(cmp5));
        dummy += cmp1 + cmp2 + cmp3 + cmp4 + cmp5;
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
    test_direct_comparisons();
    
    // Print dummy to prevent dead code elimination
    printf("Result: %d\n", dummy);
    
    return 0;
}
