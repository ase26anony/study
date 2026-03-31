#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_accumulator = 0;

// Optimization barrier functions
__attribute__((noinline)) int get_input(int x) {
    return x ^ 0x55AA55AA;
}

__attribute__((noinline)) int use_result(int x) {
    return x + 1;
}

// Test function 1: MIPS target with simple jump pattern
__attribute__((target("arch=mips32")))
int test_mips_jump_pattern(int a, int b) {
    // Initialize temporaries independent of jump condition
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = temp1 ^ temp2;
    int result = 0;
    
    // Create a non-trivial condition using input
    if (get_input(a) > get_input(b)) {
        // Simple goto to label - should generate simplejump_p
        goto target_label_1;
    }
    
    // Some other code to create basic blocks
    temp3 = temp3 * 2;
    result = use_result(temp3);
    
    // This should not be reached if jump is taken
    return result + 100;

target_label_1:
    // Safe, non-jump instruction after label
    // Uses independent temporaries not used in jump condition
    temp1 = temp2 + 5;  // Simple arithmetic
    
    // Use the result to prevent dead code elimination
    result = use_result(temp1);
    global_accumulator += result;
    return result;
}

// Test function 2: SPARC target with different pattern
__attribute__((target("arch=sparc")))
int test_sparc_jump_pattern(int x, int y) {
    // More temporaries to increase scheduling opportunities
    int t1 = x & 0xFF;
    int t2 = y | 0x55;
    int t3 = t1 + t2;
    int t4 = t2 - t1;
    int result = 0;
    
    volatile int barrier = x;  // Prevent condition optimization
    
    // Another simple jump pattern
    if ((barrier & 1) && (x != y)) {
        goto sparc_target_label;
    }
    
    // Alternative path
    t3 = t3 * 3;
    result = t3 + t4;
    return result;

sparc_target_label:
    // Safe instruction: bitwise operation on temporaries
    t4 = t1 ^ t2;  // Independent of jump condition
    
    result = use_result(t4);
    global_accumulator += result;
    return result;
}

// Test function 3: Generic pattern (rely on -march flag)
int test_generic_jump_pattern(int a, int b, int c) {
    // Multiple independent variables
    int v1 = a + b;
    int v2 = b + c;
    int v3 = c + a;
    int v4 = v1 * v2;
    int result = 0;
    
    // Complex enough condition to not be optimized away
    if ((a > 0) && (b < 100) && (c != 0)) {
        if (v1 > v2) {
            goto generic_target;
        }
    }
    
    // Other control flow
    for (int i = 0; i < 3; i++) {
        v3 += i;
    }
    result = v3 + v4;
    return result;

generic_target:
    // Safe: simple arithmetic with constants
    v4 = v3 + 42;  // Constant addition is safe
    
    result = use_result(v4);
    global_accumulator += result;
    return result;
}

// Test function 4: Nested jumps pattern
__attribute__((target("arch=mips32")))
int test_nested_jump_pattern(int x) {
    int a = x * 2;
    int b = x + 10;
    int c = a ^ b;
    int d = b - a;
    int result = 0;
    
    volatile int cond = x;
    
    // Outer condition
    if (cond > 100) {
        // Inner condition leading to simple jump
        if ((cond & 3) == 0) {
            goto nested_target_label;
        }
        c = c * 2;
    }
    
    d = d + 5;
    result = c + d;
    return result;

nested_target_label:
    // Safe: logical operation
    d = c & 0x7F;  // Mask operation - no traps
    
    result = use_result(d);
    global_accumulator += result;
    return result;
}

// Test function 5: Multiple labels in function
int test_multiple_labels(int a, int b) {
    int t1 = a;
    int t2 = b;
    int t3 = 0;
    int result = 0;
    
    // First basic block with operations
    t1 = t1 * 2;
    
    // Conditional jump to first label
    if (t1 > t2) {
        goto label_a;
    }
    
    t2 = t2 + 10;
    goto label_b;
    
label_a:
    // Safe instruction after first label
    t3 = t1 - t2;  // Simple subtraction
    
    result = use_result(t3);
    global_accumulator += result;
    return result;

label_b:
    // Another safe instruction
    t3 = t1 | t2;  // Bitwise OR
    
    result = use_result(t3);
    global_accumulator += result;
    return result;
}

int main() {
    int checksum = 0;
    
    // Call test functions with varying inputs
    checksum += test_mips_jump_pattern(10, 20);
    checksum += test_mips_jump_pattern(50, 30);
    
    checksum += test_sparc_jump_pattern(100, 200);
    checksum += test_sparc_jump_pattern(300, 150);
    
    checksum += test_generic_jump_pattern(1, 2, 3);
    checksum += test_generic_jump_pattern(4, 5, 6);
    
    checksum += test_nested_jump_pattern(75);
    checksum += test_nested_jump_pattern(125);
    
    checksum += test_multiple_labels(8, 12);
    checksum += test_multiple_labels(25, 15);
    
    // Add global accumulator to checksum
    checksum += global_accumulator;
    
    printf("Checksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return 0;
}
