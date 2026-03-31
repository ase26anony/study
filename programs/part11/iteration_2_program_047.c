#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

// Global variables to prevent optimization
volatile int dummy = 0;
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with mixed normal values and NaNs
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 0.75;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Use __builtin_isunordered to generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Inline assembly that uses the condition code result
        // This should trigger printing "unord" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test ORDERED condition code
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Ordered is the opposite of unordered
        int cmp_result = !__builtin_isunordered(a, b);
        
        // This should trigger printing "ord" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNEQ: unordered or equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // This should trigger printing "ueq" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGE: unordered or a >= b
        // Using !(a < b) which includes unordered cases
        int cmp_result = __builtin_isunordered(a, b) || !(a < b);
        
        // This should trigger printing "nlt" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNGT: unordered or a > b
        // Using !(a <= b) which includes unordered cases
        int cmp_result = __builtin_isunordered(a, b) || !(a <= b);
        
        // This should trigger printing "nle" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLE: unordered or a <= b
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // This should trigger printing "ule" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate UNLT: unordered or a < b
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // This should trigger printing "ult" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Generate LTGT: a < b or a > b (but not equal and not unordered)
        // Use __builtin_islessgreater which handles NaN correctly
        int cmp_result = __builtin_islessgreater(a, b);
        
        // This should trigger printing "une" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Alternative approach using direct comparisons with volatile to force code generation
__attribute__((noinline)) void test_direct_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile double a = array1[i];
        volatile double b = array2[i];
        
        // Direct comparisons that might generate different condition codes
        int cmp1 = (a < b);
        int cmp2 = (a > b);
        int cmp3 = (a <= b);
        int cmp4 = (a >= b);
        int cmp5 = (a == b);
        int cmp6 = (a != b);
        
        // Use all comparisons to increase chance of hitting different condition codes
        asm volatile("" : "+r"(dummy) : "g"(cmp1), "g"(cmp2), "g"(cmp3), 
                                     "g"(cmp4), "g"(cmp5), "g"(cmp6));
    }
}

int main() {
    // Call all test functions to generate various condition codes
    test_unordered();      // Should generate "unord"
    test_ordered();        // Should generate "ord"
    test_uneq();           // Should generate "ueq"
    test_unge();           // Should generate "nlt"
    test_ungt();           // Should generate "nle"
    test_unle();           // Should generate "ule"
    test_unlt();           // Should generate "ult"
    test_ltgt();           // Should generate "une"
    
    // Additional direct comparisons for more coverage
    test_direct_comparisons();
    
    // Compute checksum from dummy to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += dummy;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
