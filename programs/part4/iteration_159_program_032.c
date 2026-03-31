#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_acc = 0;

// Optimization barrier functions
__attribute__((noinline)) int barrier1(int x) { return x ^ 0x55AA; }
__attribute__((noinline)) int barrier2(int x) { return x * 3 + 1; }
__attribute__((noinline)) int barrier3(int x) { return x & 0xFF; }

// Test function for MIPS target
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b, int c) {
    // Initialize temporaries independent of jump condition
    int tmp1 = a + 1;
    int tmp2 = b * 2;
    int tmp3 = c & 0xF;
    int tmp4 = tmp1 ^ tmp2;
    
    // Create simple conditional jump to label
    // Use input-dependent condition that's not trivially true/false
    if (a > b + c) {
        // This should compile to a simple jump to target_label
        goto target_label;
    }
    
    // Some other code to create basic blocks
    tmp3 = barrier1(tmp3);
    
    // Target label with safe instruction immediately after
target_label:
    // Safe, non-jump instruction using independent temporaries
    // No trapping operations, no memory access
    tmp4 = tmp1 + tmp3 * 2;
    
    // Use result to prevent elimination
    return barrier2(tmp4) + tmp2;
}

// Test function for SPARC target
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y, int z) {
    // Independent temporaries
    int t1 = x | 0x01;
    int t2 = y << 1;
    int t3 = z - 1;
    int t4 = 0;
    
    // More complex control flow around the target pattern
    for (int i = 0; i < 3; i++) {
        t1 = barrier3(t1);
    }
    
    // Simple conditional jump
    if ((x ^ y) < z) {
        goto sparc_target;
    }
    
    t2 = barrier1(t2);
    
sparc_target:
    // Safe arithmetic operation after label
    t4 = (t1 & 0x7F) + (t3 >> 1);
    
    return t4 * 2 + t2;
}

// Generic test function with multiple basic blocks
int test_case_3(int p, int q, int r) {
    int local1 = p + q;
    int local2 = q - r;
    int local3 = r * 2;
    int local4 = 0;
    
    // Multiple basic blocks
    if (p > 0) {
        local1 = barrier2(local1);
    } else {
        local2 = barrier3(local2);
    }
    
    // Another condition to create jump to label
    volatile int cond = p;  // Prevent constant propagation
    if (cond % 2 == 0) {
        goto generic_target;
    }
    
    local3 = barrier1(local3);
    
generic_target:
    // Safe instruction: bitwise operation on locals
    local4 = (local1 ^ local2) | local3;
    
    return local4 + p;
}

// Test with nested control flow
int test_case_4(int a, int b) {
    int x = a * 3;
    int y = b + 5;
    int z = 0;
    int w = 0;
    
    // Create multiple basic blocks
    for (int i = 0; i < 2; i++) {
        x = barrier1(x);
        y = barrier2(y);
    }
    
    // Simple jump condition
    if (x != y) {
        goto nested_target;
    }
    
    z = barrier3(z);
    
nested_target:
    // Safe operation: shift and add
    w = (x << 1) + (y >> 1);
    
    return w + z;
}

// Main function that calls all test cases
int main() {
    int result = 0;
    
    // Call test functions with different inputs
    // to explore different paths
    result += test_case_1(10, 5, 3);   // a > b + c is true
    result += test_case_1(1, 5, 3);    // a > b + c is false
    
    result += test_case_2(7, 3, 15);   // (x^y) < z is true
    result += test_case_2(20, 20, 1);  // (x^y) < z is false
    
    result += test_case_3(4, 6, 2);    // p%2 == 0 is true
    result += test_case_3(5, 6, 2);    // p%2 == 0 is false
    
    result += test_case_4(10, 20);     // x != y is true
    result += test_case_4(15, 15);     // x != y is false
    
    // Update global to create side effect
    global_acc = result;
    
    printf("Result checksum: %d\n", result);
    printf("Global accumulator: %d\n", global_acc);
    
    return result != 0 ? 0 : 1;
}
