#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_accumulator = 0;

// Optimization barrier functions
__attribute__((noinline)) int get_input(int x) {
    return x ^ 0x55AA55AA;
}

__attribute__((noinline)) int use_result(int x) {
    global_accumulator += x;
    return x;
}

// Test function 1: MIPS target with simple jump pattern
__attribute__((target("arch=mips32")))
int test_mips_delay_slot_1(int a, int b) {
    // Initialize temporaries independent of jump condition
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = temp1 ^ temp2;
    int temp4 = 0;
    
    // Create dynamic condition for goto
    if (a > b) {
        // This should generate a simple jump to label
        goto target_label_1;
    }
    
    // Some other code to create CFG complexity
    temp3 = temp3 * 3;
    return use_result(temp3);
    
target_label_1:
    // Safe, non-jump instruction after label
    temp4 = temp1 + temp2;  // Simple arithmetic on already-defined locals
    return use_result(temp4);
}

// Test function 2: SPARC target with different pattern
__attribute__((target("arch=sparc")))
int test_sparc_delay_slot_2(int x, int y) {
    // Independent temporaries
    int t1 = x & 0xFF;
    int t2 = y | 0x55;
    int t3 = t1 - t2;
    int result = 0;
    
    // More complex condition to prevent optimization
    volatile int cond = get_input(x);
    if ((cond & 1) && (x != y)) {
        goto sparc_target;
    }
    
    // Alternative path
    t3 = t3 >> 2;
    return use_result(t3);
    
sparc_target:
    // Safe instruction using only pre-defined temporaries
    result = t1 * t2;  // Multiplication is safe with integers
    return use_result(result);
}

// Test function 3: Generic pattern with multiple basic blocks
__attribute__((noinline))
int test_generic_delay_slot_3(int val) {
    int a = val + 100;
    int b = val - 50;
    int c = 0;
    int d = a ^ b;
    
    // Create multiple basic blocks
    for (int i = 0; i < 3; i++) {
        d += i;
    }
    
    // Jump condition based on processed value
    if ((d & 0xF) == 0) {
        goto generic_label;
    }
    
    // Different computation
    c = a * b;
    return use_result(c);
    
generic_label:
    // Safe bitwise operation after label
    c = a & b;  // No trapping possible
    return use_result(c);
}

// Test function 4: Nested control flow with safe instruction
__attribute__((target("arch=mips32")))
int test_nested_pattern_4(int p, int q) {
    int x = p * 2;
    int y = q / 2;  // Division is safe here (compile-time known divisor)
    int z = 0;
    int w = x | y;
    
    // Nested condition
    if (p > 0) {
        if (q < 100) {
            if (p != q) {
                goto nested_target;
            }
        }
    }
    
    w = w + 5;
    return use_result(w);
    
nested_target:
    // Safe shift operation
    z = x << 2;  // No trapping, uses pre-defined variable
    return use_result(z);
}

// Test function 5: Multiple jumps to same label
__attribute__((target("arch=sparc")))
int test_multi_jump_5(int a, int b, int c) {
    int t1 = a + b;
    int t2 = b + c;
    int t3 = c + a;
    int res = 0;
    
    // Multiple jumps to same label
    if (a > 10) {
        goto common_target;
    }
    
    if (b < 20) {
        goto common_target;
    }
    
    if (c == 30) {
        goto common_target;
    }
    
    res = t1 + t2 + t3;
    return use_result(res);
    
common_target:
    // Safe instruction - logical operation
    res = t1 ^ t2 ^ t3;
    return use_result(res);
}

// Main function that exercises all patterns
int main() {
    int result = 0;
    
    // Test with various inputs to explore different paths
    for (int i = 0; i < 10; i++) {
        result ^= test_mips_delay_slot_1(i, i*2);
        result ^= test_sparc_delay_slot_2(i+1, i*3);
        result ^= test_generic_delay_slot_3(i+2);
        result ^= test_nested_pattern_4(i+3, i*4);
        result ^= test_multi_jump_5(i, i+1, i+2);
    }
    
    // Add global accumulator to result
    result ^= global_accumulator;
    
    printf("Result checksum: %d\n", result);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return 0;
}
