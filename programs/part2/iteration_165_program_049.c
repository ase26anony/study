#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline, noipa))
void complex_loops_1(volatile int n, int *result) {
    int acc = 0;
    volatile int limit = n + 5;
    
    // Outer loop with complex control flow
    for (int i = 0; i < n; ++i) {
        // First inner loop - will be subset of outer loop's blocks
        for (int j = 0; j < 8; ++j) {
            acc += i * j;
            if (j == 4) {
                // Early continue creates additional basic block
                continue;
            }
            acc += 1;
        }
        
        // Conditional break in outer loop
        if (i == limit / 2) {
            // This creates shared basic block with other loops
            acc *= 2;
        }
        
        // Switch inside loop for more blocks
        switch (i % 3) {
            case 0: acc += 100; break;
            case 1: acc += 200; break;
            case 2: acc += 300; break;
        }
    }
    
    // Sequential loop at same level - shares some blocks with previous loop
    for (int k = 0; k < 16; ++k) {
        acc += k * k;
        if (k % 2 == 0) {
            // Shared basic block pattern
            acc -= 5;
        } else {
            acc += 10;
        }
    }
    
    *result = acc;
}

__attribute__((noinline, noipa))
void complex_loops_2(volatile int m, int *result) {
    int acc = 0;
    volatile int threshold = m > 10 ? 10 : m;
    
    // Do-while loop with overlapping blocks
    int counter = 0;
    do {
        // Nested while loop - strict subset of blocks
        int inner = 0;
        while (inner < 7) {
            acc += (counter * inner);
            if (inner % 3 == 0) {
                acc >>= 1;  // Creates conditional block
            }
            inner++;
        }
        
        // Another inner for loop
        for (int x = 0; x < 4; ++x) {
            acc += x + counter;
            if (x == 2) {
                continue;  // Creates continue block
            }
            acc -= 1;
        }
        
        counter++;
    } while (counter < threshold);
    
    // Loop with early exit - different block structure
    for (int i = 0; i < 20; ++i) {
        acc += i * i;
        if (acc > 1000) {
            break;  // Creates break block
        }
        if (i % 5 == 0) {
            acc += 50;
        }
    }
    
    *result = acc;
}

__attribute__((noinline, noipa))
void overlapping_loop_blocks(volatile int seed, int *result) {
    int acc = seed;
    
    // Three sequential loops that share common header/post blocks
    for (int a = 0; a < 12; ++a) {
        // Common operation in all loops
        acc += a * 2;
        
        if (a % 4 == 0) {
            // Shared conditional block
            acc += 100;
        }
    }
    
    // Second loop with partial block overlap
    for (int b = 0; b < 8; ++b) {
        // Different operation but same structure
        acc += b * 3;
        
        if (b % 3 == 0) {
            // Different condition but similar block
            acc += 50;
        } else {
            // Alternative path
            acc -= 25;
        }
    }
    
    // Third loop with different bounds but overlapping control flow
    int c = 0;
    while (c < 10) {
        acc += c * c;
        
        // Switch creates multiple blocks
        switch (c % 4) {
            case 0: acc += 10; break;
            case 1: acc += 20; 
                    // Fall through creates edge case
            case 2: acc += 30; break;
            case 3: acc += 40; break;
        }
        
        c++;
    }
    
    *result = acc;
}

__attribute__((noinline, noipa))
void deeply_nested_loops(int *result) {
    int acc = 0;
    
    // Deep nesting creates subset relationships
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 5; ++j) {
            for (int k = 0; k < 4; ++k) {
                // Innermost loop - strict subset
                acc += i + j + k;
                
                if (k == 2) {
                    // Conditional in innermost loop
                    acc *= 2;
                }
            }
            
            // Middle loop has its own conditions
            if (j % 2 == 0) {
                acc += 100;
            }
        }
        
        // Outer loop condition
        if (i > 3) {
            acc -= 50;
        }
    }
    
    // Another set at same level
    for (int x = 0; x < 8; ++x) {
        // Loop with function call (prevents some optimizations)
        for (int y = 0; y < 3; ++y) {
            acc += x * y;
            // Inline asm prevents dead code elimination
            asm volatile("" : "+r" (acc));
        }
    }
    
    *result = acc;
}

int main() {
    volatile int input = 15;  // Volatile to prevent constant propagation
    int results[4];
    int final_checksum = 0;
    
    // Execute all loop patterns
    complex_loops_1(input, &results[0]);
    complex_loops_2(input, &results[1]);
    overlapping_loop_blocks(input, &results[2]);
    deeply_nested_loops(&results[3]);
    
    // Create data dependencies between results
    for (int i = 0; i < 4; ++i) {
        final_checksum += results[i];
        if (i > 0) {
            final_checksum ^= results[i-1];  // Create dependency
        }
    }
    
    // Additional loop to ensure hw-doloop analysis
    volatile int iter = 32;  // Good candidate for hardware loop
    int hw_loop_acc = 0;
    for (int i = 0; i < iter; ++i) {
        hw_loop_acc += i * i;
        // Prevent vectorization/unrolling
        asm volatile("" : "+r" (hw_loop_acc));
    }
    
    final_checksum += hw_loop_acc;
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Individual results: %d %d %d %d %d\n", 
           results[0], results[1], results[2], results[3], hw_loop_acc);
    
    return final_checksum != 0 ? 0 : 1;
}
