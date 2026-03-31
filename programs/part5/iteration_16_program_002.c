#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

// Force no inlining to keep RTL complex
static __attribute__((noinline)) 
uint64_t high_pressure_computation(const uint32_t* input) {
    // Declare many variables to create register pressure
    register uint32_t v0  = input[0]  ^ 0xAAAAAAAA;
    register uint32_t v1  = input[1]  | 0x55555555;
    register uint32_t v2  = input[2]  + 0x12345678;
    register uint32_t v3  = input[3]  - 0x87654321;
    register uint32_t v4  = input[4]  * 3;
    register uint32_t v5  = input[5]  / 7;
    register uint32_t v6  = input[6]  << 3;
    register uint32_t v7  = input[7]  >> 2;
    register uint32_t v8  = input[8]  & 0xF0F0F0F0;
    register uint32_t v9  = input[9]  | 0x0F0F0F0F;
    register uint32_t v10 = input[10] + v0;
    register uint32_t v11 = input[11] - v1;
    register uint32_t v12 = input[12] * v2;
    register uint32_t v13 = input[13] & v3;
    register uint32_t v14 = input[14] | v4;
    register uint32_t v15 = input[15] ^ v5;
    
    // Create rematerialization candidates - pure functions of inputs
    // These will have long live ranges across the loop
    uint32_t cand1 = v0 + 0x11111111;  // Pure: v0 + const
    uint32_t cand2 = v1 & 0xCCCCCCCC;  // Pure: v1 & const
    uint32_t cand3 = v2 << 2;          // Pure: v2 << const
    uint32_t cand4 = v3 >> 1;          // Pure: v3 >> const
    uint32_t cand5 = v4 ^ 0x33333333;  // Pure: v4 ^ const
    uint32_t cand6 = v5 | 0x88888888;  // Pure: v5 | const
    
    // More variables to increase pressure
    uint32_t v16 = input[16];
    uint32_t v17 = input[17];
    uint32_t v18 = input[18];
    uint32_t v19 = input[19];
    uint32_t v20 = input[20];
    uint32_t v21 = input[21];
    uint32_t v22 = input[22];
    uint32_t v23 = input[23];
    uint32_t v24 = input[24];
    uint32_t v25 = input[25];
    uint32_t v26 = input[26];
    uint32_t v27 = input[27];
    uint32_t v28 = input[28];
    uint32_t v29 = input[29];
    uint32_t v30 = input[30];
    uint32_t v31 = input[31];
    
    // Complex loop with conditional branches to create merging points
    uint64_t accumulator = 0;
    
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Use remat candidates inside loop - they're defined outside
        // This creates long live ranges that cross loop boundaries
        uint32_t tmp1 = cand1 + i;
        uint32_t tmp2 = cand2 - i;
        uint32_t tmp3 = cand3 * i;
        uint32_t tmp4 = cand4 ^ i;
        uint32_t tmp5 = cand5 | i;
        uint32_t tmp6 = cand6 & i;
        
        // Conditional branch using different sets of variables
        if (i & 1) {
            // Use one set of variables
            accumulator += v0 + v2 + v4 + v6 + v8 + v10 + v12 + v14;
            accumulator += tmp1 + tmp3 + tmp5;
            
            // More computations to keep values live
            v16 = v16 * 3 + 1;
            v18 = v18 / 5 + 2;
            v20 = (v20 << 1) | 0x1;
            v22 = v22 ^ 0xDEADBEEF;
        } else {
            // Use another set of variables
            accumulator += v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15;
            accumulator += tmp2 + tmp4 + tmp6;
            
            // Different computations
            v17 = v17 * 7 - 3;
            v19 = v19 / 11 + 4;
            v21 = (v21 >> 1) & 0x7FFFFFFF;
            v23 = v23 | 0xCAFEBABE;
        }
        
        // Nested loop to further complicate liveness
        for (int j = 0; j < 3; j++) {
            // Use all remat candidates and many variables
            uint32_t mix = (cand1 >> j) + (cand2 << j);
            mix ^= (cand3 * j) + (cand4 / (j + 1));
            mix |= (cand5 & j) ^ (cand6 | j);
            
            // Use many variables to keep them live
            accumulator += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
            accumulator += mix;
            
            // More independent computations
            v24 = v24 + v0;
            v25 = v25 - v1;
            v26 = v26 * v2;
            v27 = v27 & v3;
            v28 = v28 | v4;
            v29 = v29 ^ v5;
            v30 = v30 << 1;
            v31 = v31 >> 1;
        }
        
        // Use remat candidates in final computation before loop ends
        accumulator += cand1 + cand2 + cand3 + cand4 + cand5 + cand6;
    }
    
    // Final combination using all variables to ensure none are optimized away
    uint64_t result = accumulator;
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    result += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
    result += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
    result += cand1 + cand2 + cand3 + cand4 + cand5 + cand6;
    
    return result;
}

// Another noinline function to create more compilation context
static __attribute__((noinline))
uint64_t process_data(const uint32_t* data, size_t len) {
    uint64_t total = 0;
    
    for (size_t i = 0; i + NUM_VARS <= len; i += NUM_VARS) {
        total += high_pressure_computation(data + i);
    }
    
    return total;
}

int main() {
    // Initialize test data
    uint32_t data[1024];
    for (int i = 0; i < 1024; i++) {
        data[i] = i * 0x9E3779B9;  // Simple PRNG-like sequence
    }
    
    // Perform computation
    uint64_t result = process_data(data, 1024);
    
    // Print result to prevent optimization
    printf("Result: %llu\n", (unsigned long long)result);
    
    return 0;
}
