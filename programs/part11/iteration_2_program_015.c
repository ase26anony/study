#include <stdio.h>
#include <stdint.h>
#include <math.h>

// Global variable to prevent optimization
volatile int global_dummy = 0;

// Arrays for testing
#define ARRAY_SIZE 1024
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with mixed normal values and NaNs
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Every 4th element is NaN, others are normal values
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i % 10) * 0.7;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline)) void test_unordered() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Use __builtin_isunordered to generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        // Inline assembly that uses the condition code
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
    global_dummy += dummy;
}

// Test ORDERED condition code
__attribute__((noinline)) void test_ordered() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Ordered is the opposite of unordered
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
    global_dummy += dummy;
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Unordered or equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
    global_dummy += dummy;
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline)) void test_unge() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Unordered or greater than or equal
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
    global_dummy += dummy;
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline)) void test_ungt() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Unordered or greater than
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
    global_dummy += dummy;
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline)) void test_unle() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Unordered or less than or equal
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
    global_dummy += dummy;
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline)) void test_unlt() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Unordered or less than
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
    global_dummy += dummy;
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline)) void test_ltgt() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Use __builtin_islessgreater for LTGT condition code
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
    global_dummy += dummy;
}

// Additional test with direct comparisons that might generate specific condition codes
__attribute__((noinline)) void test_direct_comparisons() {
    int dummy = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Test various direct comparisons that might generate different condition codes
        int cmp1 = (a < b);   // Might generate LT
        int cmp2 = (a > b);   // Might generate GT
        int cmp3 = (a <= b);  // Might generate LE
        int cmp4 = (a >= b);  // Might generate GE
        int cmp5 = (a == b);  // Might generate EQ
        int cmp6 = (a != b);  // Might generate NEQ
        
        // Feed all comparisons to inline assembly
        asm volatile("" : "+r"(dummy) : "g"(cmp1), "g"(cmp2), "g"(cmp3), 
                                 "g"(cmp4), "g"(cmp5), "g"(cmp6));
    }
    global_dummy += dummy;
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
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", global_dummy);
    
    return 0;
}
