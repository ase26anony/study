#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_modifier = 0;
static volatile int static_counter = 0;

// Function with side effects for condition testing
__attribute__((noinline)) 
int side_effect_func(void) {
    return global_modifier++;
}

// Test function 1: Simple modification in then block
__attribute__((noinline))
void test_simple_modification(volatile int a, volatile int b) {
    // Loop context to create interesting block structure
    for (int i = 0; i < 10; i++) {
        // Condition variable 'a' modified in then block
        if (a > b) {
            // This assignment modifies 'a' which is used in the condition
            // Should be in the then block header analyzed by the for loop
            a = b + i;
            // Additional instructions to ensure non-empty block
            b = a * 2;
            global_modifier += a;
        }
    }
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(volatile int x, volatile int y, volatile int z) {
    int iterations = 5;
    
    while (iterations-- > 0) {
        // Compound condition using multiple variables
        // x is modified in the then block
        if (x != 0 && y < z && global_modifier > 0) {
            // Multiple modifications in the then block header
            x = y + z;  // Modifies x used in condition (x != 0)
            y = x * 2;  // Modifies y used in condition (y < z)
            // Ensure these are not optimized away
            asm volatile("" : : "r"(x), "r"(y) : "memory");
        }
        
        // Additional code to prevent block merging
        z += iterations;
    }
}

// Test function 3: Function call in condition with modification
__attribute__((noinline))
void test_func_call_condition(volatile int m, volatile int n) {
    for (int j = 0; j < 8; j++) {
        // Function call in condition, then modifies global state
        if (side_effect_func() > m && n < 100) {
            // Modify global_modifier which side_effect_func() reads
            global_modifier = m + n;
            // Also modify n which is used in condition
            n = global_modifier % 50;
            // Additional non-trivial operations
            m = (m * 3) / 2;
        }
        
        // Prevent loop unrolling from simplifying too much
        if (j % 2 == 0) {
            m += j;
        }
    }
}

// Test function 4: Static variable modification
__attribute__((noinline))
void test_static_modification(volatile int limit) {
    // Reset static counter
    static_counter = 0;
    
    for (int k = 0; k < 15; k++) {
        // Condition depends on static variable
        if (static_counter < limit && k > 3) {
            // Modify static_counter used in condition
            static_counter += k;
            // Also modify limit parameter
            limit = static_counter / 2;
            // Complex expression to prevent simplification
            global_modifier ^= (static_counter << 2);
        }
        
        // Additional control flow to shape basic blocks
        switch (k % 3) {
            case 0: static_counter--; break;
            case 1: limit++; break;
            default: break;
        }
    }
}

// Test function 5: Nested conditions with modifications
__attribute__((noinline))
void test_nested_modification(volatile int p, volatile int q, volatile int r) {
    int outer = 6;
    
    while (outer--) {
        // Outer condition
        if (p > q) {
            // Inner condition with modification
            if (q < r || p == 0) {
                // Modify p which is used in outer condition
                p = q * r;
                // Modify q used in both conditions
                q = p % 17;
                // Memory barrier to prevent reordering
                asm volatile("" : : : "memory");
            }
            // Additional code in then block
            r += p;
        }
        
        // Loop variant to prevent dead code elimination
        p += outer;
        q -= outer;
    }
}

int main(int argc, char *argv[]) {
    // Use command line arguments for runtime variability
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    // Initialize volatile variables
    volatile int a = seed + 10;
    volatile int b = seed - 5;
    volatile int c = seed * 2;
    volatile int x = seed % 20;
    volatile int y = seed + 3;
    volatile int z = seed - 7;
    volatile int m = seed * 3;
    volatile int n = seed / 2;
    volatile int p = seed + 15;
    volatile int q = seed - 12;
    volatile int r = seed % 15;
    volatile int limit = seed + 8;
    
    // Call test functions with different patterns
    test_simple_modification(a, b);
    test_compound_condition(x, y, z);
    test_func_call_condition(m, n);
    test_static_modification(limit);
    test_nested_modification(p, q, r);
    
    // Compute and print result to ensure side effects
    int result = global_modifier + static_counter + a + b + x + y + z + m + n + p + q + r + limit;
    printf("Result: %d (global_modifier=%d, static_counter=%d)\n", 
           result, global_modifier, static_counter);
    
    // Read from stdin to prevent constant propagation
    if (argc > 2) {
        int dummy;
        printf("Enter a number: ");
        if (scanf("%d", &dummy) == 1) {
            result += dummy;
        }
    }
    
    return result % 256;
}
