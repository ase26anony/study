#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed, int *result) {
    volatile int bound1 = 16;  // Prevent constant propagation
    volatile int bound2 = 8;
    int acc1 = 0, acc2 = 0;
    
    // First loop - will share some blocks with second loop
    for (int i = 0; i < bound1; ++i) {
        if (i % 3 == 0) {
            acc1 += i * seed;
            continue;
        }
        if (i % 5 == 0) {
            acc1 += i * 2;
            break;  // Early exit creates interesting block structure
        }
        acc1 += i;
    }
    
    // Second loop at same nesting level, partially overlapping blocks
    for (int j = 0; j < bound2; ++j) {
        switch (j % 4) {
            case 0: acc2 += j * 3; break;
            case 1: acc2 += j * seed; break;
            case 2: acc2 += j * 2; continue;  // Skip to next iteration
            default: acc2 += j;
        }
        // Common block with first loop's structure
        if (j % 2 == 0) {
            acc2 += seed;
        }
    }
    
    *result = acc1 + acc2;
    return acc1 > acc2 ? 1 : 0;
}

__attribute__((noinline))
int nested_loops_pattern(int start, int *outer_acc, int *inner_acc) {
    volatile int outer_bound = 12;
    volatile int inner_bound = 6;
    
    *outer_acc = 0;
    *inner_acc = 0;
    
    // Outer loop - inner loop's blocks are subset of outer's
    for (int i = start; i < outer_bound; ++i) {
        *outer_acc += i * 2;
        
        // Inner loop - strict subset of outer loop's blocks
        for (int j = 0; j < inner_bound; ++j) {
            if (j % 2 == 0) {
                *inner_acc += j * i;
                continue;
            }
            *inner_acc += j + i;
        }
        
        // Conditional block inside outer but outside inner
        if (i % 3 == 0) {
            *outer_acc += 7;
            // Another inner loop with different structure
            for (int k = 0; k < 4; ++k) {
                *inner_acc -= k;
            }
        }
    }
    
    return *outer_acc - *inner_acc;
}

__attribute__((noinline))
void overlapping_loop_sequences(int init, int *checksum) {
    volatile int bounds[3] = {10, 15, 8};
    int temp[3] = {0};
    
    // Three sequential loops with overlapping block structures
    for (int seq = 0; seq < 3; ++seq) {
        int limit = bounds[seq];
        
        // Loop A: Standard counted loop
        for (int a = 0; a < limit; ++a) {
            temp[seq] += a * (seq + 1);
            if (a % (seq + 2) == 0) {
                temp[seq] += init;
                // Shared structure with Loop B
                if (seq == 1 && a > limit/2) {
                    break;
                }
            }
        }
        
        // Loop B: Different induction, shares some blocks with A
        for (int b = limit - 1; b >= 0; --b) {
            temp[seq] -= b;
            // Conditional block similar to Loop A's structure
            if (b % (seq + 3) == 0) {
                temp[seq] += init * 2;
                continue;
            }
        }
    }
    
    *checksum = temp[0] + temp[1] * 2 + temp[2] * 3;
}

__attribute__((noinline))
int hardware_loop_candidate(int iterations) {
    volatile int mod = iterations;  // Prevent optimization
    int sum = 0;
    
    // Ideal hardware loop: fixed count, simple body
    for (int i = 0; i < 32; ++i) {
        sum += i * mod;
    }
    
    // Another candidate with moderate iteration count
    int prod = 1;
    for (int j = 0; j < 24; ++j) {
        prod *= (j % 7) + 1;
        sum += prod;
    }
    
    return sum;
}

int main() {
    int result1, result2, result3;
    int outer_acc, inner_acc;
    int checksum;
    
    // Initialize with non-constant to prevent compile-time computation
    volatile int seed = 42;
    
    // Execute all loop patterns
    int cmp = complex_loops_1(seed, &result1);
    
    int diff = nested_loops_pattern(seed % 10, &outer_acc, &inner_acc);
    
    overlapping_loop_sequences(seed + cmp, &checksum);
    
    int hw_result = hardware_loop_candidate(seed % 20);
    
    // Combine all results with data dependencies
    int final_result = result1 + outer_acc * 3 - inner_acc + checksum / 2 + hw_result + diff;
    
    // Use result to prevent dead code elimination
    printf("Final checksum: %d\n", final_result);
    
    // Additional volatile store to ensure all computations are kept
    volatile int sink = final_result;
    
    return (final_result > 0) ? 0 : 1;
}
