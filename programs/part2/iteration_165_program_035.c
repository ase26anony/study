#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // Volatile to prevent constant propagation
    int sum = 0;
    
    // Loop A: Sequential loop that will share blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        // Shared basic block pattern with Loop B
        if (sum > 100) {
            sum -= 50; // Branch creates additional blocks
        }
    }
    
    // Loop B: Sequential loop at same nesting level, partially overlapping blocks
    for (int j = 0; j < limit * 2; ++j) {
        // Same conditional structure as Loop A but different condition
        if (j % 4 == 0) {
            sum += j * 3;
        } else {
            sum += j;
        }
        // Shared basic block with Loop A
        if (sum > 100) {
            sum -= 50;
        }
        // Additional unique block for partial overlap
        if (j == limit) {
            sum += 1000;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int nested_loops_2(int base) {
    int total = base;
    volatile int outer_limit = 8;
    volatile int inner_limit = 4;
    
    // Outer Loop C: Contains inner loops as subsets
    for (int x = 0; x < outer_limit; ++x) {
        total += x * 10;
        
        // Inner Loop D: Strict subset of Outer Loop C's blocks
        for (int y = 0; y < inner_limit; ++y) {
            total += y * 5;
            // Conditional creates blocks that are subset of outer loop
            if (y % 2 == 0) {
                total += 1;
            } else {
                total -= 1;
            }
        }
        
        // Another Inner Loop E: Also subset but with different structure
        for (int z = 0; z < inner_limit + 2; ++z) {
            total += z * 3;
            // Early exit creates unique block pattern
            if (total > 1000) {
                break;
            }
        }
        
        // Switch statement to create complex block patterns
        switch (x % 3) {
            case 0: total += 100; break;
            case 1: total += 200; break;
            case 2: total += 300; break;
        }
    }
    
    return total;
}

__attribute__((noinline))
int overlapping_control_flow_3(int init) {
    int result = init;
    volatile int count = 12;
    
    // Loop F: do-while with complex exit conditions
    int k = 0;
    do {
        result += k * 7;
        
        // Nested while loop with continue
        int m = 0;
        while (m < 5) {
            if (m == 2) {
                m++;
                continue; // Creates back edge block
            }
            result += m;
            m++;
        }
        
        // Conditional break in middle of loop
        if (k == count / 2) {
            result += 500;
            // Not breaking - just creating branch
        }
        
        k++;
    } while (k < count);
    
    // Loop G: Sequential loop sharing some blocks with Loop F
    for (int n = 0; n < count * 2; ++n) {
        // Shared arithmetic pattern
        result += n * 7;
        
        // Different conditional structure
        if (n % 5 == 0) {
            result += n * 2;
        } else if (n % 5 == 1) {
            result += n * 3;
        } else {
            // Empty else creates distinct block
        }
    }
    
    return result;
}

__attribute__((noinline))
int hardware_loop_candidate_4(int start) {
    int acc = start;
    
    // Perfect hardware loop candidate: fixed bounds, simple increment
    for (int i = 0; i < 32; ++i) {
        acc += i * i; // Non-trivial arithmetic
    }
    
    // Another candidate with different bound
    for (int j = 0; j < 64; ++j) {
        acc += j * 3;
        // Small conditional that doesn't break loop structure
        if (j % 8 == 0) {
            acc += 1;
        }
    }
    
    return acc;
}

int main() {
    // Initialize with volatile to prevent compile-time computation
    volatile int seed = 42;
    int result1, result2, result3, result4;
    
    // Execute all loop patterns to ensure analysis
    result1 = complex_loops_1(seed);
    result2 = nested_loops_2(result1);
    result3 = overlapping_control_flow_3(result2);
    result4 = hardware_loop_candidate_4(result3);
    
    // Create data dependency chain and observable output
    int final_result = result1 + result2 * 2 + result3 * 3 + result4 * 4;
    
    // Use result to prevent dead code elimination
    printf("Final checksum: %d\n", final_result);
    
    // Also use results individually
    printf("Results: %d, %d, %d, %d\n", result1, result2, result3, result4);
    
    return final_result > 0 ? 0 : 1;
}
