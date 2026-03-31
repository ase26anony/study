#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // volatile to prevent optimization
    int sum = 0;
    
    // Loop A: Will share some blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Loop B: Nested inside A, subset of A's blocks
        for (int j = 0; j < 8; ++j) {
            if (j % 2 == 0) {
                sum += j * seed;
            } else {
                continue; // Early continue creates separate block
            }
            // Small block that might be shared
            sum += 1;
        }
        
        if (i == limit - 1) {
            break; // Early break creates another block
        }
    }
    
    // Loop C: Sequential to A, shares some blocks through similar structure
    for (int k = 0; k < limit; ++k) {
        switch (k % 4) {
            case 0:
                sum += k * 3;
                break;
            case 1:
                sum += k * 2;
                // Fall through
            case 2:
                sum += seed;
                break;
            default:
                sum += 1;
                break;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int base) {
    int acc = base;
    volatile int outer_limit = 12;
    volatile int inner_limit = 6;
    
    // Loop D: Outer loop with multiple exits
    int d = 0;
    do {
        if (d % 5 == 0) {
            acc += d * 7;
            // Early continue
            d++;
            continue;
        }
        
        // Loop E: Inner with subset relationship to D
        for (int e = 0; e < inner_limit; ++e) {
            acc += e * d;
            if (e == 3) {
                acc += 100; // Creates unique block
            }
        }
        
        // Conditional break
        if (acc > 1000) {
            break;
        }
        
        d++;
    } while (d < outer_limit);
    
    // Loop F: Sequential to D, partially overlapping blocks
    int f = 0;
    while (f < outer_limit) {
        acc += f * 2;
        
        // Nested loop G: Strict subset of F's blocks
        for (int g = 0; g < 4; ++g) {
            acc += g;
            if (g == 2) {
                continue;
            }
            acc += 1;
        }
        
        f++;
        if (f == outer_limit / 2) {
            acc += 50; // Mid-loop block
        }
    }
    
    return acc;
}

__attribute__((noinline))
int overlapping_loop_pattern(int init) {
    int result = init;
    
    // Three sequential loops that share header/post-loop blocks
    // Loop H
    for (int h = 0; h < 10; ++h) {
        result += h * h;
        if (h == 5) {
            result -= 10;
        }
    }
    
    // Shared computation block
    result *= 2;
    
    // Loop I - shares some structure with H
    for (int i = 0; i < 10; ++i) {
        result += i * i;
        if (i == 5) {
            result -= 10;
        }
        // Extra block not in H
        result += i % 2;
    }
    
    // Another shared block
    result += 100;
    
    // Loop J - different but overlapping
    int j = 0;
    while (j < 10) {
        result += j * 3;
        j++;
        if (j == 7) {
            result += 77;
            // Nested loop K: subset of J
            for (int k = 0; k < 3; ++k) {
                result += k * j;
            }
        }
    }
    
    return result;
}

int main() {
    int seed = 42; // Could be made volatile or input-dependent
    int total = 0;
    
    // Execute all loop patterns with data dependencies
    int r1 = complex_loops_1(seed);
    total += r1;
    
    int r2 = complex_loops_2(r1);
    total += r2;
    
    int r3 = overlapping_loop_pattern(r2);
    total += r3;
    
    // Final checksum to ensure all loops contribute
    printf("Final checksum: %d\n", total);
    
    // Additional volatile operations to prevent dead code elimination
    volatile int dummy = total;
    if (dummy > 1000000) {
        printf("Unexpected large value\n");
    }
    
    return total % 256;
}
