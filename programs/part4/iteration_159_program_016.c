#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_acc = 0;

// Optimization barrier functions
__attribute__((noinline)) int get_value(int x) {
    return x ^ 0x55AA;
}

__attribute__((noinline)) int use_value(int x) {
    return x + 1;
}

// Test 1: Basic pattern with independent temporaries
__attribute__((target("arch=mips32")))
int test_pattern1(int a, int b) {
    // Create temporaries that are independent of the jump condition
    int temp1 = a + b;
    int temp2 = a - b;
    int temp3 = a * 2;
    int result = 0;
    
    // Dynamic condition using function arguments
    if (a > b) {
        // This should compile to a simple jump to label
        goto target_label1;
    }
    
    // Some other code to create basic blocks
    temp1 = get_value(temp1);
    result = temp1;
    goto end1;
    
target_label1:
    // Safe, non-jump instruction using independent temporaries
    // This instruction should be eligible for delay slot filling
    temp3 = temp2 + 7;  // Simple arithmetic, no traps
    
    result = temp3;
    
end1:
    return use_value(result);
}

// Test 2: Pattern with more complex control flow
__attribute__((target("arch=mips32")))
int test_pattern2(int x, int y) {
    int t1 = x & 0xFF;
    int t2 = y & 0xFF;
    int t3 = t1 | t2;
    int t4 = t1 ^ t2;
    int result = 0;
    
    // Multiple conditions to create more complex CFG
    if (x != 0) {
        if (y % 2 == 0) {
            // Simple jump to label
            goto target_label2;
        }
        t3 = get_value(t3);
    }
    
    result = t3 + t4;
    goto end2;
    
target_label2:
    // Safe instruction: bitwise operation on locals
    t4 = (t4 << 1) | 1;  // No trapping operations
    
    result = t4;
    
end2:
    return use_value(result);
}

// Test 3: Pattern with loop around the target
__attribute__((target("arch=sparc")))
int test_pattern3(int n, int seed) {
    int i, acc = seed;
    int tmp1 = seed * 3;
    int tmp2 = seed / 2;  // Division by constant 2 is safe
    int tmp3 = 0;
    
    for (i = 0; i < n; i++) {
        if ((i + seed) % 3 == 0) {
            // Simple jump within loop
            goto target_label3;
        }
        acc += i;
        continue;
        
    target_label3:
        // Safe instruction in loop context
        tmp3 = tmp1 - tmp2;  // Simple arithmetic
        
        acc += tmp3;
    }
    
    return use_value(acc);
}

// Test 4: Nested condition pattern
__attribute__((target("arch=mips32")))
int test_pattern4(int a, int b, int c) {
    int x = a + b;
    int y = b + c;
    int z = c + a;
    int w = 0;
    
    // Complex condition to prevent trivial optimization
    if (a > 0 && b < 100 && c != 0) {
        if ((a ^ b) > c) {
            // Simple jump
            goto target_label4;
        }
        x = get_value(x);
    }
    
    w = x + y + z;
    goto end4;
    
target_label4:
    // Multiple safe instructions (compiler might schedule one into delay slot)
    z = y - x;  // Safe: subtraction of locals
    // Note: Only the first instruction after label is considered
    
    w = z;
    
end4:
    return use_value(w);
}

// Test 5: Pattern with volatile to prevent optimization
__attribute__((target("arch=mips32")))
int test_pattern5(volatile int* ptr) {
    int val = *ptr;  // Volatile read creates optimization barrier
    int t1 = val & 0x0F;
    int t2 = (val >> 4) & 0x0F;
    int t3 = t1 * t2;
    int result = 0;
    
    if (val > 100) {
        // Simple jump with volatile-based condition
        goto target_label5;
    }
    
    result = t3;
    goto end5;
    
target_label5:
    // Safe logical operation
    t3 = t1 | t2;  // No memory access, no traps
    
    result = t3;
    
end5:
    return use_value(result);
}

int main() {
    int result = 0;
    volatile int vol_val = 42;
    
    // Run all test patterns with different inputs
    result ^= test_pattern1(10, 5);
    result ^= test_pattern1(5, 10);
    
    result ^= test_pattern2(100, 50);
    result ^= test_pattern2(0, 255);
    
    result ^= test_pattern3(5, 1);
    result ^= test_pattern3(3, 10);
    
    result ^= test_pattern4(1, 2, 3);
    result ^= test_pattern4(50, 30, 20);
    
    result ^= test_pattern5(&vol_val);
    vol_val = 200;
    result ^= test_pattern5(&vol_val);
    
    // Add to global to ensure all results are used
    global_acc += result;
    
    printf("Result checksum: %d\n", result);
    printf("Global accumulator: %d\n", global_acc);
    
    return 0;
}
