#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1024

// Global variables to prevent optimization
volatile int dummy = 0;
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with mixed values including NaNs
void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Every 4th element is NaN, others are normal numbers
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 2.5;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline))
void test_unordered(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline assembly
        asm volatile("" : : "g"(cmp_result));
        
        // Also use direct comparison that might generate unordered
        int cmp_result2 = (a != a) || (b != b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

// Test ORDERED condition code
__attribute__((noinline))
void test_ordered(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate ORDERED condition code
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative ordered check
        int cmp_result2 = (a == a) && (b == b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline))
void test_uneq(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNEQ condition code
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative using operators
        int cmp_result2 = !(a < b || a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNGE condition code (unordered or greater or equal)
__attribute__((noinline))
void test_unge(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNGE condition code
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative: not less than
        int cmp_result2 = !(a < b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline))
void test_ungt(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNGT condition code
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative: not less or equal
        int cmp_result2 = !(a <= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNLE condition code (unordered or less or equal)
__attribute__((noinline))
void test_unle(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNLE condition code
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative using operators
        int cmp_result2 = !(a > b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline))
void test_unlt(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNLT condition code
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative: not greater or equal
        int cmp_result2 = !(a >= b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test LTGT condition code (less or greater, but not equal and not unordered)
__attribute__((noinline))
void test_ltgt(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate LTGT condition code using builtin
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative using operators
        int cmp_result2 = (a < b || a > b) && !__builtin_isunordered(a, b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

int main(void) {
    // Initialize arrays with mixed normal values and NaNs
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
    
    // Compute checksum from dummy to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += dummy;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
