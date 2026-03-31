#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(volatile int seed) {
    int sum = 0;
    
    // Outer loop with complex control flow
    for (int i = 0; i < (seed & 0x1F) + 10; ++i) {
        // First inner loop - will be subset of outer loop's blocks
        for (int j = 0; j < 8; ++j) {
            if (j & 1) {
                sum += i * j;
            } else {
                sum -= i * j;
            }
        }
        
        // Conditional break creates additional basic blocks
        if (sum > 1000) {
            break;
        }
        
        // Second inner loop with different structure
        int k = 0;
        while (k < 5) {
            sum += (i * k) >> 1;
            if (k == 3) continue;  // Skip iteration
            k++;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(volatile int seed) {
    int prod = 1;
    int count = (seed & 0xF) + 5;
    
    // Sequential loop 1 - shares some blocks with loop 2
    for (int i = 0; i < count; ++i) {
        switch (i % 3) {
            case 0: prod += i * 2; break;
            case 1: prod -= i * 3; break;
            case 2: prod *= (i + 1); break;
        }
        
        // Early exit creates another basic block
        if (prod < 0) {
            prod = -prod;
            break;
        }
    }
    
    // Sequential loop 2 - partially overlaps with loop 1's blocks
    // due to shared control structures
    for (int i = count - 1; i >= 0; --i) {
        if (i % 2 == 0) {
            prod += i * i;
        } else {
            // Nested loop inside sequential loop
            for (int j = 0; j < 3; ++j) {
                prod += j;
                if (j == 1) continue;
            }
        }
    }
    
    return prod;
}

__attribute__((noinline))
int overlapping_loops(volatile int bound) {
    int acc = 0;
    int limit = (bound & 0x7) + 4;
    
    // Three sequential loops that will share some basic blocks
    // due to similar structure but different bounds
    
    // Loop A
    for (int a = 0; a < limit; ++a) {
        acc += a * 2;
        if (a == limit / 2) {
            acc += 100;  // Creates additional block
        }
    }
    
    // Loop B - similar structure to A but with different condition
    for (int b = 0; b < limit * 2; ++b) {
        acc -= b;
        if (b % 3 == 0) {
            // Small inner loop - strict subset of Loop B's blocks
            for (int inner = 0; inner < 2; ++inner) {
                acc += inner * b;
            }
        }
    }
    
    // Loop C - do-while with overlapping exit block
    int c = 0;
    do {
        acc ^= c;
        c++;
        
        // Conditional continue creates another block
        if (c % 4 == 0) continue;
        
    } while (c < limit + 2);
    
    return acc;
}

__attribute__((noinline))
int nested_subset_loops(int base) {
    int result = base;
    
    // Outer loop with many basic blocks
    for (int outer = 0; outer < 16; ++outer) {
        // Multiple exit points
        if (outer == 8) {
            result += 1000;
            continue;
        }
        
        if (outer == 12) {
            break;
        }
        
        // Inner loop 1 - strict subset of outer loop's blocks
        for (int inner1 = 0; inner1 < 4; ++inner1) {
            result += outer * inner1;
            
            // Very inner loop - subset of inner1's blocks
            for (int very_inner = 0; very_inner < 2; ++very_inner) {
                result -= very_inner;
            }
        }
        
        // Inner loop 2 - different structure but still subset
        int inner2 = 0;
        while (inner2 < 3) {
            result ^= (outer << inner2);
            inner2++;
        }
    }
    
    return result;
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    int checksum = 0;
    
    // Execute all loop patterns to ensure analysis
    checksum += complex_loops_1(seed);
    checksum += complex_loops_2(seed + 1);
    checksum += overlapping_loops(seed + 2);
    checksum += nested_subset_loops(checksum);
    
    // Additional sequential loops at same level in main
    for (int i = 0; i < 20; ++i) {
        checksum += i * i;
        
        // Conditional with nested loop
        if (i % 5 == 0) {
            for (int j = 0; j < 3; ++j) {
                checksum -= j;
            }
        }
    }
    
    // Another loop sharing some control flow blocks
    int k = 0;
    while (k < 15) {
        checksum ^= k;
        k++;
        
        if (k == 10) {
            // Small loop inside - creates subset relationship
            for (int m = 0; m < 2; ++m) {
                checksum += m * 10;
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
