#include <stdio.h>
#include <stdint.h>
#include <math.h>

// Global variable to prevent optimization
volatile int dummy = 0;

// Arrays with mixed NaN and normal values
#define SIZE 1024
double array1[SIZE];
double array2[SIZE];

// Initialize arrays with pattern including NaNs
void init_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (double)(i + 1);
        // Every 4th element is NaN, others are normal values
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(SIZE - i);
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline))
void test_unordered(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Use __builtin_isunordered to generate UNORDERED condition code
        int cmp = __builtin_isunordered(a, b);
        // Inline asm to force condition code output
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp; // Prevent dead code elimination
    }
}

// Test ORDERED condition code  
__attribute__((noinline))
void test_ordered(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Ordered is the opposite of unordered
        int cmp = !__builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline))
void test_uneq(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Use direct comparison to generate UNEQ
        int cmp = (a == b) || __builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

// Test UNGE condition code (unordered or greater or equal)
__attribute__((noinline))
void test_unge(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate UNGE: !(a < b) which includes unordered cases
        int cmp = !(a < b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

// Test UNGT condition code (unordered or greater)
__attribute__((noinline))
void test_ungt(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate UNGT: !(a <= b) which includes unordered cases
        int cmp = !(a <= b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

// Test UNLE condition code (unordered or less or equal)
__attribute__((noinline))
void test_unle(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate UNLE: !(a > b) which includes unordered cases
        int cmp = !(a > b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

// Test UNLT condition code (unordered or less)
__attribute__((noinline))
void test_unlt(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Generate UNLT: !(a >= b) which includes unordered cases
        int cmp = !(a >= b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

// Test LTGT condition code (less or greater, but not equal and not unordered)
__attribute__((noinline))
void test_ltgt(void) {
    for (int i = 0; i < SIZE; i++) {
        double a = array1[i];
        double b = array2[i];
        // Use __builtin_islessgreater to generate LTGT
        int cmp = __builtin_islessgreater(a, b);
        asm volatile("" : : "g"(cmp) : "memory");
        dummy += cmp;
    }
}

// Additional test with float type to ensure different code generation
__attribute__((noinline))
void test_float_comparisons(void) {
    float fa[SIZE];
    float fb[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        fa[i] = (float)(i + 1);
        fb[i] = (i % 3 == 0) ? __builtin_nanf("") : (float)(SIZE - i);
    }
    
    // Test various float comparisons
    for (int i = 0; i < SIZE; i++) {
        float a = fa[i];
        float b = fb[i];
        
        // Test UNORDERED with floats
        int cmp1 = __builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp1) : "memory");
        
        // Test ORDERED with floats
        int cmp2 = !__builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp2) : "memory");
        
        // Test UNEQ with floats
        int cmp3 = (a == b) || __builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp3) : "memory");
        
        dummy += cmp1 + cmp2 + cmp3;
    }
}

int main(void) {
    init_arrays();
    
    // Call all test functions in sequence
    test_unordered();      // Should trigger "unord" output
    test_ordered();        // Should trigger "ord" output
    test_uneq();           // Should trigger "ueq" output
    test_unge();           // Should trigger "nlt" output
    test_ungt();           // Should trigger "nle" output
    test_unle();           // Should trigger "ule" output
    test_unlt();           // Should trigger "ult" output
    test_ltgt();           // Should trigger "une" output
    
    // Additional float tests
    test_float_comparisons();
    
    // Print dummy to prevent optimization
    printf("Result: %d\n", dummy);
    
    return 0;
}
