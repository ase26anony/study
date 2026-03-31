/* early-remat-test.c
 * Designed to trigger early rematerialization in GCC's RTL optimizer
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat early-remat-test.c -o early_remat_test
 */

#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static __attribute__((noinline)) uint64_t 
high_pressure_computation(const uint32_t* input) 
{
    /* Many distinct local variables to create register pressure */
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
    register uint32_t v14 = input[14] / v5;
    register uint32_t v15 = input[15] << v6;
    register uint32_t v16 = input[16] >> v7;
    register uint32_t v17 = input[17] & v8;
    register uint32_t v18 = input[18] ^ v9;
    register uint32_t v19 = input[19] | v10;
    register uint32_t v20 = input[20] + v11;
    register uint32_t v21 = input[21] - v12;
    register uint32_t v22 = input[22] * v13;
    register uint32_t v23 = input[23] / v14;
    register uint32_t v24 = input[24] << v15;
    register uint32_t v25 = input[25] >> v16;
    register uint32_t v26 = input[26] & v17;
    register uint32_t v27 = input[27] ^ v18;
    register uint32_t v28 = input[28] | v19;
    register uint32_t v29 = input[29] + v20;
    register uint32_t v30 = input[30] - v21;
    register uint32_t v31 = input[31] * v22;
    
    /* Rematerialization candidates - pure functions of inputs */
    uint32_t cand1 = v0 + 0x11111111;  /* Cheap to recompute */
    uint32_t cand2 = v1 & 0x0F0F0F0F;  /* Cheap to recompute */
    uint32_t cand3 = v2 << 2;          /* Cheap to recompute */
    uint32_t cand4 = v3 ^ 0x33333333;  /* Cheap to recompute */
    uint32_t cand5 = v4 | 0xCCCCCCCC;  /* Cheap to recompute */
    
    uint64_t accumulator = 0;
    
    /* Complex loop with conditional branches to create merging points */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use rematerialization candidates inside loop */
        uint32_t temp1 = cand1 + (i & 0xFF);
        uint32_t temp2 = cand2 - (i >> 8);
        uint32_t temp3 = cand3 ^ i;
        uint32_t temp4 = cand4 | (i * 3);
        uint32_t temp5 = cand5 & (i + 0x100);
        
        /* Conditional branch using different sets of variables */
        if (i % 3 == 0) {
            /* Path A: uses variables v0-v15 and candidates */
            accumulator += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 +
                          v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
                          temp1 + temp2;
        } else if (i % 3 == 1) {
            /* Path B: uses variables v16-v31 and candidates */
            accumulator += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23 +
                          v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31 +
                          temp3 + temp4;
        } else {
            /* Path C: mixes variables from both sets */
            accumulator += v0 + v16 + v31 + v15 +
                          temp1 + temp3 + temp5;
        }
        
        /* Nested loop to further complicate liveness analysis */
        for (int j = 0; j < 3; j++) {
            /* Use all candidates in nested scope */
            uint32_t nested_temp = cand1 + cand2 + cand3 + cand4 + cand5;
            accumulator += nested_temp * j;
            
            /* More computations to increase pressure */
            if (j % 2 == 0) {
                accumulator += v0 * v16 + v1 * v17;
            } else {
                accumulator += v31 / (v15 + 1);
            }
        }
        
        /* Modify some variables to prevent dead code elimination */
        v0 ^= i;
        v16 += i;
        v31 -= i;
    }
    
    /* Final computation using all variables and candidates */
    uint64_t final_result = accumulator;
    final_result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    final_result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    final_result += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
    final_result += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
    final_result += cand1 + cand2 + cand3 + cand4 + cand5;
    
    return final_result;
}

/* Another function to create additional pressure context */
static __attribute__((noinline)) uint64_t
secondary_computation(uint64_t seed) 
{
    /* More variables to increase overall pressure */
    uint64_t a = seed * 0x5DEECE66D;
    uint64_t b = seed + 0xBL;
    uint64_t c = seed ^ 0xFFFFFFFF;
    uint64_t d = seed | 0xAAAAAAAA;
    uint64_t e = seed & 0x55555555;
    uint64_t f = seed << 13;
    uint64_t g = seed >> 17;
    uint64_t h = seed * 7;
    uint64_t i = seed / 11;
    uint64_t j = seed % 19;
    
    /* Keep these live across many operations */
    uint64_t cand_a = a + 0x1234;
    uint64_t cand_b = b & 0xF0F0;
    uint64_t cand_c = c << 3;
    
    uint64_t result = 0;
    
    for (int k = 0; k < 50; k++) {
        /* Use candidates inside loop */
        result += cand_a * k;
        result += cand_b - k;
        result += cand_c ^ k;
        
        /* Complex conditional */
        if (k % 4 == 0) {
            result += a + b + c;
        } else if (k % 4 == 1) {
            result += d + e + f;
        } else if (k % 4 == 2) {
            result += g + h + i;
        } else {
            result += j + cand_a + cand_b;
        }
    }
    
    return result;
}

int main() 
{
    /* Initialize input data */
    uint32_t input_data[NUM_VARS];
    
    for (int i = 0; i < NUM_VARS; i++) {
        input_data[i] = i * 0x1234567 + 0x89ABCDEF;
    }
    
    /* Perform high-pressure computation */
    uint64_t result1 = high_pressure_computation(input_data);
    
    /* Perform secondary computation to create more context */
    uint64_t result2 = secondary_computation(result1);
    
    /* Combine results deterministically */
    uint64_t final_result = result1 ^ result2;
    
    printf("Result: %llu\n", (unsigned long long)final_result);
    
    /* Also use the result in a way that prevents optimization */
    volatile uint64_t volatile_result = final_result;
    
    return (volatile_result > 1000000) ? 0 : 1;
}
