#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) int loops_sequential_partial_overlap(int seed);
__attribute__((noinline)) int loops_nested_subset(int seed);
__attribute__((noinline)) int loops_complex_intersection(int seed);
__attribute__((noinline)) int loops_shared_header_tail(int seed);

// Global volatile to prevent optimization
volatile int g_bound = 32;

int main() {
    int result = 0;
    int seed = 12345; // Arbitrary starting value
    
    // Execute all loop patterns to force hw-doloop analysis
    result += loops_sequential_partial_overlap(seed);
    result += loops_nested_subset(seed + result);
    result += loops_complex_intersection(seed + result);
    result += loops_shared_header_tail(seed + result);
    
    // Final observable output
    printf("Final checksum: %d\n", result);
    return 0;
}

// Pattern 1: Sequential loops with partial block overlap
__attribute__((noinline)) 
int loops_sequential_partial_overlap(int seed) {
    volatile int bound = g_bound;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    // Loop A: for loop with early exit
    for (int i = 0; i < bound; ++i) {
        acc1 += i * seed;
        if (i == bound/2) {
            // This creates a conditional basic block inside the loop
            acc1 *= 2;
            continue;
        }
        acc1 -= i;
    }
    
    // Shared post-loop block (creates bitmap intersection)
    int temp = acc1;
    
    // Loop B: while loop that shares some control flow
    int j = 0;
    while (j < bound) {
        acc2 += (j * temp) % 17;
        if (j % 3 == 0) {
            acc2 += seed;
        } else {
            acc2 -= 1;
        }
        j++;
    }
    
    // Loop C: do-while with switch inside (complex basic blocks)
    int k = 0;
    do {
        switch (k % 4) {
            case 0: acc3 += temp; break;
            case 1: acc3 += acc2; break;
            case 2: acc3 *= 2; break;
            case 3: acc3 /= 2; break;
        }
        k++;
    } while (k < bound);
    
    return acc1 + acc2 + acc3;
}

// Pattern 2: Nested loops where inner is subset of outer
__attribute__((noinline))
int loops_nested_subset(int seed) {
    volatile int outer_bound = g_bound;
    volatile int inner_bound = 8; // Smaller than outer
    int acc = 0;
    
    // Outer loop with complex body
    for (int i = 0; i < outer_bound; ++i) {
        acc += i * seed;
        
        // Inner loop - its blocks are a strict subset of outer's blocks
        for (int j = 0; j < inner_bound; ++j) {
            acc += (i * j) % 7;
            if (j == inner_bound - 1) {
                acc *= 3; // Additional basic block in inner loop
            }
        }
        
        // More outer loop blocks that inner loop doesn't have
        if (i % 5 == 0) {
            acc -= seed;
            continue;
        }
        acc += 1;
    }
    
    return acc;
}

// Pattern 3: Multiple loops at same level with incomplete intersection
__attribute__((noinline))
int loops_complex_intersection(int seed) {
    volatile int bound1 = g_bound;
    volatile int bound2 = g_bound / 2;
    int acc1 = 0, acc2 = 0;
    
    // First loop with if-else chain
    for (int i = 0; i < bound1; ++i) {
        if (i < bound1/3) {
            acc1 += i * 2;
        } else if (i < 2*bound1/3) {
            acc1 += i * 3;
            continue; // Skip the rest of this iteration
        } else {
            acc1 += i * 4;
        }
        // Common block executed in all paths except continue
        acc1 += seed;
    }
    
    // Second loop that shares some but not all blocks with first
    for (int j = 0; j < bound2; ++j) {
        // This if-else structure partially overlaps with first loop's
        if (j < bound2/3) {
            acc2 += j * 2;
        } else {
            acc2 += j * 5;
        }
        // Different arithmetic than first loop
        acc2 = (acc2 * 11) % 1024;
        
        // Additional block not in first loop
        switch (j % 3) {
            case 0: acc2 += acc1; break;
            case 1: acc2 -= acc1; break;
            case 2: acc2 ^= acc1; break;
        }
    }
    
    return acc1 ^ acc2;
}

// Pattern 4: Loops sharing header/tail blocks
__attribute__((noinline))
int loops_shared_header_tail(int seed) {
    volatile int bound = g_bound;
    int acc = 0;
    int shared_temp = seed;
    
    // Shared pre-loop computation (common header block)
    int init = shared_temp * 2;
    
    // Loop X
    for (int x = 0; x < bound; x += 2) {
        acc += x * init;
        if (x % 4 == 0) {
            acc += shared_temp;
        }
    }
    
    // Reset shared_temp between loops
    shared_temp = acc % 100;
    
    // Loop Y - shares header block with Loop X
    for (int y = 1; y < bound; y += 2) {
        acc += y * init; // Same header computation as Loop X
        if (y % 3 == 0) {
            acc -= shared_temp;
            continue;
        }
        acc += 5;
    }
    
    // Shared post-loop block
    acc = (acc * 13) % 65536;
    
    // Another loop that shares the tail but not header
    for (int z = 0; z < bound/4; ++z) {
        acc += z * 7;
        // Different internal structure
        for (int w = 0; w < 3; ++w) {
            acc += w;
        }
    }
    
    // Same tail block as previous loops
    acc = (acc * 13) % 65536;
    
    return acc;
}
