#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // volatile to prevent constant propagation
    int acc = seed;
    
    // Loop A: Will have its own basic blocks
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            acc += i * 2;
        } else {
            acc += i;
        }
        
        // Nested loop B: Blocks are subset of Loop A's blocks
        for (int j = 0; j < 8; ++j) {
            acc += (i * j) & 0xFF;
            if (j == i % 7) break; // Early exit creates additional blocks
        }
    }
    
    // Loop C: Sequential to Loop A, shares some blocks (post-loop code)
    // but not all - partial overlap scenario
    int temp = acc;
    for (int k = 0; k < 12; ++k) {
        switch (k % 4) {
            case 0: temp += k * 3; break;
            case 1: temp -= k; break;
            case 2: temp ^= k; break;
            default: temp |= k; break;
        }
        
        // Early continue creates additional blocks
        if (k % 5 == 0) continue;
        
        temp += 1;
    }
    
    return acc + temp;
}

__attribute__((noinline))
int complex_loops_2(int base) {
    int result = base;
    volatile int outer_limit = 10;
    
    // Loop D: Outer loop with multiple exit points
    for (int x = 0; x < outer_limit; ++x) {
        if (x == 7) break; // Early break creates separate block
        
        // Loop E: Inner loop with complex condition
        int y = 0;
        while (y < 15) {
            result += (x * y) % 31;
            y += 1 + (x % 2); // Variable increment
            
            if (result > 1000) {
                result %= 1000; // Conditional inside loop
            }
        }
        
        // Loop F: Do-while with function call simulation
        int z = 0;
        do {
            // Function-like operation prevents simplification
            result ^= (z << (x % 4));
            z++;
            
            // Conditional continue
            if (z % 3 == 0) continue;
            
            result += 17;
        } while (z < 9);
    }
    
    // Loop G: Sequential to Loop D, partially overlapping blocks
    // through shared control structures
    for (int w = 0; w < 20; w += 2) {
        result += w;
        
        // Another nested loop H
        for (int v = 0; v < 5; ++v) {
            result -= v;
            if (v == w % 3) {
                result *= 2; // Creates divergent path
            }
        }
    }
    
    return result;
}

__attribute__((noinline))
int overlapping_loop_pattern(int init) {
    int sum = init;
    
    // Create three sequential loops that share some common structure
    // but have different bodies - will create partial bitmap overlaps
    
    // Loop I
    for (int a = 0; a < 25; a++) {
        sum += a * a;
        if (a % 6 == 0) {
            sum >>= 1; // Conditional block
        }
    }
    
    // Loop J: Similar structure but different iteration count
    // Shares some basic blocks with Loop I
    for (int b = 0; b < 18; b++) {
        sum += b * b;
        if (b % 4 == 0) { // Different condition
            sum >>= 2; // Different operation
        } else {
            sum += 3; // Extra else block
        }
    }
    
    // Loop K: Different structure but shares header/post blocks
    int counter = 0;
    while (counter < 14) {
        sum -= counter;
        counter += 1 + (sum % 2); // Variable increment
        
        // Nested loop L inside K
        for (int inner = 0; inner < 6; inner++) {
            sum ^= inner;
            if (inner == 3) continue;
            sum += 5;
        }
    }
    
    return sum;
}

int main() {
    // Initialize with volatile to prevent compile-time computation
    volatile int seed = 42;
    int checksum = 0;
    
    // Execute all loop patterns to force analysis
    checksum += complex_loops_1(seed);
    checksum += complex_loops_2(checksum);
    checksum += overlapping_loop_pattern(checksum);
    
    // Additional pattern: Nested loops where inner is strict subset
    int temp = 0;
    for (int outer = 0; outer < 32; outer++) {
        temp += outer;
        
        // Multiple inner loops at same level
        for (int inner1 = 0; inner1 < 8; inner1++) {
            temp += inner1 * outer;
            if (inner1 == outer % 5) break;
        }
        
        for (int inner2 = 0; inner2 < 12; inner2++) {
            temp -= inner2;
            if (inner2 % 2 == 0) continue;
            temp += 7;
        }
    }
    checksum += temp;
    
    // Final observable output
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
