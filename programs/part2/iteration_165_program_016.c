#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 32;  // Prevent constant propagation
    int sum = 0;
    
    // Loop A: Sequential loop that will share blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Loop B: Nested inside A but with early exit
        for (int j = 0; j < 16; ++j) {
            if (j == i % 8) {
                sum += j * 3;
                break;  // Creates complex control flow
            }
            sum += j;
        }
        
        // Conditional continue creates additional basic blocks
        if (i % 5 == 0) {
            continue;
        }
        sum += 1;
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int start) {
    volatile int outer_bound = 24;
    volatile int inner_bound = 12;
    int acc = start;
    
    // Loop C: Sequential to Loop D, shares some control flow
    int k = 0;
    while (k < outer_bound) {
        switch (k % 4) {
            case 0: acc += k * 2; break;
            case 1: acc += k * 3; break;
            case 2: acc += k * 4; break;
            default: acc += k; break;
        }
        
        // Loop D: Nested with subset relationship to outer while
        do {
            acc += (k % 3) * 5;
            if (acc % 7 == 0) {
                continue;  // Creates back edge
            }
            acc -= 2;
        } while (++k % inner_bound != 0);
        
        k++;
    }
    
    // Loop E: Sequential loop that overlaps with Loop C's blocks
    for (int m = 0; m < 18; ++m) {
        acc += m * m;
        // Early exit creates shared exit block
        if (m > 15 && acc > 1000) {
            break;
        }
    }
    
    return acc;
}

__attribute__((noinline))
int overlapping_control_flow(int base) {
    int result = base;
    volatile int mod = 8;
    
    // Loop F and G: Two sequential loops at same level
    // They share the function prologue/epilogue blocks
    for (int x = 0; x < 20; x += 2) {
        result += x * x;
        if (x % mod == 0) {
            result += 100;
        } else {
            result += 50;
        }
    }
    
    // Loop G: Shares some blocks with Loop F (if-else structure)
    for (int y = 1; y < 20; y += 2) {
        result -= y * y;
        if (y % mod == 0) {
            result -= 100;
        } else {
            result -= 50;
        }
    }
    
    // Loop H: Strict subset of outer control flow
    int z = 0;
    while (z < 10) {
        // Loop I: Complete subset of Loop H
        for (int w = 0; w < 5; ++w) {
            result += z * w;
            if (w == z) {
                result += 25;
                continue;
            }
        }
        
        if (z % 2 == 0) {
            result += 75;
        }
        z++;
    }
    
    return result;
}

__attribute__((noinline))
int mixed_nesting_patterns(int init) {
    int total = init;
    
    // Outer loop J
    for (int a = 0; a < 16; ++a) {
        total += a * 10;
        
        // Middle loop K - subset of J
        for (int b = 0; b < 8; ++b) {
            total += b * 5;
            
            // Innermost loop L - subset of K and J
            for (int c = 0; c < 4; ++c) {
                total += c * 2;
                if (c == b % 3) {
                    total += 15;
                    continue;
                }
            }
            
            if (b % 4 == 0) {
                total += 30;
                break;  // Creates shared exit with outer loops
            }
        }
        
        // Loop M: Sequential to K but nested in J
        int d = 0;
        while (d < 6) {
            total -= d * 3;
            d++;
            if (d == a % 5) {
                continue;  // Creates back edge
            }
        }
    }
    
    return total;
}

int main() {
    volatile int seed = 42;  // Prevent compile-time computation
    int checksum = 0;
    
    // Execute all complex loop patterns
    checksum += complex_loops_1(seed);
    checksum += complex_loops_2(checksum);
    checksum += overlapping_control_flow(checksum);
    checksum += mixed_nesting_patterns(checksum);
    
    // Additional sequential loops in main
    for (int i = 0; i < 28; ++i) {
        checksum += i * i;
        // Nested loop with partial block overlap
        for (int j = 0; j < 14; ++j) {
            checksum -= j;
            if (j == i % 7) {
                checksum += 10;
                continue;
            }
        }
    }
    
    // Final observable output
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
