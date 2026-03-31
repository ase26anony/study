#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(volatile int seed) {
    int sum = 0;
    
    // Outer loop with complex control flow
    for (int i = 0; i < seed + 10; ++i) {
        // First inner loop - will be subset of outer loop's blocks
        for (int j = 0; j < 8; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
            } else {
                sum -= i;
            }
        }
        
        // Conditional break creates additional basic blocks
        if (i > seed) {
            break;
        }
        
        // Second inner loop at same nesting level but different structure
        int k = 0;
        while (k < 5) {
            sum += (i + k) * 2;
            if (k == 3) continue;
            k++;
        }
    }
    
    // Sequential loop sharing some blocks with previous loop's exit
    for (int m = 0; m < seed + 5; ++m) {
        switch (m % 3) {
            case 0: sum += m * 2; break;
            case 1: sum -= m; break;
            case 2: sum += 100; break;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int overlapping_loops_2(volatile int limit) {
    int acc = 0;
    volatile int cond = limit;
    
    // Two sequential loops that will share the function's entry/exit blocks
    // First loop with early exit
    for (int a = 0; a < cond + 15; ++a) {
        acc += a * 3;
        if (a > cond) {
            acc += 1000;
            break;
        }
        
        // Nested loop that's a strict subset
        for (int b = 0; b < 4; ++b) {
            acc += (a + b) * (b + 1);
            if (b == 2) continue;
        }
    }
    
    // Second sequential loop - partially overlaps in block structure
    int c = 0;
    do {
        acc -= c * 2;
        c++;
        
        // Another nested loop with different structure
        if (c % 2 == 0) {
            for (int d = 0; d < 3; ++d) {
                acc += d * d;
                if (d == 1) break;
            }
        }
    } while (c < cond + 8);
    
    return acc;
}

__attribute__((noinline))
int nested_subset_loops_3(int base) {
    int total = base;
    
    // Outer loop with multiple exit points
    for (int x = 0; x < 32; ++x) {
        // Multiple inner loops at same level
        for (int y = 0; y < 16; ++y) {
            total += x * y;
            if (y > 8) {
                total -= 50;
                continue;
            }
        }
        
        // Another inner loop that shares some blocks
        for (int z = 0; z < 12; ++z) {
            total += z;
            if (z == x % 10) break;
        }
        
        // Conditional function-like block
        if (x % 7 == 0) {
            for (int w = 0; w < 6; ++w) {
                total += w * w;
            }
        } else if (x % 3 == 0) {
            total += 777;
        }
    }
    
    // Loop with switch inside
    for (int s = 0; s < 24; ++s) {
        switch (s % 4) {
            case 0: total += s * 3; break;
            case 1: total += s + 10; 
                    for (int t = 0; t < 2; ++t) {
                        total += t * 5;
                    }
                    break;
            case 2: continue;
            case 3: total -= s * 2; break;
        }
    }
    
    return total;
}

__attribute__((noinline))
int interleaved_loops_4(volatile int param) {
    int result = param;
    
    // Three loops at same level with overlapping control flow
    for (int i = 0; i < param + 12; ++i) {
        result += i * i;
        
        // Inner loop that can be optimized as hardware loop
        for (int j = 0; j < 16; ++j) {
            result += j;
            if (j == 7) result += 100;
        }
    }
    
    // Loop with different induction variable
    for (int k = param; k < param + 20; k += 2) {
        result -= k * 3;
        
        // Another nested countable loop
        for (int l = 0; l < 8; l += 1) {
            result += (k + l) * 2;
            if (l == 4) continue;
        }
    }
    
    // Do-while loop to create different block structure
    int m = 0;
    do {
        result += m * 5;
        m++;
        
        // Conditional nested loop
        if (m % 3 == 0) {
            for (int n = 0; n < 6; ++n) {
                result -= n;
            }
        }
    } while (m < param + 10);
    
    return result;
}

int main(int argc, char *argv[]) {
    // Use volatile and argc to prevent compile-time computation
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    // Execute all loop patterns with data dependencies
    int sum1 = complex_loops_1(seed);
    int sum2 = overlapping_loops_2(seed + 1);
    int sum3 = nested_subset_loops_3(sum1 + sum2);
    int sum4 = interleaved_loops_4(seed * 2);
    
    // Create complex checksum with all results
    int final_result = sum1 * 3 + sum2 * 7 - sum3 * 2 + sum4 * 5;
    
    // Force observable output
    printf("Loop analysis test result: %d\n", final_result);
    
    // Additional volatile store to prevent dead code elimination
    volatile int sink = final_result;
    
    return final_result > 0 ? 0 : 1;
}
