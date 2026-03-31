#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 40
#define LOOP_ITERATIONS 100

// Force no inlining to maintain complex RTL structure
static __attribute__((noinline)) 
uint64_t high_pressure_computation(const uint32_t* input) {
    // Declare many local variables to create register pressure
    register uint32_t v0  = input[0]  ^ 0xAAAAAAAA;
    register uint32_t v1  = input[1]  | 0x55555555;
    register uint32_t v2  = input[2]  + 0x12345678;
    register uint32_t v3  = input[3]  - 0x87654321;
    register uint32_t v4  = input[4]  * 3;
    register uint32_t v5  = input[5]  / 7;
    register uint32_t v6  = input[6]  << 3;
    register uint32_t v7  = input[7]  >> 2;
    register uint32_t v8  = input[8]  & 0xF0F0F0F0;
    register uint32_t v9  = input[9]  ^ v0;
    register uint32_t v10 = input[10] | v1;
    register uint32_t v11 = input[11] + v2;
    register uint32_t v12 = input[12] - v3;
    register uint32_t v13 = input[13] * v4;
    register uint32_t v14 = input[14] / (v5 + 1);
    register uint32_t v15 = input[15] << (v6 & 7);
    register uint32_t v16 = input[16] >> (v7 & 7);
    register uint32_t v17 = input[17] & v8;
    register uint32_t v18 = input[18] ^ v9;
    register uint32_t v19 = input[19] | v10;
    
    // Create rematerialization candidates - pure functions of inputs
    // These will have long live ranges across the loop
    uint32_t cand1 = v0 + 0x11111111;  // Cheap: v0 + constant
    uint32_t cand2 = v1 & 0xCCCCCCCC;  // Cheap: v1 & mask
    uint32_t cand3 = v2 << 2;          // Cheap: v2 << constant
    uint32_t cand4 = v3 ^ 0x33333333;  // Cheap: v3 ^ constant
    uint32_t cand5 = v4 + v5;          // Cheap: addition of two values
    uint32_t cand6 = v6 | 0x0F0F0F0F;  // Cheap: v6 | mask
    uint32_t cand7 = v7 - 100;         // Cheap: v7 - constant
    uint32_t cand8 = v8 ^ v9;          // Cheap: XOR of two values
    
    // Complex nested loop to create complicated liveness patterns
    uint64_t accumulator = 0;
    
    // Outer loop - uses candidates defined outside
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Inner loop with conditional branching
        for (int j = 0; j < 5; j++) {
            // Use rematerialization candidates inside loops
            // This forces them to be live across loop iterations
            uint32_t tmp1 = cand1 + (v10 * j);
            uint32_t tmp2 = cand2 & (v11 + i);
            uint32_t tmp3 = cand3 << (j & 3);
            uint32_t tmp4 = cand4 ^ (v12 >> 1);
            
            // Conditional branch creates merging points
            if (j & 1) {
                // Use different sets of variables in each branch
                uint32_t branch_val = tmp1 + tmp2 + v13 + v14;
                accumulator += branch_val * cand5;
            } else {
                uint32_t branch_val = tmp3 + tmp4 + v15 + v16;
                accumulator += branch_val * cand6;
            }
            
            // More computations to increase pressure
            uint32_t mix1 = v17 * v18 + cand7;
            uint32_t mix2 = v19 * v0 - cand8;
            accumulator += (mix1 ^ mix2);
            
            // Force all candidates to be used somewhere
            if (i & 1) {
                accumulator += cand1;
            } else {
                accumulator += cand2;
            }
            
            // Additional computations with remaining variables
            v0 = v0 ^ (v1 + j);
            v1 = v1 | (v2 - i);
            v2 = v2 + (v3 * 3);
            v3 = v3 - (v4 / 2);
        }
        
        // Use candidates after inner loop too
        accumulator += cand3 * cand4;
        accumulator += cand5 | cand6;
        accumulator += cand7 ^ cand8;
        
        // More independent computations
        uint32_t comp1 = v5 * v6 + v7;
        uint32_t comp2 = v8 / (v9 + 1) * v10;
        uint32_t comp3 = v11 << (v12 & 3);
        uint32_t comp4 = v13 >> (v14 & 3);
        uint32_t comp5 = v15 & v16 | v17;
        uint32_t comp6 = v18 ^ v19 + v0;
        
        accumulator += comp1 + comp2 + comp3 + comp4 + comp5 + comp6;
    }
    
    // Final use of all candidates to ensure they stay live
    uint64_t final_mix = cand1;
    final_mix *= cand2;
    final_mix += cand3;
    final_mix ^= cand4;
    final_mix |= cand5;
    final_mix &= cand6;
    final_mix += cand7;
    final_mix ^= cand8;
    
    return accumulator + final_mix;
}

// Another noinline function to create more pressure
static __attribute__((noinline))
uint64_t secondary_computation(uint32_t seed) {
    // More variables to increase pressure
    uint32_t a = seed * 0x5A5A5A5A;
    uint32_t b = seed + 0x12345678;
    uint32_t c = seed ^ 0xF0F0F0F0;
    uint32_t d = seed | 0x0F0F0F0F;
    uint32_t e = seed & 0xCCCCCCCC;
    uint32_t f = seed << 3;
    uint32_t g = seed >> 2;
    uint32_t h = seed * 3;
    uint32_t i = seed / 5;
    uint32_t j = seed + a;
    uint32_t k = b ^ c;
    uint32_t l = d | e;
    uint32_t m = f & g;
    uint32_t n = h * i;
    uint32_t o = j - k;
    uint32_t p = l + m;
    uint32_t q = n ^ o;
    uint32_t r = p | q;
    
    // Rematerialization candidates
    uint32_t cand9 = a + 0x11111111;
    uint32_t cand10 = b & 0x22222222;
    uint32_t cand11 = c << 1;
    uint32_t cand12 = d ^ 0x33333333;
    
    uint64_t result = 0;
    for (int i = 0; i < 50; i++) {
        // Use candidates in loop
        result += cand9 * i;
        result += cand10 & i;
        result += cand11 << (i & 3);
        result += cand12 ^ i;
        
        // More computations
        result += a * b + c - d;
        result ^= e | f & g;
        result += h / (i + 1) * j;
        result ^= k << (l & 7);
        result += m - n + o;
        result |= p ^ q & r;
        
        // Modify variables
        a = a ^ b;
        b = b + c;
        c = c - d;
        d = d * 2;
    }
    
    return result;
}

int main() {
    // Initialize input array with deterministic values
    uint32_t input[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        input[i] = i * 0x1234567 + 0x89ABCDEF;
    }
    
    // Call high-pressure function
    uint64_t result1 = high_pressure_computation(input);
    
    // Call secondary function to create more overall pressure
    uint64_t result2 = secondary_computation(input[0]);
    
    // Combine results deterministically
    uint64_t final_result = result1 ^ result2;
    
    printf("Result: 0x%016llX\n", (unsigned long long)final_result);
    
    // Expected output for verification (computed on x86-64 with -O2)
    // This ensures the computation isn't optimized away
    if (final_result == 0xFFFFFFFFFFFFFFFFULL) {
        printf("Unexpected result - all bits set\n");
    }
    
    return 0;
}
