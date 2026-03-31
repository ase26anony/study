#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) int loops_sequential_overlap(int seed);
__attribute__((noinline)) int loops_nested_subsets(int seed);
__attribute__((noinline)) int loops_complex_partial_overlap(int seed);
__attribute__((noinline)) int loops_mixed_patterns(int seed);

// Volatile variables to prevent constant propagation
volatile int g_bound1 = 32;
volatile int g_bound2 = 16;
volatile int g_bound3 = 8;

int main() {
    int result = 0;
    int seed = 12345; // Arbitrary starting value
    
    // Execute different loop patterns to create various basic block overlaps
    result ^= loops_sequential_overlap(seed);
    result ^= loops_nested_subsets(seed + 1);
    result ^= loops_complex_partial_overlap(seed + 2);
    result ^= loops_mixed_patterns(seed + 3);
    
    printf("Final checksum: %d\n", result);
    return 0;
}

// Pattern 1: Sequential loops with overlapping blocks
__attribute__((noinline)) 
int loops_sequential_overlap(int seed) {
    volatile int bound = g_bound1;
    int acc1 = seed, acc2 = seed * 2;
    
    // First loop - creates blocks that will be partially shared
    for (int i = 0; i < bound; ++i) {
        if (i % 3 == 0) {
            acc1 += i * 2;
            // Early continue creates additional basic block
            continue;
        }
        acc1 ^= i;
        
        // Nested conditional for more blocks
        switch (i % 4) {
            case 0: acc1 += 1; break;
            case 1: acc1 -= 1; break;
            case 2: acc1 *= 2; break;
            default: acc1 /= 2; break;
        }
    }
    
    // Shared post-loop block (will be in both loop's bitmaps)
    int temp = acc1;
    
    // Second loop at same nesting level, partially overlapping blocks
    // Uses same bound variable but different iteration pattern
    for (int j = 0; j < bound; ++j) {
        // This if-else structure creates blocks that overlap with first loop
        if (j % 2 == 0) {
            acc2 += j * 3;
            if (j > bound/2) {
                // Early break creates exit block
                break;
            }
        } else {
            acc2 ^= j * 7;
        }
        
        // Shared operation with first loop
        temp = acc2;
        switch (j % 3) {
            case 0: acc2 += temp; break;
            case 1: acc2 -= temp; break;
            default: acc2 ^= temp; break;
        }
    }
    
    // Third loop that shares the initialization and increment blocks
    int acc3 = seed;
    int k = 0;
    do {
        acc3 += k * k;
        k++;
        if (k % 5 == 0) {
            // Function call creates additional basic block
            acc3 = abs(acc3);
        }
    } while (k < bound);
    
    return acc1 ^ acc2 ^ acc3;
}

// Pattern 2: Nested loops where inner loop blocks are subset of outer
__attribute__((noinline))
int loops_nested_subsets(int seed) {
    volatile int outer_bound = g_bound2;
    volatile int inner_bound = g_bound3;
    int acc = seed;
    
    // Outer loop with many basic blocks
    for (int i = 0; i < outer_bound; ++i) {
        // Multiple conditionals in outer loop
        if (i % 2 == 0) {
            acc += i * i;
        } else {
            acc -= i;
        }
        
        // Inner loop - its blocks are a strict subset of outer loop's blocks
        for (int j = 0; j < inner_bound; ++j) {
            // Simple inner loop body
            acc += j * 3;
            
            // Conditional that creates additional block
            if (j % 3 == 0) {
                acc ^= 0xFF;
                continue;
            }
            
            // Another inner conditional
            switch (j % 2) {
                case 0: acc += 1; break;
                case 1: acc -= 1; break;
            }
        }
        
        // More outer loop code after inner loop
        if (i > outer_bound / 2) {
            acc *= 2;
        }
    }
    
    // Another outer loop with different inner loop pattern
    int acc2 = seed;
    for (int x = 0; x < outer_bound; x += 2) {
        acc2 += x * x;
        
        // Inner loop with early exit
        for (int y = 0; y < inner_bound; ++y) {
            acc2 += y;
            if (y == inner_bound - 2) {
                break; // Early exit creates different block structure
            }
        }
        
        // Post-inner loop block
        acc2 = acc2 % 1000;
    }
    
    return acc ^ acc2;
}

// Pattern 3: Complex partial overlap between loops
__attribute__((noinline))
int loops_complex_partial_overlap(int seed) {
    volatile int bound = g_bound1;
    int acc1 = seed, acc2 = seed * 3, acc3 = seed * 5;
    
    // Loop A with specific block pattern
    int i = 0;
    while (i < bound) {
        acc1 += i * i;
        
        // Complex conditional chain
        if (i % 3 == 0) {
            acc1 += 100;
            if (i % 6 == 0) {
                acc1 -= 50;
            }
        } else if (i % 4 == 0) {
            acc1 *= 2;
        } else {
            acc1 /= 2;
        }
        
        i++;
        
        // Function call in middle of loop
        if (i % 7 == 0) {
            acc1 = abs(acc1);
        }
    }
    
    // Loop B that partially overlaps with Loop A's blocks
    // Different structure but shares some common blocks
    for (int j = 0; j < bound; ++j) {
        // This if-else partially matches Loop A's structure
        if (j % 3 == 0) {
            acc2 += j * 2;
            // Different block than Loop A
            acc2 ^= 0xAA;
        } else {
            // Similar to Loop A but not identical
            acc2 -= j;
        }
        
        // Shared switch structure with Loop A
        switch (j % 4) {
            case 0: acc2 += 10; break;
            case 1: acc2 -= 10; break;
            case 2: acc2 *= 3; break;
            default: acc2 /= 3; break;
        }
    }
    
    // Loop C that intersects with both A and B
    int k = bound - 1;
    do {
        acc3 += k;
        
        // Conditional that appears in both A and B
        if (k % 3 == 0) {
            acc3 += 5;
        }
        
        // Unique block not in A or B
        if (k < bound / 2) {
            acc3 = ~acc3;
        }
        
        k--;
    } while (k >= 0);
    
    return acc1 + acc2 + acc3;
}

// Pattern 4: Mixed loop patterns for maximum coverage
__attribute__((noinline))
int loops_mixed_patterns(int seed) {
    volatile int bound1 = g_bound1;
    volatile int bound2 = g_bound2;
    int result = seed;
    
    // Multiple loops in sequence with varying structures
    for (int a = 0; a < bound1; a += 2) {
        result += a * a;
        
        // Nested loop with subset relationship
        for (int b = 0; b < bound2; ++b) {
            result ^= b;
            if (b % 5 == 0) {
                result += 1000;
                continue;
            }
        }
    }
    
    // While loop with different structure
    int c = 0;
    while (c < bound1) {
        result -= c * 3;
        c++;
        
        // Early exit condition
        if (result > 1000000) {
            break;
        }
    }
    
    // Do-while loop that shares some blocks with previous loops
    int d = 0;
    do {
        // Operation similar to first loop
        result += d * d;
        
        // Conditional chain like in other loops
        if (d % 3 == 0) {
            result += 1;
        } else if (d % 4 == 0) {
            result -= 1;
        }
        
        d++;
    } while (d < bound2);
    
    // Final loop with function calls
    for (int e = 0; e < bound1; ++e) {
        // Function call creates unique basic block
        result = abs(result);
        
        // Complex switch
        switch (e % 6) {
            case 0: result += e; break;
            case 1: result -= e; break;
            case 2: result *= 2; break;
            case 3: result /= 2; break;
            case 4: result ^= e; break;
            default: result |= e; break;
        }
    }
    
    return result;
}
