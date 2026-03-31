/* loop-doloop.cc coverage test
 * Specifically targets lines 136-150 of loop-doloop.cc
 * Compile with: g++ -O2 -fdump-rtl-loop2_doloop -S this_file.cc
 */

#include <cstdio>

// Global volatile to prevent dead code elimination
volatile int global_side_effect = 0;

// Simple non-inlineable function with side effect
__attribute__((noinline)) void do_side_effect(int val) {
    global_side_effect += val;
}

int main() {
    // Initialize volatile counters to prevent constant propagation
    volatile int counter1 = 100;
    volatile int counter2 = 50;
    volatile int counter3 = 30;
    volatile int counter4 = 20;
    volatile int counter5 = 15;
    
    int result = 0;
    
    // 1. Basic for loop with decrementing counter compared to zero
    // Should generate: for (reg; reg != 0; reg--)
    for (volatile int i = counter1; i != 0; i--) {
        // Simple side effect to prevent loop removal
        asm volatile("" : : : "memory");
        result += 1;
    }
    do_side_effect(result);
    
    // 2. While loop with post-decrement in condition
    // Should generate: while (counter2-- != 0)
    volatile int temp2 = counter2;
    while (temp2-- != 0) {
        asm volatile("" : : : "memory");
        result += 2;
    }
    do_side_effect(result);
    
    // 3. Nested loops with independent decrementing counters
    // Both loops should generate the target pattern
    volatile int outer = counter3;
    while (outer-- != 0) {
        volatile int inner = counter4;
        for (; inner != 0; inner--) {
            asm volatile("" : : : "memory");
            result += 3;
        }
        do_side_effect(result);
    }
    
    // 4. Mixed loop constructs: do-while with pre-decrement
    // inside a for loop with decrementing counter
    volatile int k = counter5;
    for (volatile int m = 10; m != 0; m--) {
        do {
            asm volatile("" : : : "memory");
            result += 4;
        } while (--k != 0);
        // Reset k for each outer iteration
        k = 5;
    }
    do_side_effect(result);
    
    // 5. Another variation: for loop with post-decrement in condition
    volatile int n = 25;
    for (; n--; ) {
        asm volatile("" : : : "memory");
        result += 5;
    }
    do_side_effect(result);
    
    // 6. Complex nested case with multiple decrementing loops
    volatile int a = 8, b = 6, c = 4;
    
    // Outer loop
    while (a-- != 0) {
        // Middle loop
        volatile int b_temp = b;
        for (; b_temp != 0; b_temp--) {
            // Inner loop
            volatile int c_temp = c;
            do {
                asm volatile("" : : : "memory");
                result += 6;
            } while (--c_temp != 0);
            c = 4; // Reset for next iteration
        }
        b = 6; // Reset for next iteration
    }
    do_side_effect(result);
    
    // Return value depends on all loop executions
    return result + global_side_effect;
}
