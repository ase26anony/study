#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) int func1(int seed);
__attribute__((noinline)) int func2(int seed);
__attribute__((noinline)) int func3(int seed);
__attribute__((noinline)) int func4(int seed);
__attribute__((noinline)) int func5(int seed);

// Global volatile to prevent constant propagation
volatile int g_bound = 32;

// Function with sequential loops sharing blocks
__attribute__((noinline)) int func1(int seed) {
    int acc = seed;
    volatile int limit = g_bound;
    
    // First loop - will share some blocks with second loop
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            acc += i * 2;
        } else {
            acc += i;
        }
        // Common block with next loop
        if (acc > 1000) {
            acc -= 500;
        }
    }
    
    // Second sequential loop - shares the "acc > 1000" check block
    for (int j = 0; j < limit / 2; ++j) {
        acc += j * j;
        // Shared block
        if (acc > 1000) {
            acc -= 500;
        }
    }
    
    return acc;
}

// Function with nested loops (inner is subset of outer)
__attribute__((noinline)) int func2(int seed) {
    int acc = seed;
    volatile int outer_limit = g_bound;
    volatile int inner_limit = 8;
    
    for (int i = 0; i < outer_limit; ++i) {
        acc += i;
        
        // Inner loop - strict subset of outer loop's blocks
        for (int j = 0; j < inner_limit; ++j) {
            acc += (i * j);
            if (j % 2 == 0) {
                acc += 1;
            }
        }
        
        // Conditional that creates additional blocks
        switch (i % 4) {
            case 0: acc += 10; break;
            case 1: acc += 20; break;
            case 2: acc += 30; break;
            default: acc += 40; break;
        }
    }
    
    return acc;
}

// Function with do-while and while loops at same level
__attribute__((noinline)) int func3(int seed) {
    int acc = seed;
    volatile int count = g_bound;
    int i = 0;
    
    // do-while loop
    do {
        acc += i * 3;
        i++;
        if (acc % 7 == 0) {
            acc /= 2;
        }
    } while (i < count);
    
    // while loop sharing some control flow
    int j = 0;
    while (j < count / 2) {
        acc += j * 5;
        j++;
        // Shared conditional block
        if (acc % 7 == 0) {
            acc /= 2;
        }
    }
    
    return acc;
}

// Function with complex control flow and early exits
__attribute__((noinline)) int func4(int seed) {
    int acc = seed;
    volatile int limit = g_bound;
    
    for (int i = 0; i < limit; ++i) {
        // Early continue creates additional blocks
        if (i % 5 == 0) {
            continue;
        }
        
        acc += i * i;
        
        // Early break creates exit block
        if (acc > 2000) {
            acc -= 1000;
            break;
        }
        
        // Nested loop with different bounds
        for (int k = 0; k < 4; ++k) {
            acc += k;
            if (k == 2) {
                continue;  // Another continue
            }
        }
    }
    
    return acc;
}

// Function with multiple loops intersecting incompletely
__attribute__((noinline)) int func5(int seed) {
    int acc = seed;
    volatile int bound1 = g_bound;
    volatile int bound2 = g_bound / 2;
    
    // Loop A
    for (int a = 0; a < bound1; ++a) {
        acc += a * 2;
        // Shared header-like block
        if (a % 3 == 0) {
            acc += 100;
        } else {
            acc += 50;
        }
    }
    
    // Loop B - partially overlaps with Loop A's blocks
    for (int b = 0; b < bound2; ++b) {
        acc += b * 3;
        // Different conditional structure
        if (b % 4 == 0) {
            acc += 200;
        } else if (b % 4 == 1) {
            acc += 150;
        } else {
            // This else block is NOT in Loop A
            acc += 75;
        }
    }
    
    // Loop C - subset of Loop A's blocks
    for (int c = 0; c < 5; ++c) {
        acc += c * 10;
        if (c % 3 == 0) {
            acc += 100;  // Same as Loop A's true branch
        }
    }
    
    return acc;
}

int main() {
    int result = 0;
    volatile int init = 42;  // Prevent constant folding
    
    // Execute all functions to force loop analysis
    result += func1(init);
    result += func2(result);
    result += func3(result);
    result += func4(result);
    result += func5(result);
    
    // Use result to prevent dead code elimination
    printf("Final checksum: %d\n", result);
    
    // Also use in conditional to create more blocks
    if (result > 1000000) {
        printf("Large result detected\n");
    }
    
    return result % 256;
}
