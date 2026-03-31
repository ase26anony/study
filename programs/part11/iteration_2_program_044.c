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

// Initialize arrays with pattern including NaNs
void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
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
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline asm
        asm volatile("" : : "g"(cmp_result));
        
        // Also use direct comparison that might generate UNORDERED
        int cmp_result2 = !(a == b) && !(a < b) && !(a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test ORDERED condition code
__attribute__((noinline))
void test_ordered(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative ordered check
        int cmp_result2 = (a == b) || (a < b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNEQ condition code
__attribute__((noinline))
void test_uneq(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // UNEQ: unordered or equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative using operators
        int cmp_result2 = !(a != b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNGE condition code
__attribute__((noinline))
void test_unge(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // UNGE: unordered or greater than or equal
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative: not less than
        int cmp_result2 = !(a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNGT condition code
__attribute__((noinline))
void test_ungt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // UNGT: unordered or greater than
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative: not less than or equal
        int cmp_result2 = !(a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNLE condition code
__attribute__((noinline))
void test_unle(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // UNLE: unordered or less than or equal
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Direct comparison
        int cmp_result2 = (a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNLT condition code
__attribute__((noinline))
void test_unlt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // UNLT: unordered or less than
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Direct comparison
        int cmp_result2 = (a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test LTGT condition code
__attribute__((noinline))
void test_ltgt(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        
        // LTGT: less than or greater than (ordered and not equal)
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative using operators
        int cmp_result2 = (a < b) || (a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

int main(void) {
    // Initialize arrays with NaNs and normal numbers
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
