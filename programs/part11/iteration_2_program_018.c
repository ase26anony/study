#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

// Global dummy variable to prevent optimization
volatile int dummy = 0;

// Arrays with mixed normal values and NaNs
double array1[ARRAY_SIZE];
double array2[ARRAY_SIZE];

// Initialize arrays with pattern including NaNs
void init_arrays(void) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (double)(i + 1) * 1.5;
        // Every 4th element is NaN, others are normal
        if (i % 4 == 0) {
            array2[i] = __builtin_nan("");
        } else {
            array2[i] = (double)(i + 1) * 0.75;
        }
    }
}

// Test UNORDERED condition code
__attribute__((noinline))
void test_unordered(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Use __builtin_isunordered to generate UNORDERED condition code
        int cmp_result = __builtin_isunordered(a, b);
        
        // Inline assembly that uses the comparison result
        // This should trigger printing of "unord" in i386.cc
        asm volatile("" : : "g"(cmp_result));
        
        // Also use direct comparison with unordered semantics
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
        
        // Ordered comparison - opposite of unordered
        int cmp_result = !__builtin_isunordered(a, b);
        
        // This should trigger printing of "ord" in i386.cc
        asm volatile("" : : "g"(cmp_result));
        
        // Alternative ordered check
        int cmp_result2 = (a == a) && (b == b);
        asm volatile("" : "+r"(dummy) : "g"(cmp_result2));
    }
}

// Test UNEQ condition code (unordered or equal)
__attribute__((noinline))
void test_uneq(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Unordered or equal
        int cmp_result = __builtin_isunordered(a, b) || (a == b);
        
        // This should trigger printing of "ueq" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative implementation
        int cmp_result2 = !(a < b || a > b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

// Test UNGE condition code (unordered or greater than or equal)
__attribute__((noinline))
void test_unge(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Unordered or a >= b
        int cmp_result = __builtin_isunordered(a, b) || (a >= b);
        
        // This should trigger printing of "nlt" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative: not less than (including unordered)
        int cmp_result2 = !(a < b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

// Test UNGT condition code (unordered or greater than)
__attribute__((noinline))
void test_ungt(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Unordered or a > b
        int cmp_result = __builtin_isunordered(a, b) || (a > b);
        
        // This should trigger printing of "nle" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative: not less than or equal (including unordered)
        int cmp_result2 = !(a <= b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

// Test UNLE condition code (unordered or less than or equal)
__attribute__((noinline))
void test_unle(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Unordered or a <= b
        int cmp_result = __builtin_isunordered(a, b) || (a <= b);
        
        // This should trigger printing of "ule" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative implementation
        int cmp_result2 = !(a > b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

// Test UNLT condition code (unordered or less than)
__attribute__((noinline))
void test_unlt(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Unordered or a < b
        int cmp_result = __builtin_isunordered(a, b) || (a < b);
        
        // This should trigger printing of "ult" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative implementation
        int cmp_result2 = !(a >= b);
        asm volatile("" : : "g"(cmp_result2));
    }
}

// Test LTGT condition code (less than or greater than, but not equal and not unordered)
__attribute__((noinline))
void test_ltgt(void) {
    for (int i = 0; i < ITERATIONS; i++) {
        double a = array1[i % ARRAY_SIZE];
        double b = array2[i % ARRAY_SIZE];
        
        // Use __builtin_islessgreater for LTGT condition code
        int cmp_result = __builtin_islessgreater(a, b);
        
        // This should trigger printing of "une" in i386.cc
        asm volatile("" : "+r"(dummy) : "g"(cmp_result));
        
        // Alternative: (a < b) || (a > b) but both ordered
        int cmp_result2 = (a < b || a > b) && !__builtin_isunordered(a, b);
        asm volatile("" : : "g"(cmp_result2));
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
    
    // Compute checksum from dummy variable to prevent dead code elimination
    int checksum = dummy;
    
    // Additional volatile operations to ensure code isn't optimized away
    for (int i = 0; i < 100; i++) {
        asm volatile("" : "+r"(checksum));
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
