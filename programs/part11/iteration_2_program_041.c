#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define NAN_INTERVAL 4

// Global variable to prevent optimization
volatile int dummy = 0;

// Arrays with mixed normal values and NaNs
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with pattern including NaNs
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

// Test functions for each condition code type
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNORDERED: true if either operand is NaN
        int cmp_result = __builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // ORDERED: true if neither operand is NaN
        int cmp_result = !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNEQ: unordered or equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNGE: unordered or greater than or equal
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNGT: unordered or greater than
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNLE: unordered or less than or equal
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // UNLT: unordered or less than
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // LTGT: less than or greater than (ordered and not equal)
        int cmp_result = __builtin_islessgreater(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
    }
}

// Additional test with direct comparisons to ensure all codes are generated
__attribute__((noinline)) void test_mixed_comparisons() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // Direct comparisons that should generate various condition codes
        int cmp1 = (a == b);
        int cmp2 = (a != b);
        int cmp3 = (a < b);
        int cmp4 = (a > b);
        int cmp5 = (a <= b);
        int cmp6 = (a >= b);
        
        // Feed all comparisons to asm statements
        asm volatile("" : "+r"(dummy) : "g"(cmp1));
        asm volatile("" : "+r"(dummy) : "g"(cmp2));
        asm volatile("" : "+r"(dummy) : "g"(cmp3));
        asm volatile("" : "+r"(dummy) : "g"(cmp4));
        asm volatile("" : "+r"(dummy) : "g"(cmp5));
        asm volatile("" : "+r"(dummy) : "g"(cmp6));
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
    test_mixed_comparisons();
    
    // Use dummy to compute checksum and prevent optimization
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += dummy;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
