#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_accumulator = 0;

// Optimization barrier functions
__attribute__((noinline)) int get_input(int x) {
    return x ^ 0x1234;
}

__attribute__((noinline)) void use_value(int x) {
    global_accumulator += x;
}

// Test function 1: Basic pattern with independent temporaries
__attribute__((target("arch=mips32")))
int test_basic_pattern(int a, int b) {
    // Initialize temporaries that are independent of the jump condition
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = 0;
    
    // Create a non-trivial condition for the jump
    if (a > b) {
        // This should generate a simple jump to label
        goto target_label;
    }
    
    // Some other code to create basic blocks
    temp1 = temp1 * 3;
    
target_label:
    // Safe, non-jump instruction using independent temporaries
    // This should be eligible for delay slot filling
    temp3 = temp1 + temp2;
    
    // Use the result
    return temp3;
}

// Test function 2: Multiple independent operations before jump
__attribute__((target("arch=mips32")))
int test_multiple_temps(int x, int y) {
    int t1 = x & 0xFF;
    int t2 = y | 0x55;
    int t3 = t1 ^ t2;
    int t4 = 0;
    int t5 = 0;
    
    // More complex condition using function call
    if (get_input(x) != get_input(y)) {
        t5 = 1;
        goto compute_label;
    }
    
    // Alternative path
    t3 = t3 + 100;
    
compute_label:
    // Safe arithmetic operation - independent of jump condition
    t4 = t3 * 2 + t5;
    
    return t4;
}

// Test function 3: Pattern with volatile to prevent optimization
__attribute__((target("arch=sparc")))
int test_volatile_pattern(volatile int* ptr) {
    int local_temp1 = *ptr;
    int local_temp2 = local_temp1 + 10;
    int local_temp3 = 0;
    int local_temp4 = 0;
    
    // Condition based on volatile read
    if (local_temp1 > 50) {
        local_temp4 = 5;
        goto process_label;
    }
    
    // Different computation
    local_temp2 = local_temp2 - 3;
    
process_label:
    // Safe operation using only local temporaries
    local_temp3 = local_temp2 * local_temp4;
    
    return local_temp3;
}

// Test function 4: Nested control flow with target pattern
__attribute__((target("arch=mips32")))
int test_nested_control(int a, int b, int c) {
    int r1 = a;
    int r2 = b;
    int r3 = c;
    int result = 0;
    
    // Outer loop to create more complex CFG
    for (int i = 0; i < 2; i++) {
        // Inner condition
        if (r1 < r2) {
            r3 = r1 + r2;
            if (r3 > 100) {
                // Target simple jump pattern
                goto final_calc;
            }
        }
        r2 = r2 * 2;
    }
    
    r3 = r3 - 50;
    
final_calc:
    // Safe instruction after label
    result = r3 & 0x7F;
    
    return result;
}

// Test function 5: Multiple labels and jumps
__attribute__((target("arch=sparc")))
int test_multiple_labels(int x) {
    int v1 = x;
    int v2 = x * 2;
    int v3 = 0;
    int v4 = 0;
    
    // First conditional jump
    if (v1 > 0) {
        v4 = 10;
        goto label_a;
    }
    
    v2 = v2 + 5;
    
label_a:
    // Safe operation - potential delay slot candidate
    v3 = v2 - v4;
    
    // Second conditional jump
    if (v3 < 0) {
        goto label_b;
    }
    
    v1 = v1 * 3;
    
label_b:
    // Another safe operation
    v1 = v1 + v3;
    
    return v1;
}

// Main test driver
int main() {
    int result = 0;
    
    // Test with various inputs to exercise different paths
    result += test_basic_pattern(10, 5);
    result += test_basic_pattern(5, 10);
    
    result += test_multiple_temps(100, 200);
    result += test_multiple_temps(200, 100);
    
    volatile int vol_val1 = 60;
    volatile int vol_val2 = 40;
    result += test_volatile_pattern(&vol_val1);
    result += test_volatile_pattern(&vol_val2);
    
    result += test_nested_control(30, 40, 50);
    result += test_nested_control(80, 20, 10);
    
    result += test_multiple_labels(25);
    result += test_multiple_labels(-25);
    
    // Also call use_value to create side effects
    use_value(result);
    
    printf("Accumulated result: %d\n", result);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return 0;
}
