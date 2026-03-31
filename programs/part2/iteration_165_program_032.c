#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int function_with_sequential_loops(int seed) {
    volatile int limit = 16; // Volatile to prevent constant propagation
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    // First loop - will share some basic blocks with second loop
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            acc1 += i * 2;
            continue;
        }
        acc1 += i;
        if (i == limit - 1) {
            acc1 *= 2; // Extra block at loop end
        }
    }
    
    // Second loop at same nesting level, partially overlapping blocks
    // Shares the loop exit/continuation logic with first loop
    int j = 0;
    while (j < limit) {
        switch (j % 4) {
            case 0: acc2 += j * 3; break;
            case 1: acc2 += j * 2; break;
            case 2: acc2 += j; break;
            default: acc2 += 1;
        }
        j++;
        if (j == limit / 2) {
            acc2 += 100; // Extra conditional block
        }
    }
    
    // Third loop with different structure but overlapping exit block
    do {
        acc3 += acc1 + acc2 + limit;
        limit--; // Modifying volatile affects next iteration
    } while (limit > 8);
    
    return acc1 + acc2 + acc3;
}

__attribute__((noinline))
int function_with_nested_loops(int base) {
    int outer_acc = 0;
    volatile int outer_limit = 12;
    
    // Outer loop - blocks will be superset of inner loops
    for (int x = 0; x < outer_limit; ++x) {
        int inner_acc = 0;
        
        // First inner loop - strict subset of outer loop blocks
        for (int y = 0; y < 8; ++y) {
            inner_acc += x * y;
            if (y == 4) {
                inner_acc += base; // Conditional block
            }
        }
        
        // Second inner loop at same nesting level as first inner
        // Partially overlaps with first inner loop's blocks
        int z = x;
        while (z > 0) {
            inner_acc -= z;
            z--;
            if (z % 2 == 0) {
                continue; // Creates additional basic block
            }
        }
        
        outer_acc += inner_acc;
        
        // Early exit creates additional block in outer loop
        if (outer_acc > 1000) {
            break;
        }
    }
    
    return outer_acc;
}

__attribute__((noinline))
int function_with_complex_control_flow(int init) {
    int result = init;
    volatile int a = 7, b = 13;
    
    // Loop with multiple exits and continues
    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < b; j++) {
                result += i * j;
                if (j == 5) {
                    continue; // Inner loop continue
                }
                if (i + j > 15) {
                    break; // Inner loop break
                }
            }
            continue; // Outer loop continue
        }
        
        // Another loop at same level as the inner for-loop
        int k = 0;
        do {
            result -= k;
            k++;
        } while (k < 3);
        
        if (i == a - 1) {
            result *= 2; // Final iteration block
        }
    }
    
    // Post-loop block shared with other functions
    return result;
}

int main() {
    int checksum = 0;
    volatile int seed = 42; // Volatile to prevent constant folding
    
    // Execute all functions to force loop analysis
    checksum += function_with_sequential_loops(seed);
    checksum += function_with_nested_loops(seed);
    checksum += function_with_complex_control_flow(seed);
    
    // Additional direct loops in main to create more opportunities
    volatile int iter = 32; // Hardware loop candidate count
    int main_acc = 0;
    
    // Countable loop with simple induction variable
    for (int i = 0; i < iter; i++) {
        main_acc += i * i;
        if (i == iter / 2) {
            main_acc += 777; // Mid-loop conditional
        }
    }
    
    // Another loop sharing some blocks with previous loop
    int j = iter;
    while (j > 0) {
        main_acc -= j;
        j -= 2;
    }
    
    checksum += main_acc;
    
    // Final observable output
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
