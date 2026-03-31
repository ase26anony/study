#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(volatile int seed) {
    int sum = 0;
    volatile int limit = seed + 10;
    
    // Loop A: Will share some blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
            continue;  // Creates additional basic block
        }
        
        // Nested loop inside A - subset of A's blocks
        for (int j = 0; j < 5; ++j) {
            sum += j;
            if (j == 3) break;  // Early exit creates more blocks
        }
        
        if (i == limit - 1) {
            sum *= 2;  // Final iteration special case
        }
    }
    
    // Loop B: Sequential to A, shares some common blocks
    int k = 0;
    while (k < limit) {
        sum += k;
        if (k % 2 == 0) {
            // Another nested loop - creates subset relationship
            for (int m = 0; m < 3; ++m) {
                sum -= m;
                if (m == 1) continue;
                sum += 1;
            }
        }
        k++;
    }
    
    return sum;
}

__attribute__((noinline))
int overlapping_loops_2(volatile int base) {
    int acc = base;
    volatile int mod = base % 7 + 5;
    
    // Loop C: do-while with switch inside
    int c = 0;
    do {
        switch (c % 4) {
            case 0: acc += c * 3; break;
            case 1: acc -= c; break;
            case 2: acc *= 2; break;
            case 3: acc /= (c + 1); break;
        }
        
        // Inner loop that's a strict subset of do-while blocks
        for (int d = 0; d < 8; d += 2) {
            acc += d;
            if (d == 4) {
                acc += 100;  // Creates conditional block
                continue;
            }
        }
        
        c++;
    } while (c < mod);
    
    // Loop D: Sequential to C, partially overlapping blocks
    for (int e = 0; e < 16; ++e) {
        if (e < mod) {
            // This block overlaps with Loop C's range
            acc += e * e;
        } else {
            acc -= e;
        }
        
        // Another nested loop with early exit
        int f = 0;
        while (f < 3) {
            acc += f;
            f++;
            if (acc > 1000) break;
        }
    }
    
    return acc;
}

__attribute__((noinline))
int nested_loop_patterns(volatile int init) {
    int total = init;
    
    // Outer loop E
    for (int g = 0; g < 12; ++g) {
        total += g;
        
        // Multiple inner loops at same level within E
        // Loop F: subset of E's blocks
        for (int h = 0; h < 6; ++h) {
            total += h * h;
            if (h == 2 || h == 4) {
                total -= 5;  // Conditional creates more blocks
            }
        }
        
        // Loop G: another subset of E's blocks
        int p = 0;
        while (p < 4) {
            total += p * 3;
            p++;
            if (p == 2) continue;
            total += 1;
        }
        
        // Loop H: yet another subset with different structure
        for (int q = 0; q < 8; q += 2) {
            switch (q) {
                case 0: total += 10; break;
                case 2: total += 20; break;
                case 4: total += 30; break;
                case 6: total += 40; break;
            }
        }
    }
    
    // Loop I: Sequential to E, partially overlapping
    int r = 0;
    volatile int bound = 8;
    while (r < bound) {
        total += r * r * r;
        
        // Conditional that creates shared block structure
        if (r % 3 == 0) {
            // Loop J: nested in I but shares structure with F
            for (int s = 0; s < 5; ++s) {
                total += s * 2;
                if (s == 3) break;
            }
        } else {
            total -= r;
        }
        
        r++;
    }
    
    return total;
}

int main() {
    volatile int seed = 15;  // Volatile to prevent constant propagation
    int checksum = 0;
    
    // Execute all loop patterns with data dependencies
    checksum += complex_loops_1(seed);
    checksum += overlapping_loops_2(checksum % 20);
    checksum += nested_loop_patterns(checksum % 30);
    
    // Additional sequential loops in main
    volatile int iter = 10;
    
    // Loop K: Simple countable loop
    for (int t = 0; t < iter; ++t) {
        checksum += t * t;
    }
    
    // Loop L: Another sequential loop
    int u = 0;
    while (u < 15) {
        checksum -= u;
        
        // Loop M: Nested in L
        for (int v = 0; v < 4; ++v) {
            checksum += v * 3;
            if (v == 2) continue;
            checksum += 1;
        }
        
        u++;
    }
    
    // Final observable output
    printf("Final checksum: %d\n", checksum);
    
    // Use result to prevent dead code elimination
    if (checksum > 1000) {
        return 0;
    } else {
        return 1;
    }
}
