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
        
        // Loop B: Nested inside A but with early exit
        for (int j = 0; j < 8; ++j) {
            if (j == i % 4) {
                sum += j * 3;
                break; // Early exit creates different block structure
            }
            sum += j;
        }
        
        // Conditional continue creates additional basic blocks
        if (i == limit / 2) {
            continue;
        }
        sum += 1;
    }
    
    // Loop C: Sequential to A, shares some control flow blocks
    int k = 0;
    while (k < limit) {
        switch (k % 3) {
            case 0: sum += k * 5; break;
            case 1: sum += k * 3; break;
            case 2: sum += k * 7; break;
        }
        
        // Loop D: Nested in C but with different bounds
        do {
            sum += (k % 2) ? -1 : 1;
            k++;
        } while (k % 4 != 0 && k < limit);
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int seed) {
    int sum = 0;
    volatile int outer_limit = 12;
    volatile int inner_limit = 8;
    
    // Loop E: Outer loop with complex exit condition
    for (int x = 0; x < outer_limit; x++) {
        // Multiple basic blocks due to conditionals
        if (x < outer_limit / 2) {
            sum += x * x;
        } else {
            sum += x;
        }
        
        // Loop F: Inner loop that's a strict subset of E's blocks
        for (int y = 0; y < inner_limit; y++) {
            // Function call to create additional basic blocks
            sum += (y % 2 == 0) ? y : -y;
            
            // Nested conditional for more block complexity
            if (y == x % 3) {
                sum += 100;
                continue;
            }
        }
        
        // Loop G: Another inner loop at same level as F
        // Shares some but not all blocks with F
        int z = 0;
        while (z < inner_limit) {
            sum += z * (x + 1);
            if (z == 3) {
                break; // Different exit path
            }
            z++;
        }
    }
    
    // Loop H: Sequential to E, partially overlapping blocks
    // due to similar structure but different bounds
    for (int w = 0; w < outer_limit + 4; w++) {
        if (w % 2 == 0) {
            for (int v = 0; v < 3; v++) {
                sum += w * v;
            }
        } else {
            sum += w;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int overlapping_loop_patterns(int base) {
    int result = base;
    volatile int mod = 7;
    
    // Multiple loops at same nesting level with shared header
    // Loop I
    for (int a = 0; a < 20; a += 2) {
        result += a * mod;
        
        // Shared conditional block
        if (a % 4 == 0) {
            result -= 5;
        }
    }
    
    // Loop J: Shares the post-loop block with I
    // but has different internal structure
    for (int b = 1; b < 20; b += 2) {
        result += b * (mod - 1);
        
        // Different conditional structure
        switch (b % 3) {
            case 0: result += 10; break;
            case 1: result += 20; break;
            case 2: result += 30; break;
        }
    }
    
    // Loop K: Contains Loop L as strict subset
    for (int c = 0; c < 15; c++) {
        result += c;
        
        // Loop L: All blocks are within Loop K
        for (int d = 0; d < 5; d++) {
            result += d * c;
            if (d == c % 3) {
                result += 50;
            }
        }
        
        // Additional block in K not in L
        if (c == 10) {
            result *= 2;
        }
    }
    
    // Loop M: Partially overlaps with K's blocks
    // due to similar bounds but different internal logic
    for (int e = 5; e < 20; e++) {
        result -= e;
        
        // Different nesting structure
        if (e > 10) {
            for (int f = 0; f < 2; f++) {
                result += f * 100;
            }
        }
    }
    
    return result;
}

int main() {
    int seed = 42;
    int total = 0;
    
    // Execute all loop patterns with data dependencies
    int r1 = complex_loops_1(seed);
    total += r1;
    
    int r2 = complex_loops_2(r1);
    total += r2;
    
    int r3 = overlapping_loop_patterns(r2);
    total += r3;
    
    // Additional execution with different parameters
    // to ensure loops are analyzed multiple times
    for (int iteration = 0; iteration < 3; iteration++) {
        total += complex_loops_1(seed + iteration);
        total += complex_loops_2(total);
    }
    
    // Final observable output
    printf("Final checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
