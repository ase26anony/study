/* early-remat-test.c
 * Designed to trigger GCC's early rematerialization pass by creating
 * high register pressure with rematerialization candidates.
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
    /* Declare many distinct variables to create register pressure */
    register uint32_t v0  = input[0]  + 1;
    register uint32_t v1  = input[1]  ^ 0x55AA55AA;
    register uint32_t v2  = input[2]  * 3;
    register uint32_t v3  = input[3]  | 0x00FF00FF;
    register uint32_t v4  = input[4]  - 17;
    register uint32_t v5  = input[5]  & 0xF0F0F0F0;
    register uint32_t v6  = input[6]  + v0;
    register uint32_t v7  = input[7]  ^ v1;
    register uint32_t v8  = input[8]  * v2;
    register uint32_t v9  = input[9]  | v3;
    register uint32_t v10 = input[10] - v4;
    register uint32_t v11 = input[11] & v5;
    register uint32_t v12 = input[12] << 3;
    register uint32_t v13 = input[13] >> 2;
    register uint32_t v14 = input[14] << v0;
    register uint32_t v15 = input[15] >> v1;
    
    /* Create rematerialization candidates - pure functions of inputs */
    uint32_t cand1 = v0 + 0x12345678;  /* Can be recomputed as v0 + constant */
    uint32_t cand2 = v1 & 0xAAAAAAAA;  /* Can be recomputed as v1 & mask */
    uint32_t cand3 = v2 << 5;          /* Can be recomputed as v2 << 5 */
    uint32_t cand4 = v3 ^ 0x33333333;  /* Can be recomputed as v3 ^ constant */
    uint32_t cand5 = v4 + v5;          /* Can be recomputed as v4 + v5 */
    uint32_t cand6 = v6 * 7;           /* Can be recomputed as v6 * 7 */
    
    /* Complex control flow to extend liveness */
    uint64_t accumulator = 0;
    
    /* Outer loop - keeps many variables live */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use rematerialization candidates inside loop */
        uint32_t temp1 = cand1 + i;    /* cand1 used here, defined outside */
        uint32_t temp2 = cand2 - i;    /* cand2 used here, defined outside */
        
        /* Inner loop with conditional - creates merging points */
        for (int j = 0; j < 10; j++) {
            /* Use different sets of variables in different branches */
            if (j & 1) {
                /* Branch A uses many variables */
                uint32_t mix = v7 + v8 + v9 + v10 + temp1;
                accumulator += mix * j;
                
                /* Use more candidates */
                uint32_t tmp = cand3 | cand4;
                accumulator ^= tmp;
            } else {
                /* Branch B uses different variables */
                uint32_t mix = v11 + v12 + v13 + v14 + temp2;
                accumulator += mix * (j + 1);
                
                /* Use remaining candidates */
                uint32_t tmp = cand5 ^ cand6;
                accumulator ^= tmp;
            }
            
            /* Modify some variables to prevent dead code elimination */
            v7 ^= j;
            v8 += j;
            v9 -= j;
            v10 |= j;
        }
        
        /* Use all candidates again to keep them live across loop iterations */
        accumulator += cand1 + cand2 + cand3 + cand4 + cand5 + cand6;
        
        /* More computations to increase pressure */
        uint32_t t1 = v0 * v1 + v2;
        uint32_t t2 = v3 / (v4 + 1) + v5;
        uint32_t t3 = v6 & v7 | v8;
        uint32_t t4 = v9 ^ v10 << v11;
        uint32_t t5 = v12 + v13 * v14;
        uint32_t t6 = v15 - (v0 ^ v1);
        
        accumulator += t1 + t2 + t3 + t4 + t5 + t6;
    }
    
    /* Final use of all variables to ensure they're kept live */
    uint32_t final_mix = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
                        v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    return accumulator + final_mix;
}

/* Another function to create more complex call graph */
static __attribute__((noinline)) uint64_t
nested_pressure(const uint32_t* input, int depth)
{
    if (depth <= 0) {
        return high_pressure_computation(input);
    }
    
    /* Create local computations that add to pressure */
    uint32_t local_vars[8];
    for (int i = 0; i < 8; i++) {
        local_vars[i] = input[i] * depth + i;
    }
    
    /* Recursive call with modified input */
    uint32_t modified_input[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        modified_input[i] = input[i] ^ local_vars[i % 8];
    }
    
    uint64_t result = nested_pressure(modified_input, depth - 1);
    
    /* Use local variables after recursive call to extend liveness */
    uint32_t local_mix = 0;
    for (int i = 0; i < 8; i++) {
        local_mix += local_vars[i] * (i + 1);
    }
    
    return result + local_mix;
}

int main(void)
{
    /* Initialize with non-zero values to avoid constant propagation */
    uint32_t input_data[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        input_data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Call the high-pressure function multiple times */
    uint64_t total = 0;
    for (int run = 0; run < 3; run++) {
        /* Modify input slightly each run */
        input_data[run] ^= 0xDEADBEEF;
        
        /* Call with different depths to vary register pressure patterns */
        uint64_t result = nested_pressure(input_data, run);
        total += result;
        
        printf("Run %d: result = %llu\n", run, (unsigned long long)result);
    }
    
    printf("Total: %llu\n", (unsigned long long)total);
    return 0;
}
