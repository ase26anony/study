#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 32; // Prevent constant propagation
    int sum = seed;
    
    // Loop A: Sequential loop that will share some blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
            continue; // Creates additional basic blocks
        }
        
        // Nested loop inside Loop A (strict subset of blocks)
        for (int j = 0; j < 8; ++j) {
            sum += j;
            if (j == 4) break; // Early exit creates more blocks
        }
        
        if (i > 20) {
            sum *= 2;
            break; // Another early exit path
        }
    }
    
    // Loop B: Sequential loop at same level as Loop A
    // Shares some basic blocks (header, condition check) but not all
    int k = 0;
    while (k < limit) {
        sum += k * 3;
        
        // Conditional that creates shared block structure
        if (k % 2 == 0) {
            sum -= 1;
        } else {
            // Another nested loop (different structure than Loop A's inner)
            do {
                sum += 2;
                if (sum > 1000) continue; // Continue in inner loop
            } while (++k % 5 != 0);
        }
        
        k++;
    }
    
    return sum;
}

__attribute__((noinline))
int overlapping_loops_2(int base) {
    volatile int iterations = 16;
    int result = base;
    
    // Loop C: Contains switch statement for complex control flow
    for (int x = 0; x < iterations; x++) {
        switch (x % 4) {
            case 0:
                result += x;
                break;
            case 1:
                result -= x;
                // Fall through to create shared block
            case 2:
                result *= (x + 1);
                break;
            default:
                // Loop D: Nested inside Loop C but with early exit
                for (int y = 0; y < x; y++) {
                    result += y;
                    if (y > 3) goto early_exit; // Complicated control flow
                }
                early_exit:
                break;
        }
        
        // Loop E: Another loop at same nesting level inside Loop C
        // Partially overlaps with Loop D's blocks
        int z = 0;
        while (z < 5) {
            result ^= (1 << z);
            z++;
            if (z == 3) continue; // Skip some iterations
        }
    }
    
    // Loop F: Sequential to Loop C, shares post-loop blocks
    for (int w = iterations; w > 0; w--) {
        result += w * w;
        
        // Conditional function call to prevent optimization
        if (w % 7 == 0) {
            // Small helper function to create call block
            result = helper_func(result, w);
        }
    }
    
    return result;
}

// Small helper to prevent dead code elimination
__attribute__((noinline))
int helper_func(int val, int idx) {
    return val + (idx % 3);
}

__attribute__((noinline))
int nested_subset_loops_3(int init) {
    volatile int outer_limit = 24;
    volatile int inner_limit = 12;
    int acc = init;
    
    // Loop G: Outer loop containing multiple inner loops
    for (int a = 0; a < outer_limit; a += 2) {
        acc += a;
        
        // Loop H: Inner loop - strict subset of Loop G's blocks
        for (int b = 0; b < inner_limit; b++) {
            acc += b * a;
            if (b == a % 8) break; // Variable break condition
        }
        
        // Loop I: Another inner loop at same level as Loop H
        // Partially overlaps with Loop H but not identical
        int c = 0;
        while (c < 5) {
            acc -= c;
            c++;
            if (acc < 0) acc = 0; // Reset condition
        }
        
        // Loop J: Do-while loop with different structure
        int d = 0;
        do {
            acc ^= d;
            d++;
        } while (d < 3);
    }
    
    // Loop K: Sequential to Loop G, shares some blocks
    for (int e = outer_limit - 1; e >= 0; e--) {
        acc += e;
        
        // Multiple conditions creating complex block graph
        if (e % 3 == 0) {
            for (int f = 0; f < 4; f++) {
                acc += f * e;
            }
        } else if (e % 3 == 1) {
            acc *= 2;
        }
    }
    
    return acc;
}

int main() {
    volatile int seed = 42; // Prevent constant folding
    int total = 0;
    
    // Execute all loop patterns to ensure analysis
    total += complex_loops_1(seed);
    total += overlapping_loops_2(total);
    total += nested_subset_loops_3(total);
    
    // Add some more simple loops to increase coverage
    for (int i = 0; i < 100; i++) {
        total += i;
        if (i % 10 == 0) {
            for (int j = 0; j < i % 5; j++) {
                total -= j;
            }
        }
    }
    
    // Final observable output
    printf("Result checksum: %d\n", total);
    
    return total != 0 ? 0 : 1; // Return non-error if computation occurred
}
