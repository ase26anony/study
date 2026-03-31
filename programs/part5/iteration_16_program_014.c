/* Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat -o early_remat_test early_remat_test.c */
/* For even higher pressure: add -m32 or -fno-schedule-insns */

#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 40
#define LOOP_ITERATIONS 100

/* Prevent inlining to keep RTL complex */
static __attribute__((noinline,noipa))
uint64_t high_pressure_computation(const int32_t* input) {
    /* Declare many distinct variables to create register pressure */
    register int32_t v0  = input[0]  + 1;
    register int32_t v1  = input[1]  - 2;
    register int32_t v2  = input[2]  * 3;
    register int32_t v3  = input[3]  / 4;
    register int32_t v4  = input[4]  | 0x55;
    register int32_t v5  = input[5]  & 0xAA;
    register int32_t v6  = input[6]  ^ 0xFF;
    register int32_t v7  = input[7]  << 1;
    register int32_t v8  = input[8]  >> 2;
    register int32_t v9  = input[9]  + 0x10;
    register int32_t v10 = input[10] - 0x20;
    register int32_t v11 = input[11] * 5;
    register int32_t v12 = input[12] / 6;
    register int32_t v13 = input[13] | 0x33;
    register int32_t v14 = input[14] & 0xCC;
    register int32_t v15 = input[15] ^ 0x99;
    register int32_t v16 = input[16] << 3;
    register int32_t v17 = input[17] >> 4;
    register int32_t v18 = input[18] + 0x30;
    register int32_t v19 = input[19] - 0x40;
    
    /* Create rematerialization candidates - pure functions of inputs */
    /* These will have long live ranges and be cheap to recompute */
    int32_t remat_candidate1 = v0 + v1;      /* Pure: v0 + v1 */
    int32_t remat_candidate2 = v2 & 0x7F;    /* Pure: v2 & 0x7F */
    int32_t remat_candidate3 = v3 << 2;      /* Pure: v3 << 2 */
    int32_t remat_candidate4 = v4 ^ v5;      /* Pure: v4 ^ v5 */
    int32_t remat_candidate5 = v6 | 0x0F;    /* Pure: v6 | 0x0F */
    int32_t remat_candidate6 = v7 - 10;      /* Pure: v7 - 10 */
    int32_t remat_candidate7 = v8 * 3;       /* Pure: v8 * 3 */
    int32_t remat_candidate8 = v9 / 2;       /* Pure: v9 / 2 */
    
    /* More variables to increase pressure */
    int32_t v20 = v10 + v11;
    int32_t v21 = v12 - v13;
    int32_t v22 = v14 * v15;
    int32_t v23 = v16 / (v17 + 1);
    int32_t v24 = v18 | v19;
    int32_t v25 = v20 & v21;
    int32_t v26 = v22 ^ v23;
    int32_t v27 = v24 << (v25 & 3);
    int32_t v28 = v26 >> (v27 & 3);
    int32_t v29 = v0 + v28;
    int32_t v30 = v1 - v27;
    int32_t v31 = v2 * v26;
    int32_t v32 = v3 / (v25 + 1);
    int32_t v33 = v4 | v24;
    int32_t v34 = v5 & v23;
    int32_t v35 = v6 ^ v22;
    int32_t v36 = v7 << (v21 & 3);
    int32_t v37 = v8 >> (v20 & 3);
    int32_t v38 = v9 + v19;
    int32_t v39 = v10 - v18;
    
    /* Complex control flow to create merging points with many live values */
    uint64_t accumulator = 0;
    
    /* Outer loop - remat candidates defined outside, used inside */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use rematerialization candidates inside loop */
        int32_t temp1 = remat_candidate1 + i;
        int32_t temp2 = remat_candidate2 - i;
        int32_t temp3 = remat_candidate3 * (i + 1);
        int32_t temp4 = remat_candidate4 & i;
        
        /* Inner loop with conditional branches */
        for (int j = 0; j < 5; j++) {
            /* Conditional that uses different sets of variables */
            if (j & 1) {
                /* Branch 1: uses first set of variables */
                accumulator += v0 + v1 + v2 + v3 + temp1;
                accumulator += v4 + v5 + v6 + v7 + temp2;
                accumulator += remat_candidate5 + remat_candidate6;
            } else {
                /* Branch 2: uses second set of variables */
                accumulator += v8 + v9 + v10 + v11 + temp3;
                accumulator += v12 + v13 + v14 + v15 + temp4;
                accumulator += remat_candidate7 + remat_candidate8;
            }
            
            /* More computations to keep variables live */
            if (i & 1) {
                accumulator += v16 + v17 + v18 + v19;
                accumulator += v20 + v21 + v22 + v23;
            } else {
                accumulator += v24 + v25 + v26 + v27;
                accumulator += v28 + v29 + v30 + v31;
            }
            
            /* Use all remat candidates to ensure they stay live */
            accumulator += (remat_candidate1 >> (j & 3)) +
                          (remat_candidate2 << (j & 3)) +
                          (remat_candidate3 & (0xFF >> j)) +
                          (remat_candidate4 | (j * 0x11)) +
                          (remat_candidate5 ^ (j * 0x22)) +
                          (remat_candidate6 + (j * 10)) +
                          (remat_candidate7 - (j * 5)) +
                          (remat_candidate8 * (j + 1));
        }
        
        /* Use many variables across loop iterations to extend live ranges */
        accumulator += v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39;
        
        /* Force recomputation by modifying inputs slightly */
        if (i & 2) {
            accumulator += (v0 << 1) + (v1 >> 1) + (v2 & 0xAA) + (v3 | 0x55);
        }
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    uint64_t final_result = accumulator;
    final_result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    final_result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    final_result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    final_result += v30 + v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39;
    final_result += remat_candidate1 + remat_candidate2 + remat_candidate3 + remat_candidate4;
    final_result += remat_candidate5 + remat_candidate6 + remat_candidate7 + remat_candidate8;
    
    return final_result;
}

/* Another high-pressure function with different pattern */
static __attribute__((noinline,noipa))
uint64_t nested_pressure_computation(const int32_t* input) {
    int32_t vals[NUM_VARS];
    
    /* Initialize with diverse computations */
    for (int i = 0; i < NUM_VARS; i++) {
        vals[i] = input[i % 20] + (i * 37) ^ 0xABCD;
    }
    
    /* Create many intermediate values with long live ranges */
    int32_t inter1 = vals[0] * vals[1] + vals[2];
    int32_t inter2 = vals[3] / (vals[4] + 1) | vals[5];
    int32_t inter3 = vals[6] << (vals[7] & 7);
    int32_t inter4 = vals[8] >> (vals[9] & 7);
    int32_t inter5 = vals[10] & vals[11] ^ vals[12];
    int32_t inter6 = vals[13] | vals[14] + vals[15];
    int32_t inter7 = vals[16] - vals[17] * vals[18];
    int32_t inter8 = vals[19] + vals[0] / (vals[1] + 1);
    
    uint64_t result = 0;
    
    /* Complex loop structure */
    for (int outer = 0; outer < 50; outer++) {
        /* Use intermediates defined outside loop */
        int32_t tmp = inter1 + inter2 + inter3 + inter4;
        
        for (int inner = 0; inner < 10; inner++) {
            /* Conditional with many live values */
            if ((outer + inner) & 1) {
                result += tmp + vals[inner % NUM_VARS] + inter5;
                result += inter6 + vals[(inner + 5) % NUM_VARS];
            } else {
                result += tmp - vals[inner % NUM_VARS] + inter7;
                result += inter8 + vals[(inner + 10) % NUM_VARS];
            }
            
            /* More computations using all intermediates */
            result += (inter1 >> (inner & 3)) +
                     (inter2 << (inner & 3)) +
                     (inter3 & (0xFF >> inner)) +
                     (inter4 | (inner * 0x11)) +
                     (inter5 ^ (inner * 0x22)) +
                     (inter6 + (inner * 7)) +
                     (inter7 - (inner * 3)) +
                     (inter8 * (inner + 2));
        }
        
        /* Modify intermediates to force recomputation considerations */
        if (outer & 3) {
            inter1 += vals[outer % NUM_VARS];
            inter3 ^= vals[(outer + 1) % NUM_VARS];
        }
    }
    
    return result;
}

int main() {
    /* Initialize with non-zero values to avoid constant propagation */
    int32_t input_data[20];
    for (int i = 0; i < 20; i++) {
        input_data[i] = (i * 12345 + 6789) & 0xFFFF;
    }
    
    /* Call high-pressure functions */
    uint64_t result1 = high_pressure_computation(input_data);
    uint64_t result2 = nested_pressure_computation(input_data);
    
    /* Use results to prevent optimization */
    uint64_t final_result = result1 + result2;
    
    printf("Result: %llu\n", (unsigned long long)final_result);
    
    return 0;
}
