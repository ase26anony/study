#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

// Prevent inlining to keep RTL complex
__attribute__((noinline))
static uint32_t high_pressure_computation(const uint32_t* input) {
    // Declare many variables to create register pressure
    register uint32_t v0, v1, v2, v3, v4, v5, v6, v7;
    register uint32_t v8, v9, v10, v11, v12, v13, v14, v15;
    register uint32_t v16, v17, v18, v19, v20, v21, v22, v23;
    register uint32_t v24, v25, v26, v27, v28, v29, v30, v31;
    
    // Initialize from input array (prevents constant propagation)
    v0 = input[0]; v1 = input[1]; v2 = input[2]; v3 = input[3];
    v4 = input[4]; v5 = input[5]; v6 = input[6]; v7 = input[7];
    v8 = input[8]; v9 = input[9]; v10 = input[10]; v11 = input[11];
    v12 = input[12]; v13 = input[13]; v14 = input[14]; v15 = input[15];
    v16 = input[16]; v17 = input[17]; v18 = input[18]; v19 = input[19];
    v20 = input[20]; v21 = input[21]; v22 = input[22]; v23 = input[23];
    v24 = input[24]; v25 = input[25]; v26 = input[26]; v27 = input[27];
    v28 = input[28]; v29 = input[29]; v30 = input[30]; v31 = input[31];
    
    // Create rematerialization candidates - pure computations
    // These will have long live ranges across the loop
    uint32_t cand1 = v0 + 0x12345678;  // Simple add - cheap to recompute
    uint32_t cand2 = v1 & 0xFFFF00FF;  // Bitmask - cheap to recompute  
    uint32_t cand3 = v2 << 3;          // Shift - cheap to recompute
    uint32_t cand4 = v3 ^ 0xAAAAAAAA;  // XOR - cheap to recompute
    uint32_t cand5 = v4 + v5;          // Add of two vars - cheap to recompute
    uint32_t cand6 = v6 | 0x0000FF00;  // OR - cheap to recompute
    uint32_t cand7 = v7 - 0x11111111;  // Subtract - cheap to recompute
    uint32_t cand8 = v8 * 3;           // Multiply by small constant
    
    // Complex loop with many live values
    uint32_t sum = 0;
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Use remat candidates inside loop (defined outside)
        // This creates long live ranges that cross loop boundaries
        uint32_t t1 = cand1 + i;
        uint32_t t2 = cand2 ^ i;
        uint32_t t3 = cand3 | i;
        uint32_t t4 = cand4 & i;
        uint32_t t5 = cand5 + t1;
        uint32_t t6 = cand6 ^ t2;
        uint32_t t7 = cand7 - t3;
        uint32_t t8 = cand8 * t4;
        
        // More computations to increase register pressure
        uint32_t a = v9 + v10 + v11;
        uint32_t b = v12 * v13 - v14;
        uint32_t c = v15 << (i & 7);
        uint32_t d = v16 >> ((i + 1) & 7);
        uint32_t e = v17 & v18 | v19;
        uint32_t f = v20 ^ v21 ^ v22;
        uint32_t g = v23 + v24 * 2;
        uint32_t h = v25 - v26 / 3;
        
        // Conditional to create complex control flow
        if (i & 1) {
            // Use different sets of variables in different branches
            a = a + t1 + t5;
            b = b ^ t2 ^ t6;
            c = c | t3 | t7;
            d = d & t4 & t8;
        } else {
            // Alternative computations
            a = a - t1 - t5;
            b = b | t2 | t6;
            c = c ^ t3 ^ t7;
            d = d + t4 + t8;
        }
        
        // Use all variables to keep them live
        uint32_t mix = a + b + c + d + e + f + g + h;
        mix = mix + v27 + v28 + v29 + v30 + v31;
        
        // Final use of remat candidates in computation
        mix = mix + (cand1 & 0xFF) + (cand2 >> 8) + (cand3 << 2) + 
                     (cand4 ^ 0xCC) + (cand5 & 0xAA) + (cand6 | 0x55) +
                     (cand7 - 0x22) + (cand8 * 2);
        
        sum += mix;
        
        // Modify some variables to prevent dead code elimination
        v9 = v9 ^ mix;
        v10 = v10 + i;
        v11 = v11 - (mix >> 16);
    }
    
    // Final combination using all variables
    uint32_t result = sum;
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    result += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
    result += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
    
    // Final use of rematerialization candidates
    result = result + cand1 + cand2 + cand3 + cand4 + 
                    cand5 + cand6 + cand7 + cand8;
    
    return result;
}

// Another function with different pattern to increase complexity
__attribute__((noinline))
static uint32_t nested_loop_computation(uint32_t seed) {
    uint32_t a = seed * 0x5A5A5A5A;
    uint32_t b = seed + 0x12345678;
    uint32_t c = seed ^ 0xF0F0F0F0;
    uint32_t d = seed & 0xCCCCCCCC;
    
    // Remat candidates with different expressions
    uint32_t r1 = a << 1;
    uint32_t r2 = b >> 2;
    uint32_t r3 = c & 0x0F0F0F0F;
    uint32_t r4 = d | 0xAAAAAAAA;
    
    uint32_t total = 0;
    
    // Nested loops create complex liveness patterns
    for (int i = 0; i < 50; i++) {
        uint32_t outer_acc = 0;
        
        for (int j = 0; j < 20; j++) {
            // Use remat candidates in inner loop
            uint32_t t1 = r1 + i + j;
            uint32_t t2 = r2 ^ i ^ j;
            uint32_t t3 = r3 | (i * j);
            uint32_t t4 = r4 & (i + j * 3);
            
            // Many intermediate computations
            uint32_t x1 = a + b * i;
            uint32_t x2 = c - d / (j + 1);
            uint32_t x3 = (a ^ b) << (j & 3);
            uint32_t x4 = (c & d) >> (i & 3);
            uint32_t x5 = a * i + b * j;
            uint32_t x6 = c * j - d * i;
            uint32_t x7 = (a << 2) ^ (b >> 1);
            uint32_t x8 = (c & 0xFF) | (d & 0xFF00);
            
            // Conditional with different variable usage
            if ((i + j) & 1) {
                outer_acc += t1 + t2 + x1 + x3 + x5 + x7;
            } else {
                outer_acc += t3 + t4 + x2 + x4 + x6 + x8;
            }
            
            // Modify variables to prevent optimization
            a = a ^ outer_acc;
            b = b + j;
            c = c | (i << 8);
            d = d & ~(j << 16);
        }
        
        total += outer_acc + r1 + r2 + r3 + r4;
    }
    
    return total;
}

int main() {
    // Initialize with non-constant values
    uint32_t input[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        input[i] = i * 0x1234567 + 0x89ABCDEF;
    }
    
    uint32_t result1 = high_pressure_computation(input);
    uint32_t result2 = nested_loop_computation(result1);
    
    // Combine results to ensure all computation is used
    uint32_t final_result = result1 ^ result2;
    
    printf("Result: 0x%08X\n", final_result);
    
    // Verify with expected value (computed on first run)
    // This is just for demonstration - actual value will vary
    printf("Expected (example): 0x%08X\n", 0x8A3F7C2D);
    
    return 0;
}
