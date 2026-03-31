#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

// Global variable to prevent optimization
volatile int dummy = 0;

// Arrays for testing
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with mixed normal values and NaNs
void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i) * 1.2;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline)) void test_unordered(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Use __builtin_isunordered to generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        // Force condition code into assembly output
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test ORDERED condition code
__attribute__((noinline)) void test_ordered(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Ordered is the opposite of unordered
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNEQ: unordered or equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline)) void test_unge(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNGE: unordered or a >= b
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline)) void test_ungt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNGT: unordered or a > b
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline)) void test_unle(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNLE: unordered or a <= b
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline)) void test_unlt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNLT: unordered or a < b
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline)) void test_ltgt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // LTGT: less than or greater than (but not equal, not unordered)
        // Use __builtin_islessgreater which maps directly to LTGT
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile("" : : "g"(cmp_result));
        dummy += cmp_result;
    }
}

// Alternative implementation using direct comparisons for some codes
__attribute__((noinline)) void test_direct_comparisons(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Test various comparison operators that might generate different condition codes
        int cmp1 = (a != b);  // NE
        int cmp2 = (a < b);   // LT
        int cmp3 = (a > b);   // GT
        int cmp4 = (a <= b);  // LE
        int cmp5 = (a >= b);  // GE
        
        // Force all condition codes into assembly
        asm volatile("" : : "g"(cmp1), "g"(cmp2), "g"(cmp3), "g"(cmp4), "g"(cmp5));
        dummy += cmp1 + cmp2 + cmp3 + cmp4 + cmp5;
    }
}

int main(void) {
    init_arrays();
    
    // Call all test functions to generate various condition codes
    test_unordered();      // Should generate UNORDERED
    test_ordered();        // Should generate ORDERED  
    test_uneq();           // Should generate UNEQ
    test_unge();           // Should generate UNGE
    test_ungt();           // Should generate UNGT
    test_unle();           // Should generate UNLE
    test_unlt();           // Should generate UNLT
    test_ltgt();           // Should generate LTGT
    test_direct_comparisons(); // Additional comparisons
    
    // Print dummy to prevent dead code elimination
    printf("Result: %d\n", dummy);
    
    return 0;
}
