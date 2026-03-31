#include <stdio.h>
#include <stdint.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline, noipa))
void complex_loops_1(volatile int n, int *checksum) {
    int acc = 0;
    
    // Outer loop - will share some blocks with inner loops
    for (int i = 0; i < n; ++i) {
        // First inner loop - subset of outer loop's blocks
        for (int j = 0; j < 16; ++j) {
            if (j % 3 == 0) {
                acc += i * j;
            } else {
                acc += i + j;
            }
        }
        
        // Conditional block that might be shared with other loops
        if (i % 2 == 0) {
            // Another loop at same level but different structure
            int k = 0;
            while (k < 8) {
                acc += (i << k);
                if (k == 4) break;
                k++;
            }
        }
    }
    
    // Sequential loop sharing some blocks with previous loops
    for (int i = 0; i < 32; ++i) {
        switch (i % 4) {
            case 0: acc += i * 2; break;
            case 1: acc += i * 3; continue;  // Skip to next iteration
            case 2: acc += i * 4; break;
            default: acc += i * 5;
        }
        
        // Nested do-while with different exit condition
        int m = 0;
        do {
            acc += m * i;
            m++;
        } while (m < 4);
    }
    
    *checksum += acc;
}

__attribute__((noinline, noipa))
void overlapping_loops_2(volatile int start, volatile int end, int *checksum) {
    int acc = 0;
    
    // Loop with complex exit conditions
    for (int i = start; i < end; i += 2) {
        // Multiple nested loops creating overlapping block sets
        for (int j = 0; j < i % 10 + 5; ++j) {
            if (j > 3) {
                for (int k = 0; k < 4; ++k) {
                    acc += (i * j) >> k;
                    if (k == 2) continue;
                }
            } else {
                acc += i ^ j;
            }
        }
        
        // Early exit creates additional basic blocks
        if (acc > 10000) {
            break;
        }
    }
    
    // Another loop that shares the function's prologue/epilogue
    int counter = 0;
    while (counter < 16) {
        // Loop with switch creating many basic blocks
        switch (counter) {
            case 0: case 1: case 2:
                acc += counter * 10;
                break;
            case 3: case 4: case 5:
                acc += counter * 20;
                // Fall through
            case 6:
                acc += 100;
                break;
            default:
                acc += counter;
        }
        
        // Small inner loop - strict subset of while's blocks
        for (int x = 0; x < 3; ++x) {
            acc += x * counter;
        }
        
        counter++;
    }
    
    *checksum += acc;
}

__attribute__((noinline, noipa))
void hardware_loop_candidates(int *checksum) {
    int acc = 0;
    
    // Good hardware loop candidate: fixed, moderate iteration count
    for (int i = 0; i < 32; ++i) {
        // Arithmetic operations to prevent dead code elimination
        acc += i * i;
        
        // Conditional creates additional basic blocks
        if (i % 8 == 0) {
            acc += 1000;
        }
    }
    
    // Another candidate with different structure
    for (int i = 31; i >= 0; --i) {
        acc += (i << 2);
        
        // Nested loop that's a subset
        for (int j = 0; j < 4; ++j) {
            acc += j;
            if (j == 2) continue;
        }
    }
    
    // Do-while loop with post-test condition
    int k = 0;
    do {
        acc += k * 3;
        k++;
        
        // Early continue creates additional edge
        if (k % 5 == 0) continue;
        
        acc += 7;
    } while (k < 24);
    
    *checksum += acc;
}

__attribute__((noinline, noipa))
void loop_interactions(volatile int param, int *checksum) {
    int acc = 0;
    
    // Loop with variable bound (but compile-time known after inlining prevention)
    for (int i = 0; i < (param & 0x1F) + 16; ++i) {
        // Multiple exit points
        if (acc > 5000) {
            break;
        }
        
        // Nested loops with different scopes
        {
            int temp = 0;
            for (int j = 0; j < 8; ++j) {
                temp += i * j;
                if (j == i % 8) break;
            }
            acc += temp;
        }
        
        // Another loop at same level
        if (i % 3 == 0) {
            for (int k = 0; k < 6; ++k) {
                acc += k;
                // Continue to next iteration of inner loop
                if (k == 3) continue;
                acc += 1;
            }
        }
    }
    
    // Sequential loop sharing function epilogue
    for (int i = 0; i < 12; ++i) {
        // Complex body with multiple basic blocks
        if (i < 6) {
            acc += i * 100;
        } else {
            acc += i * 200;
            // Nested loop - subset of outer loop's blocks
            for (int j = 0; j < 2; ++j) {
                acc += j;
            }
        }
    }
    
    *checksum += acc;
}

int main() {
    volatile int loop_bound = 20;  // Prevent constant propagation
    volatile int start_val = 5;
    volatile int end_val = 25;
    volatile int param = 42;
    
    int total_checksum = 0;
    
    // Execute all loop patterns to ensure analysis
    complex_loops_1(loop_bound, &total_checksum);
    overlapping_loops_2(start_val, end_val, &total_checksum);
    hardware_loop_candidates(&total_checksum);
    loop_interactions(param, &total_checksum);
    
    // Use result to prevent dead code elimination
    printf("Final checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
