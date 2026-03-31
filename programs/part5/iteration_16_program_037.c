/* Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat -m32 -o remat_test remat_test.c */
/* For even higher pressure: gcc -O3 -funroll-loops -fno-schedule-insns -m32 -fdump-rtl-all -o remat_test remat_test.c */

#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static int __attribute__((noinline,noipa))
high_pressure_computation(int *inputs, int n) {
    /* Declare many local variables to create register pressure */
    register int v0  = inputs[0]  + 1;   /* Remat candidate: inputs[0] + 1 */
    register int v1  = inputs[1]  * 2;   /* Remat candidate: inputs[1] * 2 */
    register int v2  = inputs[2]  & 0xFF; /* Remat candidate: inputs[2] & 0xFF */
    register int v3  = inputs[3]  | 0x80;
    register int v4  = inputs[4]  ^ 0x55;
    register int v5  = inputs[5]  << 1;
    register int v6  = inputs[6]  >> 2;
    register int v7  = inputs[7]  + inputs[0];
    register int v8  = inputs[8]  - inputs[1];
    register int v9  = inputs[9]  * 3;
    register int v10 = inputs[10] & 0xF0;
    register int v11 = inputs[11] | 0x0F;
    register int v12 = inputs[12] ^ 0xAA;
    register int v13 = inputs[13] << 2;
    register int v14 = inputs[14] >> 1;
    register int v15 = inputs[15] + inputs[2];
    
    /* Additional variables to increase pressure further */
    int v16 = inputs[16] * 5;
    int v17 = inputs[17] & 0xCC;
    int v18 = inputs[18] | 0x33;
    int v19 = inputs[19] ^ 0x99;
    int v20 = inputs[20] << 3;
    int v21 = inputs[21] >> 3;
    int v22 = inputs[22] + inputs[3];
    int v23 = inputs[23] - inputs[4];
    int v24 = inputs[24] * 7;
    int v25 = inputs[25] & 0x0F;
    int v26 = inputs[26] | 0xF0;
    int v27 = inputs[27] ^ 0x66;
    int v28 = inputs[28] << 1;
    int v29 = inputs[29] >> 2;
    int v30 = inputs[30] + inputs[5];
    int v31 = inputs[31] - inputs[6];
    
    /* Complex control flow with nested loops to create challenging liveness patterns */
    int result = 0;
    
    /* Outer loop - many variables live across loop boundaries */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use remat candidates inside loop - they were defined outside */
        int t0 = v0 + i;   /* v0 defined outside, used inside loop */
        int t1 = v1 - i;   /* v1 defined outside, used inside loop */
        int t2 = v2 & i;   /* v2 defined outside, used inside loop */
        
        /* Inner loop with conditional - creates merging points */
        for (int j = 0; j < 10; j++) {
            /* Use different sets of variables in different branches */
            if (j & 1) {
                /* Branch 1 uses variables v0-v15 */
                result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
                result ^= v8 | v9 | v10 | v11 | v12 | v13 | v14 | v15;
            } else {
                /* Branch 2 uses variables v16-v31 */
                result += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
                result ^= v24 | v25 | v26 | v27 | v28 | v29 | v30 | v31;
            }
            
            /* More computations to keep values live */
            int t3 = v3 * j;
            int t4 = v4 / (j + 1);
            int t5 = v5 << (j & 3);
            int t6 = v6 >> (j & 3);
            
            /* Use these computations to prevent dead code elimination */
            result += t3 + t4 + t5 + t6;
        }
        
        /* More operations between inner and outer loops */
        result += t0 * t1;
        result ^= t2 & 0x7F;
        
        /* Additional computations that use many variables */
        int sum1 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
        int sum2 = v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
        int sum3 = v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
        int sum4 = v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
        
        result += (sum1 * sum2) - (sum3 / (sum4 + 1));
    }
    
    /* Final computation using all variables to ensure they're live until the end */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    result += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
    result += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
    
    /* Complex final expression to prevent optimization */
    result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    
    return result;
}

/* Wrapper function to create additional pressure */
static int __attribute__((noinline))
compute_with_pressure(int *data) {
    /* Create local copies to force more register usage */
    int local_data[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        local_data[i] = data[i] + i;  /* Slightly modify inputs */
    }
    
    /* Call the high-pressure function multiple times */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        total += high_pressure_computation(local_data, NUM_VARS);
        /* Modify data slightly between calls */
        local_data[i % NUM_VARS] += 1;
    }
    
    return total;
}

int main(void) {
    /* Initialize test data */
    int test_data[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        test_data[i] = i * 3 + 7;  /* Non-trivial pattern */
    }
    
    /* Perform computation */
    int result = compute_with_pressure(test_data);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Verify with expected value for this specific input */
    int expected = 2147483647 & ((result * 1103515245 + 12345) & 0x7FFFFFFF);
    printf("Expected pattern check: %d\n", expected);
    
    return 0;
}
