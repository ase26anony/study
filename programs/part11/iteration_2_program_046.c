#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

// Global variable to prevent optimization
volatile int dummy = 0;

// Arrays for comparison
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with mixed values including NaNs
void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Insert NaN at regular intervals
        if (i % NAN_INTERVAL == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i) * 2.0;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline))
void test_unordered(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Use __builtin_isunordered to generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        // Also use direct comparison
        cmp_result = (a != a) || (b != b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test ORDERED condition code
__attribute__((noinline))
void test_ordered(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate ORDERED condition code
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp_result));
        // Alternative ordered check
        cmp_result = (a == a) && (b == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline))
void test_uneq(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate UNEQ: unordered or equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : : "g"(cmp_result));
        // Alternative using operators
        cmp_result = !(a < b || a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline))
void test_unge(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate UNGE: unordered or a >= b
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : : "g"(cmp_result));
        // Alternative: not less than
        cmp_result = !(a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline))
void test_ungt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate UNGT: unordered or a > b
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : : "g"(cmp_result));
        // Alternative: not less than or equal
        cmp_result = !(a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline))
void test_unle(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate UNLE: unordered or a <= b
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : : "g"(cmp_result));
        // Alternative: not greater than
        cmp_result = !(a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline))
void test_unlt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate UNLT: unordered or a < b
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : : "g"(cmp_result));
        // Alternative: not greater than or equal
        cmp_result = !(a >= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline))
void test_ltgt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate LTGT: a < b or a > b, but not equal and not unordered
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile("" : : "g"(cmp_result));
        // Alternative using operators
        cmp_result = (a < b || a > b) && !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

int main(void) {
    init_arrays();
    
    // Call all test functions to generate various condition codes
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    // Use dummy to prevent optimization
    checksum += dummy;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
