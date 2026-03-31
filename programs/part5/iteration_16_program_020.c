#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

// Force no inlining to maintain complex RTL structure
__attribute__((noinline))
static uint32_t high_pressure_computation(const uint32_t* input) {
    // Declare many variables to create register pressure
    register uint32_t v0  = input[0];
    register uint32_t v1  = input[1];
    register uint32_t v2  = input[2];
    register uint32_t v3  = input[3];
    register uint32_t v4  = input[4];
    register uint32_t v5  = input[5];
    register uint32_t v6  = input[6];
    register uint32_t v7  = input[7];
    register uint32_t v8  = input[8];
    register uint32_t v9  = input[9];
    register uint32_t v10 = input[10];
    register uint32_t v11 = input[11];
    register uint32_t v12 = input[12];
    register uint32_t v13 = input[13];
    register uint32_t v14 = input[14];
    register uint32_t v15 = input[15];
    
    // Create rematerialization candidates - pure computations
    // These will have long live ranges and be cheap to recompute
    uint32_t r0 = v0 + 0x12345678;      // Candidate 1
    uint32_t r1 = v1 & 0xFFFF0000;      // Candidate 2
    uint32_t r2 = v2 << 3;              // Candidate 3
    uint32_t r3 = v3 ^ 0xAAAAAAAA;      // Candidate 4
    uint32_t r4 = v4 | 0x0000FFFF;      // Candidate 5
    uint32_t r5 = v5 * 7;               // Candidate 6
    uint32_t r6 = v6 - 0x11111111;      // Candidate 7
    uint32_t r7 = v7 + v8;              // Candidate 8
    
    // Keep these candidates live by using them in the loop
    // but define them outside to create cross-block liveness
    
    // Complex loop with many live values
    uint32_t accumulator = 0;
    
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Use all remat candidates inside the loop
        // This creates pressure as they need to stay live
        uint32_t temp = 0;
        
        // Branch to create complex control flow
        if (i & 1) {
            // Use one set of variables
            temp = r0 + r1 + r2;
            temp ^= v9 + i;
            temp *= r3;
            temp += v10;
        } else {
            // Use different set to create merging points
            temp = r4 + r5 + r6;
            temp ^= v11 + i;
            temp *= r7;
            temp += v12;
        }
        
        // More computations to increase pressure
        uint32_t t1 = v13 * v14 + i;
        uint32_t t2 = v15 ^ (v0 << (i & 7));
        uint32_t t3 = v1 + v2 * v3;
        uint32_t t4 = v4 | v5 & v6;
        uint32_t t5 = v7 - v8 + v9;
        uint32_t t6 = v10 ^ v11 ^ v12;
        uint32_t t7 = v13 << 2;
        uint32_t t8 = v14 >> 1;
        uint32_t t9 = v15 * 3;
        
        // Use remat candidates again
        accumulator += temp + r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
        accumulator ^= t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
        
        // Modify some variables to prevent CSE
        v0 += i;
        v1 ^= i;
        v2 *= (i & 0xF) + 1;
    }
    
    // Final computation using all remat candidates
    // Ensures they must stay live until the end
    uint32_t final = accumulator;
    final += r0 * 2;
    final ^= r1 << 1;
    final += r2 >> 2;
    final ^= r3 & 0x55555555;
    final += r4 | 0x33333333;
    final ^= r5 * 3;
    final += r6 - 0x22222222;
    final ^= r7 + 0x44444444;
    
    // Use all input variables one more time
    final += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    final ^= v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    return final;
}

// Another layer to increase optimization complexity
__attribute__((noinline))
static uint32_t nested_pressure(const uint32_t* input, int depth) {
    if (depth <= 0) {
        return high_pressure_computation(input);
    }
    
    // Create more intermediate values
    uint32_t intermediate[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        intermediate[i] = input[i] ^ depth;
    }
    
    uint32_t result = 0;
    for (int j = 0; j < 3; j++) {
        result ^= high_pressure_computation(intermediate);
        
        // Modify intermediate values
        for (int i = 0; i < NUM_VARS; i++) {
            intermediate[i] += j * 0x12345;
        }
    }
    
    return result;
}

int main() {
    // Initialize with non-zero values to prevent constant folding
    uint32_t input_data[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        input_data[i] = (i * 0x1234567) ^ 0x89ABCDEF;
    }
    
    // Call the high-pressure function
    uint32_t result = nested_pressure(input_data, 2);
    
    // Print result to prevent dead code elimination
    printf("Result: 0x%08X\n", result);
    
    // Verify with a simple computation
    uint32_t check = 0;
    for (int i = 0; i < NUM_VARS; i++) {
        check ^= input_data[i] + i;
    }
    printf("Check: 0x%08X\n", check);
    
    return 0;
}
