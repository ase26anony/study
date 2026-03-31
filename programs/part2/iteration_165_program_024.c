#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 32; // Volatile to prevent constant propagation
    int sum = 0;
    
    // Loop A: Will have its own basic blocks
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 3 == 1) {
            sum += i * 3;
        } else {
            // Nested loop B: Strict subset of A's blocks
            for (int j = 0; j < 8; ++j) {
                sum += (i + j) * seed;
                if (j == 4) break; // Early exit creates additional blocks
            }
        }
        
        // Function call inside loop
        if (i == limit / 2) {
            sum += rand() % 10; // External call
        }
    }
    
    // Loop C: Sequential to A, shares some blocks (post-loop code)
    // but has different internal structure
    int k = 0;
    while (k < 16) {
        switch (k % 4) {
            case 0: sum += k * 5; break;
            case 1: sum += k * 7; continue; // Skip increment
            case 2: sum += k * 11; break;
            default: sum += k * 13;
        }
        k++;
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int base) {
    int total = base;
    volatile int outer = 24;
    
    // Loop D: Outer loop with multiple exit points
    for (int x = 0; x < outer; ++x) {
        if (x == outer - 1) break; // Early exit block
        
        // Loop E: Inner loop with continue
        for (int y = 0; y < 12; ++y) {
            if (y % 2 == 0) continue;
            total += x * y;
            
            // Loop F: Innermost - strict subset of E's blocks
            for (int z = 0; z < 4; ++z) {
                total += (x + y + z) * 2;
                if (z == 2) goto inner_exit; // Complicated control flow
            }
            inner_exit:
            if (y == 10) return total; // Early return from function
        }
        
        // Loop G: At same level as E but different structure
        int w = 0;
        do {
            total += w * 3;
            w++;
            if (w > 6) break;
        } while (w < 8);
    }
    
    return total;
}

__attribute__((noinline))
int overlapping_loops(int init) {
    int result = init;
    volatile int count = 20;
    
    // Loop H and I: Sequential loops that share header/post blocks
    // but have different internal block structure
    
    // Loop H
    for (int a = 0; a < count; a += 2) {
        result += a * a;
        if (a > count / 2) {
            result -= 5;
        }
    }
    
    // Shared basic block here (function epilogue)
    int temp = result;
    
    // Loop I: Different iteration pattern but shares some blocks
    for (int b = 1; b < count; b += 3) {
        result += b * 3;
        if (b % 4 == 0) {
            result += temp;
        } else {
            result -= 2;
        }
    }
    
    // Loop J: Partially overlaps with H and I's blocks
    int c = 0;
    while (c < 10) {
        result += c * 7;
        c++;
        if (c == 5) {
            // This block is unique to J
            result += 100;
        }
    }
    
    return result;
}

int main() {
    int seed = 42;
    int checksum = 0;
    
    // Force compiler to analyze all loops with data dependencies
    checksum += complex_loops_1(seed);
    checksum += complex_loops_2(checksum);
    checksum += overlapping_loops(checksum);
    
    // Additional sequential loops at same nesting level
    volatile int iterations = 16;
    for (int i = 0; i < iterations; ++i) {
        checksum += i * i;
        // Loop K: Nested but with early continue
        for (int j = 0; j < 8; ++j) {
            if (j == i % 4) continue;
            checksum += j * 3;
        }
    }
    
    // Loop L: Another sequential loop
    int m = 0;
    while (m < 12) {
        checksum += m * 5;
        m++;
        if (m == 6) {
            // Loop M: Nested inside L but not in previous loops
            for (int n = 0; n < 4; ++n) {
                checksum += n * 11;
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
