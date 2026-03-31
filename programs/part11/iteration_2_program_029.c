#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

// Global variable to prevent optimization
volatile int dummy = 0;

// Arrays with mixed normal values and NaNs
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with pattern including NaNs
__attribute__((constructor)) void init_arrays() {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i % 100) * 1.5;
        // Every 4th element in array2 is NaN, others are normal
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i % 50) * 2.0;
        }
    }
}

// Test UNORDERED condition code (unord)
__attribute__((noinline)) void test_unordered() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Force condition code output through inline asm
        asm volatile("" : : "g"(cmp_result));
        
        // Also use direct comparison that might generate unord
        asm volatile("" : : "g"(a != a || b != b));
    }
}

// Test ORDERED condition code (ord)
__attribute__((noinline)) void test_ordered() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate ORDERED condition code
        int cmp_result = !__builtin_isunordered(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative ordered check
        asm volatile("" : : "g"(a == a && b == b));
    }
}

// Test UNEQ condition code (ueq)
__attribute__((noinline)) void test_uneq() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNEQ condition code: unordered or equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative using builtin
        asm volatile("" : : "g"(!(a < b || a > b)));
    }
}

// Test UNGE condition code (nlt)
__attribute__((noinline)) void test_unge() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNGE condition code: unordered or not less than
        int cmp_result = __builtin_isunordered(a, b) || !(a < b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative: not less than (including unordered)
        asm volatile("" : : "g"(!(a < b)));
    }
}

// Test UNGT condition code (nle)
__attribute__((noinline)) void test_ungt() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNGT condition code: unordered or not less or equal
        int cmp_result = __builtin_isunordered(a, b) || !(a <= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative: greater than or unordered
        asm volatile("" : : "g"(a > b || __builtin_isunordered(a, b)));
    }
}

// Test UNLE condition code (ule)
__attribute__((noinline)) void test_unle() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNLE condition code: unordered or less or equal
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative: not greater than (including unordered)
        asm volatile("" : : "g"(!(a > b)));
    }
}

// Test UNLT condition code (ult)
__attribute__((noinline)) void test_unlt() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate UNLT condition code: unordered or less than
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative: less than or unordered
        asm volatile("" : : "g"(a < b || __builtin_isunordered(a, b)));
    }
}

// Test LTGT condition code (une)
__attribute__((noinline)) void test_ltgt() {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Generate LTGT condition code: less than or greater than (but not unordered)
        int cmp_result = __builtin_islessgreater(a, b);
        
        // Force condition code output
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative using direct comparisons
        asm volatile("" : : "g"((a < b) || (a > b)));
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
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (int)(array1[i] + array2[i]);
    }
    
    // Print checksum to ensure code isn't optimized away
    printf("Checksum: %d\n", checksum + dummy);
    
    return 0;
}
