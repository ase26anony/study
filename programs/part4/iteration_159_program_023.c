#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_acc = 0;

// Optimization barrier functions
__attribute__((noinline)) int barrier1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) int barrier2(int x) { return x ^ 0xAA55AA55; }

// Test function 1: Basic pattern with independent temporaries
__attribute__((target("arch=mips32")))
int test_pattern1(int a, int b) {
    // Create independent temporary variables
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = a ^ b;
    int result = 0;
    
    // Dynamic condition using function arguments
    if (a > b) {
        // This should compile to a simple jump to label
        goto target_label1;
    }
    
    // Some other code to create basic blocks
    temp1 = barrier1(temp1);
    result = temp1 + temp2;
    goto end_label1;
    
target_label1:
    // Safe, non-jump instruction using independent temporaries
    // This is the instruction we want to be considered for delay slot
    temp3 = temp3 & 0xFF;  // Simple bitwise operation
    
    // Use the result
    result = temp3 + 1;
    
end_label1:
    return barrier2(result);
}

// Test function 2: Pattern with more complex control flow
__attribute__((target("arch=mips32")))
int test_pattern2(int x, int y) {
    int t1 = x * 3;
    int t2 = y / 2;  // Note: safe because y is not zero in our test
    int t3 = t1 ^ t2;
    int t4 = 0;
    
    // Create more basic blocks
    for (int i = 0; i < 2; i++) {
        t4 += i;
    }
    
    // Another dynamic condition
    if (x != y) {
        goto target_label2;
    }
    
    t3 = barrier1(t3);
    return t3 + t4;
    
target_label2:
    // Another safe instruction - arithmetic operation
    t4 = t1 - t2;  // Uses only temporaries defined before jump
    
    return barrier2(t4);
}

// Test function 3: Pattern with multiple labels and jumps
__attribute__((target("arch=sparc")))
int test_pattern3(int p, int q) {
    int local_a = p + 100;
    int local_b = q - 50;
    int local_c = 0;
    int local_d = local_a | local_b;
    
    // Multiple conditions to create CFG
    if (p > 0) {
        if (q < 0) {
            goto target_label3;
        }
        local_c = barrier1(local_a);
    } else {
        local_c = barrier2(local_b);
    }
    
    return local_c + local_d;
    
target_label3:
    // Safe logical operation
    local_d = local_d ^ 0x1234;
    
    return local_d;
}

// Test function 4: Minimal pattern
__attribute__((target("arch=mips32")))
int test_pattern4(int val) {
    volatile int v = val;  // Prevent constant propagation
    int tmp1 = v + 5;
    int tmp2 = v * 2;
    int tmp3 = 0;
    
    // Very simple condition
    if (v != 0) {
        goto target_label4;
    }
    
    tmp3 = tmp1;
    return tmp3;
    
target_label4:
    // Minimal safe instruction
    tmp2 = tmp2 + 1;  // Simple increment
    
    return tmp2;
}

// Test function 5: Pattern with register pressure
__attribute__((target("arch=mips32")))
int test_pattern5(int a, int b, int c, int d) {
    // Create many temporaries to increase register pressure
    int t1 = a + b;
    int t2 = c - d;
    int t3 = a * c;
    int t4 = b | d;
    int t5 = t1 ^ t2;
    int t6 = t3 & t4;
    int t7 = 0;
    
    // Complex condition
    if ((a + b) > (c - d)) {
        goto target_label5;
    }
    
    t7 = barrier1(t5) + barrier2(t6);
    return t7;
    
target_label5:
    // Safe instruction using only some temporaries
    t5 = t5 << 2;  // Simple shift operation
    
    return t5;
}

// Main function that runs all tests
int main() {
    int results[5];
    int checksum = 0;
    
    // Run test with various inputs to ensure different paths
    results[0] = test_pattern1(10, 5);   // a > b, takes jump
    results[1] = test_pattern2(5, 10);   // x != y, takes jump
    results[2] = test_pattern3(1, -1);   // p > 0 && q < 0, takes jump
    results[3] = test_pattern4(42);      // v != 0, takes jump
    results[4] = test_pattern5(10, 20, 5, 15);  // (a+b) > (c-d), takes jump
    
    // Calculate checksum
    for (int i = 0; i < 5; i++) {
        checksum ^= results[i];
        global_acc += results[i];
    }
    
    printf("Test results checksum: 0x%08X\n", checksum);
    printf("Global accumulator: %d\n", global_acc);
    
    return 0;
}
