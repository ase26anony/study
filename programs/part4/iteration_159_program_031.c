#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

// Optimization barrier
int __attribute__((noinline)) get_value(int x) {
    return x ^ global_seed;
}

// Test function 1: MIPS target with simple jump pattern
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    // Create temporaries independent of jump condition
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = temp1 ^ temp2;
    
    // Use input-dependent condition to prevent optimization
    if (get_value(a) > get_value(b)) {
        // This should generate a simple jump to label
        goto target_label_1;
    }
    
    // Some other code to create basic blocks
    temp3 = temp3 + 5;
    return temp3;
    
target_label_1:
    // Safe, non-jump instruction after label
    // Uses independent temporary not involved in jump condition
    temp3 = temp3 & 0xFF;
    
    // Use result to prevent elimination
    return temp3 + 1;
}

// Test function 2: SPARC target with different pattern
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    // Multiple independent temporaries
    int t1 = x - y;
    int t2 = x + y;
    int t3 = t1 * t2;
    int t4 = t2 >> 1;
    
    // Complex enough condition to not be optimized away
    if ((get_value(x) & 0xF) != (get_value(y) & 0xF)) {
        goto target_label_2;
    }
    
    // Alternative path
    t3 = t3 | 0x100;
    return t3;
    
target_label_2:
    // Safe arithmetic operation after label
    // Uses temporaries not live across the jump
    t4 = t4 + t1;
    
    return t4 * 2;
}

// Test function 3: Generic pattern with multiple jumps
int test_case_3(int p, int q) {
    int a = p * 3;
    int b = q / 2;  // Safe division by constant 2
    int c = a ^ b;
    int d = b + 7;
    
    // Nested conditions to create more CFG complexity
    if (get_value(p) != 0) {
        if (get_value(q) % 2 == 0) {
            goto target_label_3;
        }
        c = c - 1;
    }
    
    d = d * 3;
    return c + d;
    
target_label_3:
    // Multiple safe operations (will be single instruction after compilation)
    d = d << 2;
    
    return d - a;
}

// Test function 4: Pattern with loop and jump
int test_case_4(int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        int temp = i * 2;
        
        // Jump inside loop
        if (get_value(i) == global_seed) {
            goto target_label_4;
        }
        
        sum += temp;
        continue;
        
    target_label_4:
        // Safe operation after label in loop
        temp = temp | 0x1;
        sum += temp;
    }
    
    return sum;
}

// Test function 5: Multiple labels and jumps
int test_case_5(int x, int y, int z) {
    int r1 = x + y;
    int r2 = y + z;
    int r3 = z + x;
    
    // First possible jump
    if (get_value(x) < get_value(y)) {
        goto label_a;
    }
    
    r1 = r1 * 2;
    
    // Second possible jump
    if (get_value(y) < get_value(z)) {
        goto label_b;
    }
    
    r2 = r2 / 2;  // Safe division by constant
    return r1 + r2 + r3;
    
label_a:
    // Safe operation after first label
    r3 = r3 & 0x7F;
    return r3;
    
label_b:
    // Safe operation after second label
    r2 = r2 ^ r1;
    return r2;
}

int main() {
    int result = 0;
    
    // Call test functions with different inputs
    result += test_case_1(10, 20);
    result += test_case_2(30, 40);
    result += test_case_3(50, 60);
    result += test_case_4(5);
    result += test_case_5(70, 80, 90);
    
    // Also test with volatile inputs to prevent constant folding
    volatile int v1 = 100, v2 = 200, v3 = 300;
    result += test_case_1(v1, v2);
    result += test_case_2(v2, v3);
    result += test_case_3(v3, v1);
    
    printf("Result checksum: %d\n", result);
    global_accumulator = result;
    
    return 0;
}
