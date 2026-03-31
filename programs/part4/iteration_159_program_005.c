#include <stdio.h>
#include <stdlib.h>

// Global accumulator to prevent optimization
volatile int global_acc = 0;

// Optimization barrier functions
__attribute__((noinline)) int barrier(int x) {
    volatile int v = x;
    return v;
}

// Test function 1: MIPS target with simple conditional jump
__attribute__((target("arch=mips32")))
int test_mips_simple_jump(int a, int b) {
    // Initialize temporaries that aren't used in the condition
    int temp1 = barrier(a);
    int temp2 = barrier(b);
    int temp3 = 0;
    
    // Create a non-trivial condition using input-dependent values
    if (barrier(a) > barrier(b)) {
        // Simple goto to label - should generate simplejump_p
        goto target_label;
    }
    
    // Some other code to create basic blocks
    temp3 = temp1 + temp2;
    
    // This should be unreachable if condition is true
    return temp3 * 2;

target_label:
    // Safe, non-jump instruction using temporaries
    // Simple arithmetic that doesn't trap
    temp3 = temp1 - temp2;
    
    // Use the result to prevent dead code elimination
    return temp3 + 1;
}

// Test function 2: SPARC target with more complex flow
__attribute__((target("arch=sparc")))
int test_sparc_jump_with_loop(int x, int y) {
    int i, j, k;
    int result = 0;
    
    // Initialize independent temporaries
    i = barrier(x);
    j = barrier(y);
    k = i * j;
    
    // Create a loop to build CFG complexity
    for (int n = 0; n < 3; n++) {
        result += barrier(n);
    }
    
    // Non-trivial condition
    if (barrier(i) != barrier(j) && barrier(k) > 10) {
        // This should be a simple jump to label
        goto compute_label;
    }
    
    // Alternative path
    result = i + j;
    return result;

compute_label:
    // Safe instruction: bitwise operation on locals
    // Uses variables not referenced in the jump condition
    int mask = 0xFF;
    result = (k & mask) | (i ^ j);
    
    return result + 2;
}

// Test function 3: Generic pattern with volatile reads
int test_generic_volatile_jump(volatile int* ptr) {
    int a = *ptr;  // Volatile read
    int b = barrier(a + 1);
    int c = barrier(a - 1);
    int d = 0;
    
    // Use the volatile value in condition
    if (a > 100 && b < 200) {
        // Simple jump to label
        goto process_label;
    }
    
    d = a * b;
    return d;

process_label:
    // Safe arithmetic on independent variables
    // c is not used in the condition above
    d = c * 3 + 7;
    
    return d;
}

// Test function 4: Multiple basic blocks with nested conditions
__attribute__((target("arch=mips32")))
int test_mips_complex_cfg(int p, int q) {
    int t1 = barrier(p);
    int t2 = barrier(q);
    int t3 = t1 + t2;
    int t4 = t1 - t2;
    
    // First condition
    if (t1 > 0) {
        // Second condition
        if (t2 < 0) {
            // This is our target simple jump
            goto final_calc;
        }
        t3 = t1 * 2;
    }
    
    // Another basic block
    t4 = t3 + 5;
    return t4;

final_calc:
    // Safe instruction after label
    // Uses t4 which isn't in the jump conditions
    t4 = (t4 << 2) | 1;
    
    return t4;
}

// Test function 5: SPARC with multiple temporaries
__attribute__((target("arch=sparc")))
int test_sparc_multi_temp(int base) {
    // Create many independent temporaries
    int a = barrier(base);
    int b = barrier(base + 1);
    int c = barrier(base + 2);
    int d = barrier(base + 3);
    int e = barrier(base + 4);
    int f = 0;
    
    // Complex but reducible condition
    if ((a ^ b) > (c & d) || e > 50) {
        goto do_work;
    }
    
    f = a + b + c;
    return f;

do_work:
    // Safe: uses variables not in the exact condition
    // f and d are safe to use here
    f = d * 3 - 2;
    
    return f;
}

int main() {
    int checksum = 0;
    
    // Test with various inputs to explore different paths
    for (int i = 0; i < 10; i++) {
        checksum += test_mips_simple_jump(i, i * 2);
        checksum += test_sparc_jump_with_loop(i, i + 5);
        
        volatile int v = i * 10;
        checksum += test_generic_volatile_jump(&v);
        
        checksum += test_mips_complex_cfg(i, 10 - i);
        checksum += test_sparc_multi_temp(i);
        
        // Update global to prevent over-optimization
        global_acc += checksum;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_acc);
    
    return 0;
}
