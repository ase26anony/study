#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) int func1(int seed);
__attribute__((noinline)) int func2(int seed);
__attribute__((noinline)) int func3(int seed);
__attribute__((noinline)) int func4(int seed);

// Global volatile to prevent optimization
volatile int g_bound = 32;

// Function 1: Contains sequential loops with overlapping blocks
__attribute__((noinline)) int func1(int seed) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    volatile int v = seed;
    
    // First loop - will share some blocks with second loop
    for (int i = 0; i < g_bound; ++i) {
        acc1 += i * v;
        if (i % 3 == 0) {
            acc1 += v;
            continue;  // Creates additional basic block
        }
        acc1 -= i;
    }
    
    // Shared basic block here (post-loop code)
    int temp = acc1;
    
    // Second loop - overlaps with first loop's blocks
    for (int j = 0; j < g_bound / 2; ++j) {
        acc2 += j * temp;
        if (j % 2 == 0) {
            acc2 += temp;
            // Same conditional structure as first loop
            if (j % 4 == 0) {
                continue;
            }
        }
        acc2 -= j;
    }
    
    // Third loop - nested inside conditional
    if (v > 10) {
        for (int k = 0; k < 16; ++k) {
            acc3 += k * v;
            switch (k % 3) {
                case 0: acc3 += 1; break;
                case 1: acc3 += 2; break;
                case 2: acc3 += 3; break;
            }
        }
    }
    
    return acc1 + acc2 + acc3;
}

// Function 2: Contains nested loops where inner is subset of outer
__attribute__((noinline)) int func2(int seed) {
    int acc = 0;
    volatile int limit = seed % 8 + 8;
    
    // Outer loop
    for (int i = 0; i < limit; ++i) {
        acc += i * 3;
        
        // Inner loop - strict subset of outer's blocks
        for (int j = 0; j < 4; ++j) {
            acc += j * seed;
            if (j % 2 == 0) {
                acc += 1;
                continue;
            }
            acc -= 1;
        }
        
        // More code in outer loop after inner
        if (i % 2 == 0) {
            acc += seed;
        } else {
            acc -= seed;
        }
    }
    
    return acc;
}

// Function 3: Complex loop structure with early exits
__attribute__((noinline)) int func3(int seed) {
    int acc = 0;
    int bound = g_bound;
    
    // Loop with multiple exit points
    for (int i = 0; i < bound; ++i) {
        acc += i * seed;
        
        if (acc > 1000) {
            break;  // Early exit creates separate block
        }
        
        if (i % 5 == 0) {
            continue;
        }
        
        // Nested loop with different bound
        for (int j = 0; j < 3; ++j) {
            acc += j;
            if (j == 1) {
                continue;
            }
            acc += seed;
        }
        
        if (i == bound / 2) {
            acc *= 2;
        }
    }
    
    // Another sequential loop sharing some blocks
    int temp = acc;
    for (int k = 0; k < 8; ++k) {
        temp += k * seed;
        if (k % 2 == 0) {
            temp += acc;
        }
    }
    
    return acc + temp;
}

// Function 4: Do-while and while loops mixed
__attribute__((noinline)) int func4(int seed) {
    int acc = seed;
    int count = 0;
    
    // Do-while loop
    do {
        acc += count * 2;
        count++;
        
        // Small inner while loop
        int inner = 0;
        while (inner < 3) {
            acc += inner;
            inner++;
            if (inner == 2) {
                continue;  // Creates additional block
            }
            acc -= 1;
        }
        
    } while (count < g_bound / 4);
    
    // While loop with complex condition
    int i = 0;
    while (i < 8) {
        acc += i * seed;
        i++;
        
        if (i == 4) {
            // Another tiny loop
            for (int j = 0; j < 2; ++j) {
                acc += j * 10;
            }
            continue;
        }
        
        acc += 5;
    }
    
    return acc;
}

int main() {
    int seed = 42;  // Arbitrary seed
    int result = 0;
    
    // Force data dependencies between functions
    int r1 = func1(seed);
    result += r1;
    
    int r2 = func2(r1 % 100);
    result += r2;
    
    int r3 = func3(r2 % 100);
    result += r3;
    
    int r4 = func4(r3 % 100);
    result += r4;
    
    // Final checksum to ensure all loops are live
    printf("Final checksum: %d\n", result);
    
    return 0;
}
