#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) int complex_loops_1(int seed);
__attribute__((noinline)) int complex_loops_2(int seed);
__attribute__((noinline)) int nested_overlap_loops(int seed);
__attribute__((noinline)) int sequential_shared_loops(int seed);

// Global volatile to prevent optimization
volatile int g_bound = 32;

int complex_loops_1(int seed) {
    int result = seed;
    volatile int v = g_bound / 2; // Volatile to prevent constant propagation
    
    // Loop A: for loop with conditional break
    for (int i = 0; i < g_bound; ++i) {
        if (i % 7 == 0) {
            result += i * 3;
            if (i > v) break; // Early exit creates additional basic blocks
        } else {
            result -= i;
        }
        
        // Nested loop B inside A (subset relationship)
        for (int j = 0; j < 8; ++j) {
            result += (i * j) & 0xFF;
            if (j == 4) continue; // Creates additional block
        }
    }
    
    // Loop C: sequential to A, shares some blocks through control flow
    int k = 0;
    while (k < g_bound) {
        result ^= (k << 2);
        if (k % 3 == 0) {
            // Function call creates new basic blocks
            result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        k++;
    }
    
    return result;
}

int complex_loops_2(int seed) {
    int result = seed;
    volatile int limit = g_bound + 4;
    
    // Loop D: do-while with switch inside
    int counter = 0;
    do {
        switch (counter % 4) {
            case 0: result += counter * 2; break;
            case 1: result -= counter * 3; break;
            case 2: result ^= counter; break;
            case 3: result |= counter << 4; break;
        }
        
        // Loop E: nested inside D but with different bounds
        for (int m = 0; m < 16; m += 2) {
            result += (counter * m) % 256;
            if (m == 8) {
                // This creates a block that might be shared with outer loop
                result >>= 1;
            }
        }
        
        counter++;
    } while (counter < limit);
    
    return result;
}

int nested_overlap_loops(int seed) {
    int result = seed;
    
    // Outer loop F with complex control flow
    for (int x = 0; x < 24; x++) {
        // Multiple conditions create many basic blocks
        if (x % 2 == 0) {
            result += x * x;
            if (x % 6 == 0) {
                // Inner loop G: strict subset of F's blocks
                for (int y = 0; y < 12; y++) {
                    result += x * y;
                    if (y == 5) continue;
                    result -= y;
                }
            }
        } else {
            result ^= x;
        }
        
        // Another inner loop H at same nesting level as G
        // Shares some blocks with G but not all
        int z = 0;
        while (z < 8) {
            result |= (x << z);
            z++;
            if (z == 4) break;
        }
    }
    
    return result;
}

int sequential_shared_loops(int seed) {
    int result = seed;
    volatile int shared_bound = 20;
    
    // Loop I and J are sequential but share post-loop blocks
    // through compiler-generated cleanup code
    
    // Loop I: for with multiple exits
    for (int a = 0; a < shared_bound; a++) {
        result += a * 7;
        if (a == 10) {
            result >>= 2;
            continue;
        }
        if (a == 15) {
            break; // Early exit
        }
        
        // Small inner loop
        for (int b = 0; b < 3; b++) {
            result ^= b;
        }
    }
    
    // Loop J: while loop that shares some basic block structure
    // with I's post-loop code
    int c = 0;
    while (c < shared_bound) {
        if (c < 5) {
            result += c * 11;
        } else {
            result -= c * 13;
        }
        c++;
        
        // Conditional function-like macro expansion
        // creates overlapping block patterns
        #define COND_OP(val) \
            if ((val) % 2) { result += (val); } \
            else { result -= (val); }
        
        COND_OP(c);
        #undef COND_OP
    }
    
    // Loop K: Another sequential loop with overlapping
    // header block structure
    for (int d = shared_bound - 1; d >= 0; d--) {
        result = (result * 6364136223846793005ULL + 1) & 0xFFFFFFFF;
        if (d % 4 == 0) continue;
        result += d;
    }
    
    return result;
}

int main() {
    // Initialize with volatile to prevent compile-time computation
    volatile int init_seed = 0x12345678;
    int seed = init_seed;
    int checksum = 0;
    
    // Execute all loop patterns, creating data dependencies
    seed = complex_loops_1(seed);
    checksum ^= seed;
    
    seed = complex_loops_2(seed);
    checksum ^= seed;
    
    seed = nested_overlap_loops(seed);
    checksum ^= seed;
    
    seed = sequential_shared_loops(seed);
    checksum ^= seed;
    
    // Final computation to ensure all results are used
    checksum = (checksum * 1103515245 + 12345) & 0x7FFFFFFF;
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
