#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
#define NOINLINE __attribute__((noinline))

// Global accumulators with data dependencies
static volatile int global_seed = 42;
static int global_result = 0;

// Function 1: Sequential loops with overlapping blocks
NOINLINE int sequential_loops_overlap(int start) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    volatile int limit = start + 16;
    
    // First loop - shares header block with second loop
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            acc1 += i * 2;
            continue;
        }
        acc1 += i;
        
        // Early exit creates additional basic block
        if (acc1 > 1000) break;
    }
    
    // Second loop - shares some blocks with first loop
    // This creates bitmap intersection but not complete subset
    for (int j = start; j < limit * 2; ++j) {
        switch (j % 4) {
            case 0: acc2 += j * 3; break;
            case 1: acc2 += j * 2; break;
            case 2: acc2 += j; break;
            default: acc2 += 1;
        }
        
        // Conditional continue creates separate block
        if (j % 5 == 0) continue;
        
        acc2 += global_seed;
    }
    
    // Third loop - completely different structure
    // Creates non-intersecting bitmap
    int k = 0;
    while (k < 32) {
        acc3 += k * k;
        if (k % 2 == 0) {
            acc3 += global_result;
        }
        k++;
    }
    
    return acc1 + acc2 + acc3;
}

// Function 2: Nested loops where inner is subset of outer
NOINLINE int nested_subset_loops(int base) {
    int outer_acc = 0, inner_acc = 0;
    volatile int outer_limit = base + 24;
    
    // Outer loop - contains inner loop as strict subset
    for (int i = 0; i < outer_limit; ++i) {
        outer_acc += i * 3;
        
        // Conditional that creates shared block
        if (i % 2 == 0) {
            // Inner loop - blocks are subset of outer's blocks
            for (int j = 0; j < 8; ++j) {
                inner_acc += j * i;
                
                // Early exit in inner loop
                if (inner_acc > 500) break;
                
                inner_acc += global_seed % 7;
            }
        } else {
            // Alternative path without inner loop
            outer_acc += global_result;
        }
        
        // Function call creates additional block
        if (i % 7 == 0) {
            outer_acc += rand() % 10;
        }
    }
    
    return outer_acc + inner_acc * 2;
}

// Function 3: Complex control flow with multiple exits
NOINLINE int complex_control_flow(int iterations) {
    int acc = 0;
    volatile int mod = iterations % 10 + 5;
    
    // Loop with multiple exit points
    for (int i = 0; i < iterations; ++i) {
        acc += i;
        
        // Multiple conditional branches
        if (i % mod == 0) {
            acc += 100;
            continue;
        }
        
        if (i % (mod + 3) == 0) {
            acc += 200;
            // Nested loop with different structure
            for (int k = 0; k < 4; ++k) {
                acc += k * 10;
                if (k == 2) break;
            }
        }
        
        // Early return path
        if (acc > 2000) {
            // Another loop in early exit
            for (int j = 0; j < 3; ++j) {
                acc += j * 50;
            }
            return acc;
        }
    }
    
    // Post-loop processing
    do {
        acc += 1;
    } while (acc % 100 != 0);
    
    return acc;
}

// Function 4: Mixed loop types at same level
NOINLINE int mixed_loop_types(int seed) {
    int sum = 0;
    volatile int count = seed + 8;
    
    // do-while loop
    int idx = 0;
    do {
        sum += idx * 5;
        idx++;
        
        // Nested for-loop
        for (int n = 0; n < 3; ++n) {
            sum += n * idx;
            if (n == 1) continue;
            sum += global_seed;
        }
    } while (idx < count);
    
    // while loop with complex condition
    int temp = sum;
    while (temp > 0 && idx < count * 2) {
        temp -= 10;
        idx++;
        
        // Switch inside loop
        switch (temp % 4) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            case 3: sum += 4; break;
        }
    }
    
    // Another for loop sharing some blocks
    for (int i = 0; i < count / 2; ++i) {
        sum += i * i;
        if (i % 2 == 0) {
            sum += temp;
        }
    }
    
    return sum;
}

// Main driver with data dependencies
int main() {
    int final_result = 0;
    
    // Initialize with volatile to prevent constant propagation
    volatile int init_val = global_seed;
    
    // Execute all loop patterns with data dependencies
    int r1 = sequential_loops_overlap(init_val);
    global_result = r1 % 100;  // Create dependency
    
    int r2 = nested_subset_loops(init_val + 5);
    global_result = (global_result + r2) % 200;
    
    int r3 = complex_control_flow(init_val + 10);
    global_result = (global_result + r3) % 300;
    
    int r4 = mixed_loop_types(init_val + 15);
    global_result = (global_result + r4) % 400;
    
    // Combine all results
    final_result = r1 + r2 * 2 + r3 * 3 + r4 * 4 + global_result;
    
    // Use result to prevent dead code elimination
    printf("Final checksum: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
