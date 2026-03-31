#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline,noipa))
void complex_loops_1(volatile int n, int *checksum) {
    int acc = 0;
    volatile int limit = n + 5;
    
    // Outer loop - will share some blocks with inner loops
    for (int i = 0; i < limit; ++i) {
        // First inner loop - subset of outer loop's blocks
        for (int j = 0; j < 8; ++j) {
            acc += i * j;
            if (j == 4) continue;  // Creates conditional branch
        }
        
        // Conditional that creates shared block with sequential loops
        if (i % 3 == 0) {
            acc += i * 2;
        } else {
            acc += i * 3;
        }
        
        // Early exit creates another block
        if (acc > 1000) break;
    }
    
    // Sequential loop at same level - shares some blocks with above
    for (int k = 0; k < limit / 2; ++k) {
        acc += k * 7;
        // Switch creates multiple basic blocks
        switch (k % 3) {
            case 0: acc += 1; break;
            case 1: acc += 2; break;
            default: acc += 3;
        }
    }
    
    *checksum += acc;
}

__attribute__((noinline,noipa))
void overlapping_loops_2(volatile int m, int *checksum) {
    int acc = 0;
    volatile int iter = m > 10 ? 16 : 24;
    
    // Do-while loop with complex condition
    int x = 0;
    do {
        // Nested while loop - strict subset of do-while's blocks
        int y = 0;
        while (y < 8) {
            acc += x * y;
            y++;
            if (y == 5) {
                acc += 100;  // Creates another block
                continue;
            }
        }
        
        // Another inner for loop
        for (int z = 0; z < 4; ++z) {
            acc += (x + z) * 3;
            if (z == 2) break;  // Early exit
        }
        
        x++;
        // Complex loop condition with function call simulation
        if (x > iter / 2) {
            acc += 50;
        }
    } while (x < iter);
    
    // Sequential loop sharing the exit block
    for (int i = iter; i > 0; i -= 2) {
        acc += i * i;
        // Conditional continue
        if (i % 5 == 0) continue;
        acc += i;
    }
    
    *checksum += acc;
}

__attribute__((noinline,noipa))
void nested_switch_loops(volatile int p, int *checksum) {
    int acc = 0;
    volatile int count = p % 8 + 12;
    
    // Loop with switch inside
    for (int outer = 0; outer < count; ++outer) {
        // Multiple inner loops at same nesting level
        for (int inner1 = 0; inner1 < 6; ++inner1) {
            acc += outer * inner1;
            
            // Switch creates multiple basic blocks
            switch (inner1 % 4) {
                case 0:
                    // Another nested loop inside switch case
                    for (int deep = 0; deep < 3; ++deep) {
                        acc += deep;
                        if (deep == 1) continue;
                    }
                    break;
                case 1:
                    acc += 10;
                    break;
                case 2:
                    // Do-while inside switch
                    int d = 0;
                    do {
                        acc += d * 2;
                        d++;
                    } while (d < 3);
                    break;
                default:
                    acc += 5;
            }
        }
        
        // Second inner loop with different structure
        int inner2 = 0;
        while (inner2 < 4) {
            acc += outer + inner2;
            inner2++;
            if (inner2 == 2) {
                // Continue creates another edge
                continue;
            }
            acc += 1;
        }
        
        // Conditional break in outer loop
        if (acc > 2000 && outer > count / 2) {
            acc += 100;
            break;
        }
    }
    
    *checksum += acc;
}

__attribute__((noinline,noipa))
void sequential_hardware_candidates(int *checksum) {
    int acc = 0;
    
    // Series of loops at same level - potential for overlapping blocks
    // Loop 1: Fixed count, hardware candidate
    for (int i = 0; i < 32; ++i) {
        acc += i * i;
        if (i == 16) acc += 50;  // Conditional
    }
    
    // Loop 2: Different count but similar structure
    for (int j = 0; j < 24; ++j) {
        acc += j * 3;
        // Shared pattern with Loop 1
        if (j == 12) acc += 25;
    }
    
    // Loop 3: While loop version
    int k = 0;
    while (k < 16) {
        acc += k * 7;
        k++;
        // No condition here - different block structure
    }
    
    // Loop 4: Do-while with post-loop block
    int m = 0;
    do {
        acc += m * 2;
        m++;
    } while (m < 20);
    
    *checksum += acc;
}

int main(int argc, char **argv) {
    int checksum = 0;
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    // Execute all loop patterns with varying parameters
    // to create different block bitmap overlaps
    complex_loops_1(seed, &checksum);
    overlapping_loops_2(seed + 3, &checksum);
    nested_switch_loops(seed + 7, &checksum);
    sequential_hardware_candidates(&checksum);
    
    // Additional execution with different parameters
    // to explore more block overlap scenarios
    complex_loops_1(seed * 2, &checksum);
    overlapping_loops_2(seed * 3, &checksum);
    
    // Final observable output
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
