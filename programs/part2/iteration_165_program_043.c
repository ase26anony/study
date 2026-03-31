#include <stdio.h>
#include <stdlib.h>

// Prevent inlining to preserve loop structures
__attribute__((noinline, noipa))
int func1(int seed) {
    volatile int limit = 32; // Volatile to prevent constant propagation
    int sum = 0;
    
    // Outer loop - will share some blocks with inner loops
    for (int i = 0; i < limit; ++i) {
        // Conditional branch creates additional basic blocks
        if (i % 3 == 0) {
            // Inner loop 1 - subset of outer loop's blocks
            for (int j = 0; j < 16; ++j) {
                sum += (i * j) ^ seed;
                // Early continue creates another block
                if (j % 5 == 0) continue;
                sum ^= j;
            }
        } else if (i % 3 == 1) {
            // Switch statement creates multiple basic blocks
            switch (i % 4) {
                case 0: sum += i * 2; break;
                case 1: sum += i * 3; break;
                case 2: sum += i * 4; break;
                default: sum += i * 5;
            }
        } else {
            // Inner loop 2 - different structure but overlapping blocks
            int k = 0;
            while (k < 8) {
                sum += (i << k);
                if (k == 4) break; // Early exit
                k++;
            }
        }
        
        // Common post-loop block shared with other loops
        if (sum > 1000) {
            sum %= 1000;
        }
    }
    
    return sum;
}

__attribute__((noinline, noipa))
int func2(int start) {
    volatile int outer_bound = 24;
    volatile int inner_bound = 12;
    int acc = start;
    
    // Sequential loop 1 - shares header with loop 2
    for (int x = 0; x < outer_bound; ++x) {
        acc = (acc * 1103515245 + 12345) & 0x7fffffff;
        
        // Nested do-while with complex condition
        int y = 0;
        do {
            acc ^= (x << y);
            y++;
            if (y > 6) continue; // Creates additional block
            acc += y;
        } while (y < inner_bound);
        
        // Conditional break creates another block
        if (acc > 0x3fffffff) {
            acc >>= 1;
        }
    }
    
    // Sequential loop 2 - at same nesting level, shares some blocks
    for (int x = outer_bound - 1; x >= 0; --x) {
        acc += x * x;
        
        // Different inner loop structure
        for (int z = 0; z < 8; z += 2) {
            acc -= z;
            if (z == 4) {
                acc *= 2;
                // Nested if creates more blocks
                if (acc < 0) acc = -acc;
            }
        }
    }
    
    return acc;
}

__attribute__((noinline, noipa))
int func3(int base) {
    int result = base;
    volatile int count = 28;
    
    // Loop with multiple exit points
    for (int a = 0; a < count; ++a) {
        result += a * a;
        
        // Multiple nested loops with overlapping blocks
        for (int b = 0; b < a % 7 + 1; ++b) {
            result ^= b;
            for (int c = 0; c < 3; ++c) {
                result += c;
                // Function call simulation
                if (c == 1) result |= 0x1;
            }
            
            // Early continue in middle loop
            if (b % 2 == 0) continue;
            result += 0x100;
        }
        
        // Break from outer loop under condition
        if (result > 1000000) {
            result %= 1000000;
            break;
        }
    }
    
    // Another sequential loop with shared post-dominator
    int temp = 0;
    while (temp < 16) {
        result -= temp * temp;
        temp++;
        // Complex condition with && operator
        if (temp > 8 && result < 500000) {
            result += 1000;
        }
    }
    
    return result;
}

__attribute__((noinline, noipa))
int func4(int val) {
    volatile int iterations = 20;
    int modified = val;
    
    // Loop with switch inside
    for (int i = 0; i < iterations; i++) {
        switch (i % 5) {
            case 0: modified += i; break;
            case 1: modified -= i * 2; break;
            case 2: modified ^= i; break;
            case 3: modified |= i << 4; break;
            case 4: modified &= ~i; break;
        }
        
        // Inner loop that's a strict subset of outer's blocks
        for (int j = 0; j < 4; j++) {
            modified += j * j;
            if (j == 2) {
                // Nested if creates another block
                if (modified % 2 == 0) {
                    modified >>= 1;
                }
            }
        }
    }
    
    return modified;
}

int main(int argc, char *argv[]) {
    // Use argc to create input-dependent bounds
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    volatile int init = seed;
    
    // Execute all functions with data dependencies
    int result1 = func1(init);
    int result2 = func2(result1);
    int result3 = func3(result2);
    int result4 = func4(result3);
    
    // Combine results to ensure all loops contribute
    int final = result1 ^ result2 ^ result3 ^ result4;
    
    // Use result to prevent dead code elimination
    printf("Final checksum: %d\n", final);
    
    // Additional volatile store to prevent optimization
    volatile int sink = final;
    
    return final != 0 ? 0 : 1;
}
