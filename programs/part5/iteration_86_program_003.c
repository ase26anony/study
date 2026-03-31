/* loop-doloop.cc coverage test program
 * Designed to trigger the specific RTL pattern in lines 136-150:
 * COMPARE against zero where first operand is (reg + -1)
 */

#include <stdio.h>

// Global variable to create data dependencies
static int global_result = 0;

// Simple side effect to prevent loop removal
static void side_effect(int value) {
    // Use inline asm to ensure side effect isn't optimized away
    asm volatile ("" : : "r"(value) : "memory");
}

int main() {
    // Initialize volatile counters to prevent constant propagation
    volatile int outer_counter = 100;
    volatile int inner_counter = 50;
    volatile int count1 = 75;
    volatile int count2 = 25;
    volatile int k_value = 30;
    
    int local_sum = 0;
    
    // 1. Basic for loop with decrementing counter compared to zero
    // Should generate: for (reg; reg != 0; reg--)
    for (volatile int i = outer_counter; i != 0; i--) {
        side_effect(i);
        local_sum += 1;
    }
    global_result += local_sum;
    
    // 2. While loop with post-decrement in condition
    // Pattern: while (count-- != 0) generates (reg + -1) compare with 0
    local_sum = 0;
    volatile int temp = count1;
    while (temp-- != 0) {
        side_effect(temp);
        local_sum += 2;
    }
    global_result += local_sum;
    
    // 3. Nested loops with independent decrementing counters
    // Both loops should generate the target pattern
    local_sum = 0;
    volatile int i = 10;
    while (i-- != 0) {
        volatile int j = 5;
        // Inner loop with post-decrement in condition
        while (j-- != 0) {
            side_effect(i + j);
            local_sum += 3;
        }
    }
    global_result += local_sum;
    
    // 4. Mixed loop constructs: do-while with pre-decrement inside for loop
    local_sum = 0;
    volatile int m = 8;
    for (; m; m--) {
        volatile int k = k_value;
        // do-while with pre-decrement comparison
        do {
            side_effect(k);
            local_sum += 4;
        } while (--k != 0);
    }
    global_result += local_sum;
    
    // 5. Another variation: for loop with empty increment
    local_sum = 0;
    volatile int n = count2;
    for (; n--; ) {
        side_effect(n);
        local_sum += 5;
    }
    global_result += local_sum;
    
    // 6. Complex nested structure with multiple patterns
    local_sum = 0;
    volatile int a = 6;
    volatile int b = 4;
    
    // Outer while with post-decrement
    while (a-- != 0) {
        // Inner for with decrement
        for (volatile int c = b; c != 0; c--) {
            side_effect(a + c);
            local_sum += 6;
        }
        
        // Another inner do-while
        volatile int d = 3;
        do {
            side_effect(d);
            local_sum += 7;
        } while (--d != 0);
    }
    global_result += local_sum;
    
    // Return sum to prevent dead code elimination
    return global_result % 256;
}
