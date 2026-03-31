#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) int func1(int seed);
__attribute__((noinline)) int func2(int seed);
__attribute__((noinline)) int func3(int seed);
__attribute__((noinline)) int func4(int seed);

// Use volatile to prevent constant propagation
volatile int g_bound = 32;
volatile int g_switch = 1;

// Function 1: Contains sequential loops with overlapping blocks
__attribute__((noinline)) int func1(int seed) {
    int acc1 = seed;
    int acc2 = 0;
    volatile int limit = g_bound;
    
    // First loop - will share some blocks with second loop
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            acc1 += i * 2;
            if (acc1 > 1000) break;  // Early exit creates separate block
        } else {
            acc1 -= i;
        }
        // Common block with next loop
        if (i == limit / 2) {
            acc2 = acc1;
        }
    }
    
    // Second loop at same nesting level, shares some blocks with first
    // This creates bitmap_intersect_p = true
    for (int j = 0; j < limit; ++j) {
        if (j % 4 == 0) {
            acc2 += j * 3;
            continue;
        }
        // Shared block pattern with first loop
        if (j == limit / 2) {
            acc1 = acc2;
        }
        acc2 -= j / 2;
    }
    
    // Third loop - subset of first loop's blocks
    int k = 0;
    while (k < 16) {  // Smaller bound than first loop
        acc1 += k;
        if (k % 2 == 0) {
            acc2 -= k;
        }
        k++;
        // No break/continue here - simpler block structure
    }
    
    return acc1 + acc2;
}

// Function 2: Nested loops with complex control flow
__attribute__((noinline)) int func2(int seed) {
    int total = seed;
    volatile int outer_bound = 8;
    volatile int inner_bound = 4;
    
    // Outer loop - inner loop's blocks are subset of outer's blocks
    for (int i = 0; i < outer_bound; ++i) {
        total += i * 10;
        
        // Inner loop 1 - strict subset of outer loop blocks
        // This should trigger bitmap_intersect_compl_p checks
        for (int j = 0; j < inner_bound; ++j) {
            total += j;
            if (j == 2) {
                total *= 2;
                continue;
            }
            total -= 1;
        }
        
        // Conditional that creates branching in outer loop
        switch (i % 3) {
            case 0:
                total += 100;
                break;
            case 1:
                total += 200;
                // Fall through to create different block structure
            case 2:
                total += 50;
                break;
        }
        
        // Another inner loop with different structure
        int k = 0;
        do {
            total += k * 3;
            if (k == inner_bound - 1) break;
            k++;
        } while (k < inner_bound);
        
        if (total > 5000) {
            total /= 2;
        }
    }
    
    return total;
}

// Function 3: Multiple loops with function calls inside
__attribute__((noinline)) int helper(int x, int y) {
    return x * y + (x % y);
}

__attribute__((noinline)) int func3(int seed) {
    int result = seed;
    volatile int count = 16;
    
    // Loop A
    for (int a = 0; a < count; a += 2) {
        result += helper(a, 3);
        if (a == 8) {
            result -= 50;
            continue;
        }
    }
    
    // Loop B - partially overlaps with Loop A's blocks
    for (int b = 1; b < count; b += 2) {
        result += helper(b, 2);
        // Shared computation pattern with Loop A
        if (b > count / 2) {
            result *= 2;
        }
    }
    
    // Loop C - do-while with different structure
    int c = 0;
    do {
        result += c * c;
        c++;
        if (c == 5) {
            // Nested mini-loop inside do-while
            for (int d = 0; d < 3; d++) {
                result -= d;
            }
        }
    } while (c < 10);
    
    return result;
}

// Function 4: Complex nested structure with early returns
__attribute__((noinline)) int func4(int seed) {
    int val = seed;
    volatile int max1 = 12;
    volatile int max2 = 6;
    
    // Outer loop 1
    for (int i = 0; i < max1; i++) {
        val += i * i;
        
        // Inner loop that's a candidate for hardware loops
        for (int j = 0; j < max2; j++) {
            val += j * 3;
            if (val > 10000) {
                // Early exit from inner loop
                val /= 2;
                break;
            }
        }
        
        // Another loop at same level as the inner loop
        // This creates interesting bitmap intersections
        int k = 0;
        while (k < 4) {
            val -= k * 2;
            k++;
            if (k == 2 && i == 3) {
                continue;  // Skip rest of iteration
            }
        }
        
        if (i == max1 - 1) {
            // Final iteration special case
            val += 999;
        }
    }
    
    // Sequential loop that shares exit block with previous loops
    for (int m = 0; m < 8; m++) {
        val += m * 5;
        // Conditional that creates block overlap
        if (m % 2 == 0) {
            val -= 10;
        } else {
            val += 10;
        }
    }
    
    return val;
}

int main() {
    // Initialize with volatile to prevent compile-time computation
    volatile int start = 42;
    int checksum = 0;
    
    // Execute all functions to force loop analysis
    checksum += func1(start);
    checksum += func2(checksum);
    checksum += func3(checksum);
    checksum += func4(checksum);
    
    // Additional loops in main to create more opportunities
    volatile int main_loop_bound = 20;
    int temp = 0;
    
    // Two sequential loops in main
    for (int i = 0; i < main_loop_bound; i++) {
        temp += i * i;
        if (i == main_loop_bound / 2) {
            temp /= 2;
        }
    }
    
    for (int j = 0; j < main_loop_bound / 2; j++) {
        temp -= j * 3;
        // Shared pattern with first loop
        if (j == main_loop_bound / 4) {
            temp *= 3;
        }
    }
    
    checksum += temp;
    
    // Print result to prevent dead code elimination
    printf("Final checksum: %d\n", checksum);
    
    return checksum % 256;
}
