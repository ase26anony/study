#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline, noipa))
int complex_loops_1(volatile int seed) {
    int sum = 0;
    volatile int limit = seed + 10;
    
    // Outer loop - will share some blocks with inner loops
    for (int i = 0; i < limit; ++i) {
        // First inner loop - subset of outer loop's blocks
        for (int j = 0; j < 8; ++j) {
            if (j % 2 == 0) {
                sum += i * j;
            } else {
                sum -= i * j;
            }
            
            // Early exit creates additional basic blocks
            if (sum > 1000) {
                break;
            }
        }
        
        // Conditional that creates shared block with next loop
        if (i % 3 == 0) {
            sum += i * 2;
        }
        
        // Second inner loop at same nesting level as first
        // Shares some blocks with first inner loop
        for (int k = 0; k < 5; ++k) {
            switch (k) {
                case 0:
                    sum += 1;
                    break;
                case 1:
                    sum += 2;
                    break;
                default:
                    sum += k;
                    if (k == 3) continue;
            }
        }
    }
    
    return sum;
}

__attribute__((noinline, noipa))
int complex_loops_2(volatile int base) {
    int acc = base;
    volatile int mod = (base % 7) + 3;
    
    // Sequential loop that shares exit block with next loop
    int m = 0;
    while (m < 16) {
        acc = (acc * 3 + m) % 100;
        if (acc < 0) acc = -acc;
        m++;
    }
    
    // Another loop at same level - shares pre-header or post-loop blocks
    do {
        for (int n = 0; n < mod; ++n) {
            acc += n * n;
            // Nested conditional creates overlapping block structure
            if (n == mod / 2) {
                acc -= 10;
                continue;
            }
        }
        mod--;
    } while (mod > 0);
    
    // Loop with switch inside - creates many basic blocks
    for (int p = 0; p < 12; ++p) {
        switch (p % 4) {
            case 0:
                acc += p;
                break;
            case 1:
                acc -= p;
                // Fall through creates different control flow
            case 2:
                acc *= 2;
                break;
            case 3:
                if (p > 6) {
                    acc /= 2;
                } else {
                    acc += 100;
                }
                break;
        }
    }
    
    return acc;
}

__attribute__((noinline, noipa))
int overlapping_nested_loops(volatile int init) {
    int total = init;
    
    // Complex nesting with partial block overlap
    for (int a = 0; a < 20; a += 2) {
        total += a;
        
        // Inner loop 1 - strict subset of outer's blocks
        int b = 0;
        while (b < 8) {
            total += b;
            if (b == 4) {
                total *= 2;
                b++;
                continue;
            }
            b++;
        }
        
        // Conditional block shared with next inner loop
        if (a % 4 == 0) {
            total -= 5;
        }
        
        // Inner loop 2 - overlaps with inner loop 1 but not identical
        for (int c = 0; c < 6; ++c) {
            total += a * c;
            if (c == 3) {
                // Early return creates different exit path
                if (total > 1000) {
                    return total;
                }
            }
        }
        
        // Another loop at same nesting level as inner loops
        int d = 5;
        do {
            total += d;
            d--;
        } while (d > 0);
    }
    
    return total;
}

__attribute__((noinline, noipa))
int sequential_loops_with_shared_blocks(int start) {
    int result = start;
    
    // First loop
    for (int x = 0; x < 15; ++x) {
        result = (result + x) & 0xFF;
        if (x == 7) {
            result ^= 0xAA;
        }
    }
    
    // Shared post-loop block
    
    // Second loop - shares some blocks with first
    int y = 0;
    while (y < 10) {
        result += y * y;
        if (result % 2 == 0) {
            result >>= 1;
        } else {
            result <<= 1;
        }
        y++;
    }
    
    // Third loop - different structure but shares header
    for (int z = 0; z < 8; ++z) {
        switch (z) {
            case 0: case 1: case 2:
                result += z * 10;
                break;
            case 3: case 4:
                result -= z * 5;
                break;
            default:
                result ^= z;
                if (z == 6) continue;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    // Use argc to create some variability
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    // Execute all loop patterns
    int sum1 = complex_loops_1(seed);
    int sum2 = complex_loops_2(sum1);
    int sum3 = overlapping_nested_loops(sum2);
    int sum4 = sequential_loops_with_shared_blocks(sum3);
    
    // Final checksum to ensure all loops contribute
    int final_result = sum1 ^ sum2 ^ sum3 ^ sum4;
    
    printf("Final checksum: %d\n", final_result);
    printf("Individual sums: %d, %d, %d, %d\n", sum1, sum2, sum3, sum4);
    
    return final_result != 0 ? 0 : 1;
}
