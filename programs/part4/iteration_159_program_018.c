#include <stdio.h>
#include <stdlib.h>

volatile int g_volatile = 0;
static int g_accumulator = 0;

// Barrier function to prevent optimization
__attribute__((noinline)) int barrier(int x) {
    return x ^ 0x55AA;
}

// Test function for MIPS target
__attribute__((target("arch=mips32")))
int test_mips_delay_slot(int a, int b) {
    // Initialize temporaries - these should be in registers
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = a ^ b;
    int temp4 = 0;
    
    // Create a non-trivial condition using input args
    if (a > b) {
        // This should generate a simple jump to label
        goto target_label;
    }
    
    // Some other code to create basic blocks
    temp1 = barrier(temp1);
    temp2 = barrier(temp2);
    
    // This is the target label - the next instruction should be safe to move
target_label:
    // Safe, non-trapping operation using temporaries not used in jump condition
    // Uses temp3 and temp4 which aren't in the condition (a > b)
    temp4 = temp3 & 0xFF;
    
    // Use the result to prevent dead code elimination
    return temp4 + temp1 + temp2;
}

// Test function for SPARC target
__attribute__((target("arch=sparc")))
int test_sparc_delay_slot(int x, int y) {
    // Multiple temporaries to work with
    int t1 = x + y;
    int t2 = x - y;
    int t3 = x * 2;
    int t4 = y * 3;
    int result = 0;
    
    // Volatile read to create non-constant condition
    int cond = g_volatile;
    
    if (cond > 0) {
        // Simple jump to label
        goto sparc_target;
    }
    
    // Other operations
    t1 = barrier(t1);
    t2 = barrier(t2);
    
sparc_target:
    // Safe instruction: logical operation on temporaries
    // t3 and t4 aren't used in the condition (cond > 0)
    result = t3 | t4;
    
    return result + t1 + t2;
}

// Generic test without architecture-specific attributes
// (rely on -march flag during compilation)
int test_generic_delay_slot(int p, int q, int r) {
    // Create multiple independent temporaries
    int local1 = p + q;
    int local2 = q + r;
    int local3 = p * r;
    int local4 = 0;
    
    // Complex enough condition to not be optimized away
    if ((p ^ q) > (q ^ r)) {
        goto generic_target;
    }
    
    // Some intervening code
    local1 = barrier(local1);
    local2 = barrier(local2);
    
generic_target:
    // Safe arithmetic operation
    // local3 isn't used in the jump condition
    local4 = local3 + 7;
    
    return local4 + local1 + local2;
}

// Another variant with different operations
int test_variant2(int a, int b, int c) {
    int x = a + b;
    int y = b + c;
    int z = c + a;
    int w = 0;
    
    // Different condition pattern
    if (a % 2 == 0) {
        goto variant_target;
    }
    
    x = barrier(x);
    y = barrier(y);
    
variant_target:
    // Different safe operation
    w = z << 2;
    
    return w + x + y;
}

// Test with more complex control flow around the target pattern
int test_with_loop(int n, int seed) {
    int sum = seed;
    int temp1 = seed * 2;
    int temp2 = seed / 3;  // Note: division is safe with non-zero seed
    
    for (int i = 0; i < n; i++) {
        sum += i;
        
        // Put the target pattern inside the loop
        if (sum > 100) {
            goto loop_target;
        }
        
        temp1 = barrier(temp1);
        
    loop_target:
        // Safe operation - temp2 isn't used in the condition
        temp2 = temp2 ^ i;
        
        // Continue loop
        sum += temp2;
    }
    
    return sum + temp1;
}

int main() {
    int result = 0;
    
    // Initialize volatile
    g_volatile = 42;
    
    // Call all test functions with different inputs
    result ^= test_mips_delay_slot(10, 5);
    result ^= test_mips_delay_slot(5, 10);
    
    result ^= test_sparc_delay_slot(7, 3);
    result ^= test_sparc_delay_slot(3, 7);
    
    result ^= test_generic_delay_slot(1, 2, 3);
    result ^= test_generic_delay_slot(3, 2, 1);
    
    result ^= test_variant2(4, 5, 6);
    result ^= test_variant2(6, 5, 4);
    
    result ^= test_with_loop(5, 10);
    result ^= test_with_loop(3, 20);
    
    // Add to global accumulator
    g_accumulator += result;
    
    printf("Result checksum: %d\n", result);
    printf("Global accumulator: %d\n", g_accumulator);
    
    return 0;
}
