#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static int __attribute__((noinline)) 
high_pressure_computation(int *input) 
{
    /* Declare many distinct variables to create register pressure */
    register int v0  = input[0]  + 1;
    register int v1  = input[1]  * 2;
    register int v2  = input[2]  & 0xFF;
    register int v3  = input[3]  | 0x100;
    register int v4  = input[4]  ^ 0x55;
    register int v5  = input[5]  << 1;
    register int v6  = input[6]  >> 2;
    register int v7  = input[7]  + 0x7F;
    register int v8  = input[8]  - 42;
    register int v9  = input[9]  * 3;
    register int v10 = input[10] & 0xF0;
    register int v11 = input[11] | 0x0F;
    register int v12 = input[12] ^ 0xAA;
    register int v13 = input[13] << 2;
    register int v14 = input[14] >> 1;
    register int v15 = input[15] + 99;
    
    /* Additional variables to further increase pressure */
    int v16 = input[16] * 5;
    int v17 = input[17] & 0x33;
    int v18 = input[18] | 0xCC;
    int v19 = input[19] ^ 0x66;
    int v20 = input[20] << 3;
    int v21 = input[21] >> 3;
    int v22 = input[22] + 255;
    int v23 = input[23] - 128;
    int v24 = input[24] * 7;
    int v25 = input[25] & 0x0F;
    int v26 = input[26] | 0xF0;
    int v27 = input[27] ^ 0x99;
    int v28 = input[28] << 4;
    int v29 = input[29] >> 4;
    int v30 = input[30] + 512;
    int v31 = input[31] - 256;
    
    /* Create rematerialization candidates - pure functions kept live across loop */
    int cand1 = v0 + v1;      /* Cheap to recompute: v0 + v1 */
    int cand2 = v2 & v3;      /* Cheap to recompute: v2 & v3 */
    int cand3 = v4 ^ v5;      /* Cheap to recompute: v4 ^ v5 */
    int cand4 = v6 | v7;      /* Cheap to recompute: v6 | v7 */
    int cand5 = v8 * 2;       /* Cheap to recompute: v8 * 2 */
    
    /* Complex loop with conditional branches to create merging points */
    int sum = 0;
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use rematerialization candidates inside loop - their defs are outside */
        if (i & 1) {
            /* Branch 1: Use some candidates and variables */
            sum += cand1 + cand2 + v9 + v10 + v11;
            sum += (v12 << (i & 3)) + (v13 >> (i & 3));
        } else {
            /* Branch 2: Use different candidates and variables */
            sum += cand3 + cand4 + cand5 + v14 + v15;
            sum += (v16 * (i & 7)) - (v17 & (i & 0xF));
        }
        
        /* More computations keeping many values live */
        int t0 = v18 + v19 + v20;
        int t1 = v21 * v22 - v23;
        int t2 = (v24 << 1) | (v25 >> 1);
        int t3 = v26 ^ v27 ^ v28;
        int t4 = v29 + v30 * v31;
        
        /* Conditional inside loop using different variable sets */
        if ((i % 3) == 0) {
            sum += t0 + t1 + cand1;  /* cand1 used again */
        } else if ((i % 3) == 1) {
            sum += t2 + t3 + cand3;  /* cand3 used again */
        } else {
            sum += t4 + cand5;       /* cand5 used again */
        }
        
        /* More independent computations to prevent reuse */
        v0 = v0 + (v1 & 0x1);
        v2 = v2 ^ (v3 << 1);
        v4 = v4 | (v5 >> 1);
        v6 = v6 - (v7 & 0x2);
        v8 = v8 + (v9 * 2);
        v10 = v10 & (v11 | 0x1);
        v12 = v12 ^ (v13 + 1);
        v14 = v14 | (v15 - 1);
    }
    
    /* Final computation using all variables to ensure they stay live */
    int result = cand1 + cand2 + cand3 + cand4 + cand5;
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    result += v18 + v19 + v20 + v21 + v22 + v23 + v24 + v25;
    result += v26 + v27 + v28 + v29 + v30 + v31;
    result += sum;
    
    return result;
}

/* Another layer to increase optimization complexity */
static int __attribute__((noinline))
nested_pressure(int *input, int iterations) 
{
    int total = 0;
    for (int j = 0; j < iterations; j++) {
        /* Modify input slightly each iteration to prevent loop invariant removal */
        int modified_input[NUM_VARS];
        for (int k = 0; k < NUM_VARS; k++) {
            modified_input[k] = input[k] + (j & 0xF);
        }
        
        /* Call high-pressure function */
        total += high_pressure_computation(modified_input);
        
        /* Additional computations between calls */
        int temp = 0;
        for (int m = 0; m < 8; m++) {
            temp += modified_input[m] * (j + m);
        }
        total ^= temp;
    }
    return total;
}

int main(void) 
{
    /* Initialize with non-zero values to prevent constant propagation */
    int input[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        input[i] = (i * 13 + 7) & 0xFFF;  /* Pattern prevents optimization */
    }
    
    /* Perform computation */
    int result = nested_pressure(input, 5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify with simple computation */
    int check = 0;
    for (int i = 0; i < NUM_VARS; i++) {
        check += input[i];
    }
    printf("Input sum: %d\n", check);
    
    return 0;
}
