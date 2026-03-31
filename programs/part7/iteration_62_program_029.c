#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to isolate loop patterns
__attribute__((noinline)) int test_for_loop_signed(int n) {
    volatile int dummy1, dummy2, dummy3, dummy4, dummy5;
    int sum = 0;
    
    // Create register pressure
    dummy1 = n * 2;
    dummy2 = n + 1;
    dummy3 = n - 1;
    dummy4 = n * n;
    dummy5 = n / 2;
    
    // Target pattern: for (int i = N; i != 0; i--)
    for (int i = n; i != 0; i--) {
        // Simple control flow inside loop
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 5 == 0) {
            sum -= i;
        } else {
            sum += 1;
        }
        
        // Additional register pressure inside loop
        volatile int temp = i * 3;
        (void)temp;
    }
    
    // Post-loop use of counter (i is out of scope, use n)
    return sum + (n % 10);
}

__attribute__((noinline)) int test_while_loop_unsigned(unsigned int n) {
    volatile int r1, r2, r3, r4, r5, r6, r7, r8;
    int sum = 0;
    unsigned int count = n;
    
    // Heavy register pressure
    r1 = count * 2;
    r2 = count + 100;
    r3 = count ^ 0xFF;
    r4 = r1 + r2;
    r5 = r3 * r4;
    r6 = r5 - count;
    r7 = r6 / 3;
    r8 = r7 & 0x7F;
    
    // Target pattern: while (count-- > 0)
    while (count-- > 0) {
        // Nested control flow
        if (count % 4 == 0) {
            sum += (int)count;
            if (sum > 1000) {
                sum = 1000;
            }
        } else {
            sum -= 2;
        }
        
        // More register pressure
        volatile int t1 = sum * 2;
        volatile int t2 = t1 + (int)count;
        (void)t1; (void)t2;
    }
    
    // Use final counter value
    return sum + (int)(n % 20);
}

__attribute__((noinline)) int test_do_while_loop(int n) {
    volatile long a1, a2, a3, a4, a5;
    int sum = 0;
    int iterations = n;
    
    // Register pressure with different types
    a1 = n * 100L;
    a2 = n + 500L;
    a3 = a1 - a2;
    a4 = a3 * 2L;
    a5 = a4 / 3L;
    
    // Target pattern: do { ... } while (--iterations);
    if (iterations > 0) {
        do {
            // Complex control flow
            switch (iterations % 4) {
                case 0: sum += 5; break;
                case 1: sum += iterations; break;
                case 2: sum -= 3; break;
                case 3: sum *= 2; if (sum > 10000) sum = 10000; break;
            }
            
            // Conditional break based on internal state
            if (sum < -1000) {
                break;
            }
            
            // Register pressure
            volatile long temp = sum * iterations;
            (void)temp;
        } while (--iterations);
    }
    
    // Post-loop use
    return sum + iterations;
}

__attribute__((noinline)) int test_manual_decrement(int n) {
    volatile double d1, d2, d3;
    volatile int v1, v2, v3, v4, v5;
    int sum = 0;
    int counter = n;
    
    // Mixed-type register pressure
    d1 = n * 1.5;
    d2 = n / 2.0;
    d3 = d1 + d2;
    v1 = (int)d1;
    v2 = (int)d2;
    v3 = v1 * v2;
    v4 = v3 + n;
    v5 = v4 % 256;
    
    // Target pattern: manual decrement with separate comparison
    while (1) {
        // Loop body with control flow
        if (counter & 1) {
            sum += counter * 3;
        } else {
            sum += counter;
        }
        
        // Manual decrement
        counter -= 1;  // Should become (plus reg -1)
        
        // Separate zero comparison
        if (counter == 0) {
            break;
        }
        
        // Additional complexity
        if (sum > 5000) {
            sum = 5000;
        }
    }
    
    return sum + counter;
}

__attribute__((noinline)) int test_nested_control_flow(unsigned int n) {
    volatile int pressure[10];
    int sum = 0;
    
    // Array-based register pressure
    for (int j = 0; j < 10; j++) {
        pressure[j] = n * j;
    }
    
    unsigned int k = n;
    
    // Target pattern with complex body
    for (; k != 0; k--) {
        // Multiple conditionals
        if (k < n / 2) {
            sum += pressure[k % 10];
        } else {
            sum -= (int)k;
        }
        
        // Inner conditional with early continue
        if (k % 7 == 0) {
            continue;
        }
        
        // Another conditional
        if (sum < 0) {
            sum = 0;
        } else if (sum > 10000) {
            sum = 10000;
        }
        
        // Use pressure array
        pressure[k % 10] = sum % 100;
    }
    
    return sum + pressure[0];
}

__attribute__((noinline)) int test_mixed_operations(int n) {
    volatile int p1, p2, p3, p4;
    int accumulator = 0;
    int idx = n;
    
    // Create register pressure
    p1 = n * 3;
    p2 = n + 7;
    p3 = p1 ^ p2;
    p4 = p3 << 2;
    
    // Multiple decrement patterns in sequence
    while (idx > 0) {
        accumulator += idx;
        idx--;
        
        // Conditional that might affect RTL generation
        if (accumulator & 0x1) {
            accumulator += p4;
        }
    }
    
    // Second loop with different pattern
    int count2 = n / 2;
    for (; count2 != 0; --count2) {
        accumulator -= count2;
        
        // Use volatile to prevent optimization
        volatile int temp = accumulator;
        if (temp < 0) {
            accumulator = -accumulator;
        }
    }
    
    return accumulator + idx + count2;
}

int main(int argc, char *argv[]) {
    // Use command line argument or default for loop iterations
    int base_iterations = (argc > 1) ? atoi(argv[1]) : 50;
    
    if (base_iterations <= 0) {
        base_iterations = 50;
    }
    
    printf("Testing doloop patterns with base_iterations = %d\n", base_iterations);
    
    // Call all test functions with different iteration counts
    int total = 0;
    
    total += test_for_loop_signed(base_iterations);
    total += test_while_loop_unsigned((unsigned int)base_iterations + 10);
    total += test_do_while_loop(base_iterations / 2 + 5);
    total += test_manual_decrement(base_iterations + 7);
    total += test_nested_control_flow((unsigned int)base_iterations + 3);
    total += test_mixed_operations(base_iterations);
    
    printf("Total result: %d\n", total);
    
    // Ensure result is used
    if (total == 0) {
        printf("All loops returned zero (unlikely)\n");
    }
    
    return total != 0 ? 0 : 1;
}
