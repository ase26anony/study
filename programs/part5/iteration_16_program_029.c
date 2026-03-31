#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

// Prevent inlining to maintain complex RTL structure
__attribute__((noinline))
static uint32_t high_pressure_computation(uint32_t *inputs) {
    // Declare many variables to create register pressure
    register uint32_t v0, v1, v2, v3, v4, v5, v6, v7;
    register uint32_t v8, v9, v10, v11, v12, v13, v14, v15;
    register uint32_t v16, v17, v18, v19, v20, v21, v22, v23;
    register uint32_t v24, v25, v26, v27, v28, v29, v30, v31;
    
    // Initialize from input array (prevents constant propagation)
    v0 = inputs[0] ^ 0x12345678;
    v1 = inputs[1] + 0x87654321;
    v2 = inputs[2] * 0x55555555;
    v3 = inputs[3] | 0xAAAAAAAA;
    v4 = inputs[4] & 0xCCCCCCCC;
    v5 = inputs[5] ^ 0xF0F0F0F0;
    v6 = inputs[6] + 0x0F0F0F0F;
    v7 = inputs[7] * 0x33333333;
    v8 = inputs[8] | 0xCCCCCCCC;
    v9 = inputs[9] & 0xAAAAAAAA;
    v10 = inputs[10] ^ 0x55555555;
    v11 = inputs[11] + 0x11111111;
    v12 = inputs[12] * 0x99999999;
    v13 = inputs[13] | 0x66666666;
    v14 = inputs[14] & 0x99999999;
    v15 = inputs[15] ^ 0x66666666;
    v16 = inputs[16] + 0x33333333;
    v17 = inputs[17] * 0xCCCCCCCC;
    v18 = inputs[18] | 0x33333333;
    v19 = inputs[19] & 0xCCCCCCCC;
    v20 = inputs[20] ^ 0x99999999;
    v21 = inputs[21] + 0x66666666;
    v22 = inputs[22] * 0x33333333;
    v23 = inputs[23] | 0xCCCCCCCC;
    v24 = inputs[24] & 0x33333333;
    v25 = inputs[25] ^ 0xCCCCCCCC;
    v26 = inputs[26] + 0x99999999;
    v27 = inputs[27] * 0x66666666;
    v28 = inputs[28] | 0x99999999;
    v29 = inputs[29] & 0x66666666;
    v30 = inputs[30] ^ 0x33333333;
    v31 = inputs[31] + 0xCCCCCCCC;
    
    // Create rematerialization candidates - pure functions of inputs
    // These will have long live ranges across the loop
    uint32_t cand1 = v0 + v1;  // Candidate 1: v0 + v1
    uint32_t cand2 = v2 & v3;  // Candidate 2: v2 & v3
    uint32_t cand3 = v4 ^ v5;  // Candidate 3: v4 ^ v5
    uint32_t cand4 = v6 | v7;  // Candidate 4: v6 | v7
    uint32_t cand5 = v8 + v9;  // Candidate 5: v8 + v9
    uint32_t cand6 = v10 & v11; // Candidate 6: v10 & v11
    uint32_t cand7 = v12 ^ v13; // Candidate 7: v12 ^ v13
    uint32_t cand8 = v14 | v15; // Candidate 8: v14 | v15
    
    // Complex loop with many live values
    uint32_t accumulator = 0;
    
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Use rematerialization candidates inside loop
        // They were defined outside, creating cross-block liveness
        uint32_t t1 = cand1 + (v16 << (i & 7));
        uint32_t t2 = cand2 ^ (v17 >> ((i + 1) & 7));
        uint32_t t3 = cand3 & (v18 + i);
        uint32_t t4 = cand4 | (v19 - i);
        uint32_t t5 = cand5 + (v20 * (i + 1));
        uint32_t t6 = cand6 ^ (v21 & (0xFF + i));
        uint32_t t7 = cand7 & (v22 | (0xAA + i));
        uint32_t t8 = cand8 | (v23 ^ (0x55 + i));
        
        // More computations to increase pressure
        uint32_t a = v24 + v25 + t1;
        uint32_t b = v26 ^ v27 ^ t2;
        uint32_t c = v28 & v29 & t3;
        uint32_t d = v30 | v31 | t4;
        uint32_t e = t5 + t6 + a;
        uint32_t f = t7 ^ t8 ^ b;
        uint32_t g = a & c & d;
        uint32_t h = b | e | f;
        
        // Conditional to create merging points
        if (i & 1) {
            accumulator += a + c + e + g;
        } else {
            accumulator += b + d + f + h;
        }
        
        // Modify some variables to prevent dead code elimination
        v0 ^= t1;
        v1 += t2;
        v2 &= t3;
        v3 |= t4;
        v4 ^= t5;
        v5 += t6;
        v6 &= t7;
        v7 |= t8;
    }
    
    // Final computation using all variables to ensure they're live
    uint32_t result = accumulator;
    result ^= v0 + v1 + v2 + v3;
    result ^= v4 | v5 | v6 | v7;
    result ^= v8 & v9 & v10 & v11;
    result ^= v12 ^ v13 ^ v14 ^ v15;
    result ^= v16 * v17 * v18 * v19;
    result ^= v20 + v21 + v22 + v23;
    result ^= v24 | v25 | v26 | v27;
    result ^= v28 & v29 & v30 & v31;
    
    // Also use remat candidates one last time
    result += cand1 + cand2 + cand3 + cand4;
    result += cand5 + cand6 + cand7 + cand8;
    
    return result;
}

// Another function to create more pressure through nested calls
__attribute__((noinline))
static uint32_t nested_pressure(uint32_t *inputs, int depth) {
    if (depth <= 0) {
        return high_pressure_computation(inputs);
    }
    
    // Create local variables that will be live across the call
    uint32_t local1 = inputs[depth] * 0x12345678;
    uint32_t local2 = inputs[depth + 1] ^ 0x87654321;
    uint32_t local3 = inputs[depth + 2] + 0x55555555;
    uint32_t local4 = inputs[depth + 3] & 0xAAAAAAAA;
    
    uint32_t result = nested_pressure(inputs, depth - 1);
    
    // Use locals after recursive call (creates live ranges across calls)
    result ^= local1 + local2;
    result |= local3 & local4;
    
    return result;
}

int main() {
    // Initialize with non-constant values to prevent optimization
    uint32_t inputs[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = (i * 0x1234567) ^ 0x89ABCDEF;
    }
    
    // Call the high-pressure function
    uint32_t result = nested_pressure(inputs, 3);
    
    // Print result to prevent dead code elimination
    printf("Result: 0x%08X\n", result);
    
    // Verify with expected value (computed from known inputs)
    // This is just for demonstration - actual value depends on implementation
    uint32_t expected = 0;
    for (int i = 0; i < NUM_VARS; i++) {
        expected ^= inputs[i];
    }
    printf("Check: 0x%08X\n", expected);
    
    return 0;
}
