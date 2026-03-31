#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(volatile int seed) {
    int sum = 0;
    
    // Outer loop with complex control flow
    for (int i = 0; i < (seed % 32 + 8); ++i) {
        // First inner loop - will be subset of outer loop's blocks
        for (int j = 0; j < 16; ++j) {
            if (j % 3 == 0) {
                sum += i * j;
                continue;
            }
            if (j == 7) {
                sum -= i;
                break;
            }
            sum += j;
        }
        
        // Conditional block that creates partial overlap
        if (i % 2 == 0) {
            // Sequential loop at same level with shared blocks
            int k = 0;
            while (k < 8) {
                sum += (i + k) * 2;
                if (k == 4) {
                    sum /= 2;
                    continue;
                }
                k++;
            }
        } else {
            // Different path with do-while
            int m = 0;
            do {
                sum += m * i;
                m++;
            } while (m < 5);
        }
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(volatile int base, int prev_result) {
    int acc = prev_result;
    
    // Multiple sequential loops with overlapping blocks
    for (int x = 0; x < base + 10; ++x) {
        // Shared header-like computation
        int temp = x * 2;
        if (temp > 20) {
            acc += temp;
            continue;
        }
        acc -= x;
    }
    
    // Second sequential loop sharing some blocks
    int y = 0;
    while (y < 12) {
        // This block overlaps with previous loop's structure
        if (y % 3 == 0) {
            acc += y * 3;
            y += 2;
            continue;
        }
        acc -= y;
        y++;
    }
    
    // Nested loop where inner is strict subset
    for (int outer = 0; outer < 15; ++outer) {
        acc += outer;
        
        // Inner loop - complete subset of outer's blocks
        for (int inner = 0; inner < 8; ++inner) {
            if (inner == outer % 3) {
                acc *= 2;
                break;
            }
            acc += inner;
        }
        
        // Switch inside loop creates more basic blocks
        switch (outer % 4) {
            case 0: acc += 100; break;
            case 1: acc -= 50; break;
            case 2: acc *= 3; break;
            default: acc /= 2; break;
        }
    }
    
    return acc;
}

__attribute__((noinline))
int overlapping_loop_patterns(volatile int param) {
    int result = 0;
    
    // Three sequential loops with partial block overlap
    // Loop A
    for (int a = 0; a < param + 5; ++a) {
        result += a * a;
        if (a == 3) {
            result -= 10;
            continue;
        }
        if (a > 10) break;
    }
    
    // Loop B - shares some but not all blocks with A
    int b = 0;
    while (b < 8) {
        result += b * 3;
        // This if-else structure creates different block partitioning
        if (b % 2 == 0) {
            result += 7;
        } else {
            result -= 2;
            if (b == 5) {
                b++;
                continue;
            }
        }
        b++;
    }
    
    // Loop C - intersects with both A and B differently
    do {
        result /= 2;
        if (result < 0) result = 0;
        
        // Nested loop inside do-while
        for (int c = 0; c < 6; ++c) {
            result += c + param;
            if (c == 2 || c == 4) {
                result *= -1;
                continue;
            }
        }
    } while (result < 1000);
    
    return result;
}

int main() {
    volatile int seed = 42;  // Volatile to prevent constant propagation
    int checksum = 0;
    
    // Execute loops with data dependencies
    int r1 = complex_loops_1(seed);
    checksum += r1;
    
    int r2 = complex_loops_2(seed, r1);
    checksum += r2;
    
    int r3 = overlapping_loop_patterns(seed + r2 % 17);
    checksum += r3;
    
    // Additional loop to create more analysis opportunities
    volatile int limit = 25;
    for (int final_i = 0; final_i < limit; ++final_i) {
        // Loop with early exit that creates interesting block structure
        if (final_i > checksum % 20) {
            checksum *= (final_i + 1);
            break;
        }
        
        // Very inner loop - strict subset
        for (int final_j = 0; final_j < 7; ++final_j) {
            checksum += final_i * final_j;
            if (final_j == 3) continue;
            if (final_j == 5) break;
        }
        
        // Another loop at same level
        int k = 0;
        while (k < 4) {
            checksum -= k;
            k++;
            if (k == 2 && final_i % 2 == 0) {
                checksum += 100;
                continue;
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
