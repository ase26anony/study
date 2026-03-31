#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to isolate loops from interprocedural optimizations
__attribute__((noinline)) int test_for_loop_signed(int iterations) {
    volatile int dummy1, dummy2, dummy3, dummy4, dummy5;
    int result = 0;
    
    // Create register pressure before the loop
    dummy1 = iterations * 2;
    dummy2 = iterations + 12345;
    dummy3 = dummy1 ^ dummy2;
    dummy4 = dummy3 << 3;
    dummy5 = dummy4 | 0xFF;
    
    // Critical pattern: for loop with decrement and zero comparison
    for (int i = iterations; i != 0; i--) {
        // Simple control flow inside loop
        if (i % 3 == 0) {
            result += i * 2;
        } else if (i % 5 == 0) {
            result -= i;
        } else {
            result += 1;
        }
        
        // Additional register pressure inside loop
        volatile int temp = dummy5;
        dummy5 = temp + i;
    }
    
    // Post-loop use of counter (i is out of scope here, use iterations)
    result += iterations;
    
    // More register pressure
    dummy1 = result ^ dummy2;
    dummy2 = dummy3 + dummy4;
    
    return result;
}

__attribute__((noinline)) int test_while_loop_unsigned(unsigned int n) {
    volatile int reg_pressure[10];
    int sum = 0;
    
    // Create register pressure
    for (int j = 0; j < 10; j++) {
        reg_pressure[j] = n + j * 17;
    }
    
    unsigned int counter = n;
    // Critical pattern: while loop with post-decrement and zero comparison
    while (counter-- > 0) {
        // Control flow with conditional break
        if (sum > 1000) {
            sum /= 2;
        }
        
        // Use counter in computation
        sum += (counter % 7) + 1;
        
        // Register pressure
        volatile int temp = reg_pressure[counter % 10];
        reg_pressure[counter % 10] = temp + sum;
    }
    
    // Post-loop use of counter
    sum += (int)counter;
    
    return sum;
}

__attribute__((noinline)) int test_do_while_loop(int count) {
    volatile long pressure1, pressure2, pressure3, pressure4;
    int accumulator = 0;
    
    // Register pressure setup
    pressure1 = count * 3L;
    pressure2 = count * 7L;
    pressure3 = pressure1 ^ pressure2;
    pressure4 = pressure3 << 2;
    
    int loop_counter = count;
    // Critical pattern: do-while with pre-decrement
    do {
        // Internal control flow
        if (loop_counter & 1) {
            accumulator += loop_counter;
        } else {
            accumulator -= loop_counter / 2;
        }
        
        // Potential early exit (but rare)
        if (accumulator < -10000) {
            accumulator = 0;
        }
        
        // Register pressure
        volatile long temp = pressure4;
        pressure4 = temp + loop_counter;
    } while (--loop_counter);
    
    // Post-loop use
    accumulator += loop_counter * 2;
    
    return accumulator;
}

__attribute__((noinline)) int test_modified_inside_body(unsigned int start) {
    volatile int array[20];
    int total = 0;
    
    // Initialize array for register pressure
    for (int k = 0; k < 20; k++) {
        array[k] = start + k * 11;
    }
    
    unsigned int idx = start;
    // Critical pattern: counter modified inside body with separate comparison
    while (idx != 0) {
        total += array[idx % 20];
        
        // Control flow
        if (total % 13 == 0) {
            total >>= 1;
        }
        
        // Decrement inside body (not in loop condition)
        idx -= 1;
        
        // Register pressure
        volatile int temp = array[0];
        array[0] = temp + idx;
    }
    
    // Post-loop use
    total += (int)idx;
    
    return total;
}

__attribute__((noinline)) int test_nested_pressure(int base) {
    volatile int p1, p2, p3, p4, p5, p6, p7, p8;
    int value = 0;
    
    // Heavy register pressure
    p1 = base * 2;
    p2 = base + 100;
    p3 = p1 ^ p2;
    p4 = p3 * 3;
    p5 = p4 - p1;
    p6 = p5 >> 2;
    p7 = p6 | 0xABCD;
    p8 = p7 & 0xFFFF;
    
    // Multiple decrementing loops
    for (int outer = 3; outer != 0; outer--) {
        int inner = base / (outer + 1);
        
        // Inner decrementing loop
        while (inner-- > 0) {
            value += (outer * inner);
            
            // Control flow
            if (value > 5000) {
                value = 5000;
            }
            
            // Use pressure variables
            volatile int mix = p1 + p8;
            p8 = mix ^ value;
        }
        
        // More pressure between loops
        p1 = p2 + p3;
        p2 = p4 - p5;
    }
    
    // Post-loop use
    value += p1 + p2 + p3;
    
    return value;
}

int main() {
    int total_result = 0;
    
    // Call all test functions with different iteration counts
    total_result += test_for_loop_signed(50);
    total_result += test_while_loop_unsigned(40);
    total_result += test_do_while_loop(30);
    total_result += test_modified_inside_body(35);
    total_result += test_nested_pressure(25);
    
    // Add some variation
    total_result += test_for_loop_signed(17);
    total_result += test_while_loop_unsigned(23);
    
    printf("Total result: %d\n", total_result);
    
    // Ensure result is used
    if (total_result > 1000) {
        printf("Result exceeds threshold\n");
    }
    
    return 0;
}
