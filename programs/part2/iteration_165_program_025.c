#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline)) 
int complex_loops_1(int seed) {
    volatile int limit = 16; // volatile to prevent optimization
    int sum = 0;
    
    // Loop A: Sequential loop that will share blocks with Loop B
    for (int i = 0; i < limit; ++i) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Loop B: Nested loop with subset relationship to Loop A
        for (int j = 0; j < 8; ++j) {
            if (j % 2 == 0) {
                sum += (i + j) * 3;
            } else {
                continue; // Creates additional basic blocks
            }
            
            // Small inner loop creating more block complexity
            int k = 0;
            do {
                sum += k;
                k++;
            } while (k < 4);
        }
        
        // Early exit creates separate basic block
        if (sum > 1000) {
            break;
        }
    }
    
    // Loop C: Sequential to Loop A, partially overlapping blocks
    int count = 0;
    while (count < 10) {
        switch (count % 3) {
            case 0: sum += count * 5; break;
            case 1: sum += count * 3; break;
            default: sum += count; break;
        }
        count++;
    }
    
    return sum;
}

__attribute__((noinline))
int complex_loops_2(int base) {
    int result = base;
    volatile int outer = 12;
    
    // Loop D: Outer loop containing multiple inner loops
    for (int x = 0; x < outer; x++) {
        // Loop E: Inner loop with simple counter
        for (int y = 0; y < 24; y++) {
            result += x * y;
            
            // Conditional continue creates block divergence
            if (y % 4 == 0) {
                continue;
            }
            result -= 1;
        }
        
        // Loop F: Another inner loop at same nesting level as E
        int z = 0;
        while (z < 16) {
            result += z * 2;
            z++;
            
            // Nested switch for block complexity
            switch (z % 4) {
                case 0: result += 10; break;
                case 1: result += 20; break;
                case 2: result += 30; break;
                case 3: result += 40; break;
            }
        }
        
        // Loop G: do-while with early break
        int w = 0;
        do {
            if (w == 5) {
                break;
            }
            result += w * 3;
            w++;
        } while (w < 8);
    }
    
    return result;
}

__attribute__((noinline))
int overlapping_loop_patterns(int init) {
    int acc = init;
    volatile int mod = 7;
    
    // Loop H and Loop I: Sequential loops sharing some control flow
    // They will have partially overlapping block bitmaps
    
    // Loop H
    for (int a = 0; a < 20; a++) {
        if (a % mod == 0) {
            acc += a * 7;
            // Function call creates separate basic block
            acc += complex_loops_1(0) % 100;
        } else {
            acc += a * 3;
        }
    }
    
    // Shared basic block region
    int temp = acc;
    
    // Loop I - shares some blocks with Loop H but not all
    for (int b = 5; b < 25; b++) {
        if (b % mod == 0) {
            acc += b * 7;
            acc += complex_loops_1(0) % 100;
        } else if (b % 2 == 0) {
            acc += b * 2;
            // Additional else-if creates different block structure
        } else {
            acc += b;
        }
        
        // Different termination condition
        if (acc - temp > 500) {
            break;
        }
    }
    
    // Loop J: Strict subset of Loop K
    // Loop K (outer)
    for (int c = 0; c < 15; c++) {
        acc += c * 10;
        
        // Loop J (inner) - strict subset
        for (int d = 0; d < 8; d++) {
            acc += d * 5;
            if (d % 3 == 0) {
                acc += 1;
            }
        }
        
        // Additional code in outer loop only
        acc += complex_loops_2(0) % 50;
    }
    
    return acc;
}

int main() {
    volatile int seed = 42; // volatile to prevent constant propagation
    
    printf("Starting complex loop patterns...\n");
    
    // Execute all loop patterns with data dependencies
    int result1 = complex_loops_1(seed);
    printf("Result 1: %d\n", result1);
    
    int result2 = complex_loops_2(result1);
    printf("Result 2: %d\n", result2);
    
    int final_result = overlapping_loop_patterns(result2);
    printf("Final checksum: %d\n", final_result);
    
    // Additional loop at main level to ensure hw-doloop sees top-level loops
    int checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += i * i;
        if (i % 8 == 0) {
            checksum += final_result % 100;
        }
    }
    
    printf("Total checksum: %d\n", checksum + final_result);
    
    return 0;
}
