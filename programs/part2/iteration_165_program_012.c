#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int loops_partial_overlap(int seed) {
    volatile int limit = 32; // Prevent constant propagation
    int sum = 0;
    
    // Loop A: Will share some blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Loop B: Nested inside A but with early exit
        // Creates partial block overlap with A
        for (int j = 0; j < 16; ++j) {
            if (j == i % 8) {
                sum += j * 3;
                break; // Early exit creates different block structure
            }
            sum += j;
        }
        
        // Conditional continue creates additional blocks
        if (i % 5 == 0) {
            continue;
        }
        sum += 1;
    }
    
    return sum;
}

__attribute__((noinline))
int loops_sequential_shared(int seed) {
    volatile int n = 24;
    int acc1 = 0, acc2 = 0;
    
    // Loop C and D are sequential at same nesting level
    // They share the function prologue/epilogue blocks
    
    // Loop C: Has complex control flow
    for (int i = 0; i < n; ++i) {
        switch (i % 4) {
            case 0: acc1 += i * 2; break;
            case 1: acc1 += i * 3; break;
            case 2: acc1 += i * 4; break;
            default: acc1 += i; break;
        }
        
        if (i % 7 == 0) {
            continue;
        }
        acc1 += seed;
    }
    
    // Loop D: Different structure but shares some blocks with C
    // (like the function's entry/exit blocks)
    for (int i = n - 1; i >= 0; --i) {
        if (i % 2 == 0) {
            acc2 += i * i;
        } else {
            acc2 += i + seed;
        }
        
        // Small inner loop - subset of D's blocks
        for (int k = 0; k < 4; ++k) {
            acc2 += k;
            if (k == 2) continue;
        }
    }
    
    return acc1 + acc2;
}

__attribute__((noinline))
int loops_strict_subset(int seed) {
    volatile int outer = 20;
    volatile int inner = 8;
    int total = 0;
    
    // Outer loop E contains inner loop F
    // F's blocks are a strict subset of E's blocks
    for (int x = 0; x < outer; ++x) {
        total += x * seed;
        
        // Multiple conditionals create many basic blocks
        if (x % 3 == 0) {
            total += 100;
        } else if (x % 3 == 1) {
            total += 200;
        } else {
            total += 300;
        }
        
        // Loop F: Strict subset of E's blocks
        for (int y = 0; y < inner; ++y) {
            total += y * x;
            if (y % 2 == 0) {
                total += 5;
            }
        }
        
        // More blocks in outer loop only
        switch (x % 4) {
            case 0: total += 1; break;
            case 1: total += 2; break;
            case 2: total += 3; break;
            case 3: total += 4; break;
        }
    }
    
    return total;
}

__attribute__((noinline))
int loops_disjoint_with_shared_header(int seed) {
    int result = 0;
    volatile int count = 16;
    
    // Loop G and H are at same level with some shared pre-header blocks
    // but mostly disjoint bodies
    
    // Shared setup code
    int base = seed * 2;
    
    // Loop G
    for (int i = 0; i < count; i += 2) {
        result += base + i;
        if (i % 3 == 0) {
            result *= 2;
        }
    }
    
    // Loop H - different step, different condition
    for (int i = 1; i < count; i += 2) {
        result += base - i;
        for (int j = 0; j < 3; ++j) {
            result += j;
            if (j == 1) break;
        }
    }
    
    return result;
}

int main() {
    volatile int seed = 42; // Prevent constant folding
    int checksum = 0;
    
    // Execute all loop patterns to ensure analysis
    checksum += loops_partial_overlap(seed);
    checksum += loops_sequential_shared(seed + 1);
    checksum += loops_strict_subset(seed + 2);
    checksum += loops_disjoint_with_shared_header(seed + 3);
    
    // Use result to prevent dead code elimination
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
