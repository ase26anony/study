#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline, noipa))
void complex_loops_1(volatile int n, int *checksum) {
    int acc = 0;
    
    // Outer loop with multiple inner loops
    for (int i = 0; i < n; ++i) {
        // First inner loop - will be subset of outer loop's blocks
        for (int j = 0; j < 8; ++j) {
            acc += i * j;
            if (j == 4) continue;  // Creates conditional branch
        }
        
        // Conditional that creates shared basic block
        if (i % 2 == 0) {
            // Another loop at same nesting level as first inner loop
            for (int k = 0; k < 6; ++k) {
                acc += i + k;
                if (k == 3) break;  // Early exit creates different block structure
            }
        } else {
            // Different path with its own loop
            int m = 0;
            while (m < 5) {
                acc -= i * m;
                m++;
            }
        }
    }
    
    // Sequential loop sharing some blocks with previous loops
    int p = 0;
    do {
        acc += p * p;
        p++;
        if (p > 10) break;  // Conditional break
    } while (p < n);
    
    *checksum += acc;
}

__attribute__((noinline, noipa))
void overlapping_loops_2(volatile int m, int *checksum) {
    int acc = 0;
    
    // Two sequential loops that will share some basic blocks
    // Loop A
    for (int i = 0; i < m; ++i) {
        acc += i * 2;
        if (i == m/2) {
            // Nested loop inside conditional
            for (int j = 0; j < 4; ++j) {
                acc += j;
            }
        }
    }
    
    // Loop B - sequential to Loop A, shares some control flow
    int k = m;
    while (k > 0) {
        acc -= k;
        k--;
        
        // Switch inside loop creates complex block structure
        switch (k % 3) {
            case 0: acc += 10; break;
            case 1: acc += 20; break;
            default: acc += 30;
        }
    }
    
    // Another loop that partially overlaps with previous ones
    for (int x = 0; x < 16; ++x) {
        if (x < 8) {
            acc += x * x;
        } else {
            acc -= x;
        }
        // Function call inside loop
        if (x % 5 == 0) {
            // Small inline-like operation
            int temp = acc;
            acc = temp + 1;
        }
    }
    
    *checksum += acc;
}

__attribute__((noinline, noipa))
void nested_loop_subsets(volatile int limit, int *checksum) {
    int acc = 0;
    
    // Deep nesting creating subset relationships
    for (int a = 0; a < limit; ++a) {
        // Middle loop
        for (int b = 0; b < 12; ++b) {
            // Innermost loop - strict subset of middle loop's blocks
            for (int c = 0; c < 6; ++c) {
                acc += a * b * c;
                
                // Conditional continue creates additional blocks
                if (c % 2 == 0) continue;
                acc += 1;
            }
            
            // Additional control flow in middle loop
            if (b % 3 == 0) {
                // Another loop at same level as innermost
                int d = 0;
                while (d < 4) {
                    acc -= b * d;
                    d++;
                }
            }
        }
        
        // Loop at same level as middle loop but different structure
        int e = 0;
        do {
            acc += a + e;
            e++;
            if (e > 8) {
                // Break to outer scope
                break;
            }
        } while (e < 7);
    }
    
    *checksum += acc;
}

__attribute__((noinline, noipa))
void sequential_partial_overlap(int *checksum) {
    int acc = 0;
    volatile int bound = 20;
    
    // Series of sequential loops with partial block overlap
    // Loop 1
    for (int i = 0; i < bound; i += 2) {
        acc += i * i;
        if (i > 10) {
            // Common block pattern
            acc += 100;
        }
    }
    
    // Loop 2 - shares some blocks with Loop 1
    for (int j = 1; j < bound; j += 2) {
        acc += j * 3;
        if (j > 10) {
            // Same conditional block as Loop 1
            acc += 100;
        }
        // Additional block not in Loop 1
        switch (j % 4) {
            case 0: acc += 1; break;
            case 1: acc += 2; break;
            case 2: acc += 3; break;
            default: acc += 4;
        }
    }
    
    // Loop 3 - different structure but shares exit blocks
    int k = bound;
    while (k-- > 0) {
        acc -= k;
        if (k < 5) {
            // Different conditional
            acc *= 2;
        }
    }
    
    *checksum += acc;
}

int main() {
    volatile int iter_count = 32;  // Volatile to prevent constant propagation
    int total_checksum = 0;
    
    // Execute all loop patterns
    complex_loops_1(iter_count, &total_checksum);
    overlapping_loops_2(iter_count / 2, &total_checksum);
    nested_loop_subsets(8, &total_checksum);
    sequential_partial_overlap(&total_checksum);
    
    // Additional execution with different parameters
    complex_loops_1(16, &total_checksum);
    overlapping_loops_2(24, &total_checksum);
    
    // Final observable output
    printf("Final checksum: %d\n", total_checksum);
    
    return 0;
}
