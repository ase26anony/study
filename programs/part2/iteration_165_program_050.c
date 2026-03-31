#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 32; // Prevent constant propagation
    int acc1 = seed;
    int acc2 = 0;
    
    // Loop A: Will have its own basic blocks
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            acc1 += i * 2;
            continue;
        }
        acc1 += i;
        
        // Nested loop B: Strict subset of A's blocks
        for (int j = 0; j < 8; ++j) {
            acc2 += j * i;
            if (j == 4) break;
        }
    }
    
    // Loop C: Sequential to A, shares some blocks
    int acc3 = 0;
    for (int i = 0; i < limit; ++i) {
        if (i % 2 == 0) {
            acc3 += i * 3;
            continue;
        }
        acc3 += i;
        
        // Early exit creates different block structure
        if (acc3 > 100) break;
    }
    
    return acc1 + acc2 + acc3;
}

__attribute__((noinline))
int overlapping_loops_2(int base) {
    volatile int n = 16;
    int sum = base;
    
    // Loop D and E will have overlapping but not identical blocks
    // due to different control flow
    
    // Loop D: with switch inside
    for (int i = 0; i < n; ++i) {
        switch (i % 4) {
            case 0: sum += i; break;
            case 1: sum += i * 2; break;
            case 2: sum += i * 3; continue;
            default: sum += i * 4;
        }
        
        // Common block with Loop E
        if (sum % 5 == 0) {
            sum += 1;
        }
    }
    
    // Loop E: Shares some blocks with D but has different structure
    for (int i = 0; i < n; ++i) {
        if (i % 3 == 0) {
            sum += i * 5;
        } else {
            sum += i * 6;
        }
        
        // Shared block with Loop D
        if (sum % 5 == 0) {
            sum += 1;
        }
        
        // Additional unique block
        if (i == n - 1) {
            sum += 100;
        }
    }
    
    return sum;
}

__attribute__((noinline))
int nested_loop_subsets(int init) {
    volatile int outer = 24;
    volatile int inner = 12;
    int total = init;
    
    // Outer loop F contains inner loop G as strict subset
    for (int x = 0; x < outer; ++x) {
        total += x * x;
        
        if (x % 6 == 0) {
            // Conditional continue creates additional blocks
            continue;
        }
        
        // Inner loop G: All blocks are subset of F's blocks
        for (int y = 0; y < inner; ++y) {
            total += x * y;
            if (y == x % inner) break;
        }
        
        // Another inner loop H at same level as G
        for (int z = 0; z < 6; ++z) {
            total -= z;
            if (total < 0) total = 0;
        }
    }
    
    // Loop I: Partially overlaps with F's blocks
    for (int x = 0; x < outer / 2; ++x) {
        total += x * 3;
        // Shares some but not all blocks with F
        if (x % 4 == 0) {
            total += 7;
        }
    }
    
    return total;
}

__attribute__((noinline))
int mixed_control_flow(int start) {
    volatile int count = 20;
    int result = start;
    int i = 0;
    
    // Loop J: do-while with complex exit
    do {
        result += i * i;
        
        if (i == 10) {
            // Nested loop K inside conditional
            for (int k = 0; k < 5; ++k) {
                result += k;
                if (k == 3) continue;
                result += 1;
            }
        }
        
        i++;
        
        // Multiple exit conditions
        if (result > 1000) break;
        if (i >= count) break;
        
    } while (1);
    
    // Loop L: while loop that shares header with J's continuation
    while (i < count * 2) {
        result -= i;
        i++;
        
        // Switch creates multiple basic blocks
        switch (result % 4) {
            case 0: result += 10; break;
            case 1: result += 20; break;
            case 2: result += 30; break;
            case 3: result += 40; break;
        }
    }
    
    return result;
}

int main() {
    // Initialize with volatile to prevent constant folding
    volatile int seed = 42;
    int checksum = 0;
    
    // Execute all loop patterns to ensure analysis
    checksum += complex_loops_1(seed);
    checksum += overlapping_loops_2(checksum);
    checksum += nested_loop_subsets(checksum);
    checksum += mixed_control_flow(checksum);
    
    // Final observable output
    printf("Final checksum: %d\n", checksum);
    
    // Additional loops in main to create more opportunities
    volatile int final_loop = 8;
    int extra = 0;
    
    // Two sequential loops that might share post-loop blocks
    for (int a = 0; a < final_loop; ++a) {
        extra += a * a;
    }
    
    for (int b = 0; b < final_loop; ++b) {
        extra -= b;
    }
    
    printf("Extra result: %d\n", extra + checksum);
    
    return 0;
}
