#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structure
__attribute__((noinline)) int test_for_loop_signed(int n) {
    volatile int dummy1 = 0, dummy2 = 0, dummy3 = 0;
    int sum = 0;
    
    // Create register pressure before the loop
    int r1 = n * 2, r2 = n * 3, r3 = n * 4;
    int r4 = n * 5, r5 = n * 6, r6 = n * 7;
    int r7 = n * 8, r8 = n * 9, r9 = n * 10;
    
    // Critical pattern: for loop with decrement and zero comparison
    for (int i = n; i != 0; i--) {
        // Simple control flow inside loop
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 5 == 0) {
            sum -= i;
        } else {
            sum += 1;
        }
        
        // Use some register pressure variables
        dummy1 = r1 + r2;
        dummy2 = r3 + r4;
    }
    
    // Post-loop use of counter (i is out of scope, use n)
    sum += n;
    
    // Consume register pressure variables
    return sum + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + dummy1 + dummy2;
}

__attribute__((noinline)) int test_for_loop_unsigned(unsigned int n) {
    volatile int dummy1 = 0, dummy2 = 0;
    int sum = 0;
    
    // Register pressure
    unsigned int ur1 = n * 2, ur2 = n * 3, ur3 = n * 4;
    unsigned int ur4 = n * 5, ur5 = n * 6;
    
    // Different pattern: unsigned counter
    for (unsigned int i = n; i > 0; i--) {
        // Conditional break to add control flow
        if (i == n / 2) {
            sum += 100;
        }
        
        sum += (int)i;
        dummy1 = ur1 + ur2;
    }
    
    // Post-loop use
    sum += (int)n;
    
    return sum + ur1 + ur2 + ur3 + ur4 + ur5 + dummy1 + dummy2;
}

__attribute__((noinline)) int test_while_loop_decrement(int n) {
    volatile int dummy1 = 0, dummy2 = 0, dummy3 = 0;
    int sum = 0;
    
    // Heavy register pressure
    int r1 = n, r2 = n*2, r3 = n*3, r4 = n*4, r5 = n*5;
    int r6 = n*6, r7 = n*7, r8 = n*8, r9 = n*9, r10 = n*10;
    
    int count = n;
    // While loop with post-decrement and zero comparison
    while (count-- > 0) {
        // Nested if-else for control flow
        if (count % 4 == 0) {
            sum += count * 3;
        } else {
            sum += 2;
        }
        
        // Use register pressure vars
        dummy1 = r1 + r2 + r3;
        dummy2 = r4 + r5 + r6;
        dummy3 = r7 + r8;
    }
    
    // Post-loop use of counter (count is -1 after loop)
    sum += (count + 1); // Should be 0
    
    return sum + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

__attribute__((noinline)) int test_do_while_loop(int n) {
    volatile int dummy1 = 0;
    int sum = 0;
    
    // Register pressure
    int r1 = n * 11, r2 = n * 12, r3 = n * 13;
    int r4 = n * 14, r5 = n * 15;
    
    int counter = n;
    // Do-while with pre-decrement
    do {
        if (counter % 7 == 0) {
            sum += 50;
        }
        sum += counter;
        dummy1 = r1 + r2;
    } while (--counter);
    
    // Post-loop use
    sum += counter; // counter is 0 after loop
    
    return sum + r1 + r2 + r3 + r4 + r5 + dummy1;
}

__attribute__((noinline)) int test_manual_decrement_in_body(int n) {
    volatile int dummy1 = 0, dummy2 = 0;
    int sum = 0;
    
    // Register pressure
    int r1 = n * 16, r2 = n * 17, r3 = n * 18;
    int r4 = n * 19, r5 = n * 20, r6 = n * 21;
    
    int i = n;
    // Manual decrement in body with explicit comparison
    while (i != 0) {
        sum += i * i;
        
        // Decrement in body - still should match pattern
        i -= 1;  // Equivalent to i = i - 1
        
        // Control flow
        if (i < n / 3) {
            sum += 10;
        }
        
        dummy1 = r1 + r3 + r5;
        dummy2 = r2 + r4 + r6;
    }
    
    // Post-loop use
    sum += i; // i is 0
    
    return sum + r1 + r2 + r3 + r4 + r5 + r6 + dummy1 + dummy2;
}

__attribute__((noinline)) int test_nested_counter_usage(int n) {
    volatile int dummy1 = 0;
    int sum = 0;
    
    // Multiple counters to increase register pressure
    int c1 = n, c2 = n * 2, c3 = n * 3;
    int r1 = n * 22, r2 = n * 23, r3 = n * 24;
    
    // Primary loop with decrementing counter
    for (int i = n; i > 0; i--) {
        // Use other counters in computation
        sum += c1 + c2;
        
        // Modify counters
        c1--;
        c2 = c2 - 1;
        
        // Main loop body
        if (i % 2 == 0) {
            sum += i * 5;
        }
        
        dummy1 = r1 + r2 + r3 + i;
    }
    
    // Post-loop use of all counters
    sum += c1 + c2 + c3;
    
    return sum + r1 + r2 + r3 + dummy1;
}

int main() {
    int total = 0;
    int base_iterations = 50; // Moderate iteration count
    
    // Call all test functions with different iteration counts
    total += test_for_loop_signed(base_iterations);
    total += test_for_loop_unsigned(base_iterations + 10);
    total += test_while_loop_decrement(base_iterations + 5);
    total += test_do_while_loop(base_iterations - 5);
    total += test_manual_decrement_in_body(base_iterations + 7);
    total += test_nested_counter_usage(base_iterations + 3);
    
    printf("Total result: %d\n", total);
    
    // Prevent dead code elimination
    volatile int prevent_opt = total;
    
    return (prevent_opt > 0) ? 0 : 1;
}
