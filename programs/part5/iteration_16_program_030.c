#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent optimization */
static __attribute__((noinline)) unsigned long long
high_pressure_computation(int *inputs) {
    /* Declare many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    
    /* Initialize from inputs to avoid constant propagation */
    v0 = inputs[0] + 1;
    v1 = inputs[1] * 2;
    v2 = inputs[2] | 0xFF;
    v3 = inputs[3] ^ 0x55;
    v4 = inputs[4] << 1;
    v5 = inputs[5] >> 2;
    v6 = inputs[6] + inputs[7];
    v7 = inputs[8] - inputs[9];
    v8 = inputs[10] & 0xF0;
    v9 = inputs[11] | 0x0F;
    v10 = inputs[12] * 3;
    v11 = inputs[13] / 2;
    v12 = inputs[14] ^ inputs[15];
    v13 = inputs[16] + 0x100;
    v14 = inputs[17] - 50;
    v15 = inputs[18] << 2;
    v16 = inputs[19] >> 1;
    v17 = inputs[20] & 0xAA;
    v18 = inputs[21] | 0x55;
    v19 = inputs[22] * 5;
    v20 = inputs[23] + 999;
    v21 = inputs[24] ^ 0x33;
    v22 = inputs[25] << 3;
    v23 = inputs[26] >> 4;
    v24 = inputs[27] + inputs[28];
    v25 = inputs[29] - inputs[30];
    v26 = inputs[31] & 0xCC;
    v27 = inputs[0] | 0x11;
    v28 = inputs[1] * 7;
    v29 = inputs[2] + 0x200;
    
    /* Create rematerialization candidates - pure functions of inputs */
    /* These will have long live ranges across the loop */
    int cand1 = v0 + v1;      /* Cheap: addition */
    int cand2 = v2 & 0x7F;    /* Cheap: bitwise AND with constant */
    int cand3 = v3 << 1;      /* Cheap: shift left */
    int cand4 = v4 ^ 0xAA;    /* Cheap: XOR with constant */
    int cand5 = v5 + 42;      /* Cheap: addition with constant */
    int cand6 = v6 | 0x01;    /* Cheap: bitwise OR */
    int cand7 = v7 - 10;      /* Cheap: subtraction */
    int cand8 = v8 >> 1;      /* Cheap: shift right */
    
    /* Complex nested loop to create control flow and extend liveness */
    unsigned long long result = 0;
    
    /* Outer loop - uses candidates defined outside */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Inner loop with conditional - creates merging points */
        for (int j = 0; j < 10; j++) {
            /* Use remat candidates inside loops (defined outside) */
            /* This forces them to be kept live or rematerialized */
            int temp1 = cand1 + i;
            int temp2 = cand2 - j;
            int temp3 = cand3 * i;
            int temp4 = cand4 ^ j;
            
            /* Conditional that uses different variable sets */
            if ((i + j) & 1) {
                /* Branch 1: use first set of variables */
                result += v9 + v10 + v11 + temp1;
                result += v12 ^ v13 ^ temp2;
                result += v14 * v15 * temp3;
                result += v16 | v17 | temp4;
            } else {
                /* Branch 2: use second set of variables */
                result += v18 + v19 + v20 + cand5;
                result += v21 ^ v22 ^ cand6;
                result += v23 * v24 * cand7;
                result += v25 | v26 | cand8;
            }
            
            /* More computations to keep many values live */
            int t1 = v27 + v28;
            int t2 = v29 * v0;
            int t3 = v1 & v2;
            int t4 = v3 | v4;
            int t5 = v5 ^ v6;
            int t6 = v7 + v8;
            int t7 = v9 - v10;
            int t8 = v11 << 1;
            int t9 = v12 >> 2;
            int t10 = v13 & 0x0F;
            
            /* Use all these temporaries to prevent dead code elimination */
            result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
            
            /* Use the remat candidates again */
            result += cand1 * cand2;
            result += cand3 | cand4;
            result += cand5 ^ cand6;
            result += cand7 - cand8;
        }
        
        /* More variable usage outside inner loop */
        int outer1 = v0 * i;
        int outer2 = v1 + i;
        int outer3 = v2 ^ i;
        int outer4 = v3 - i;
        
        result += outer1 + outer2 + outer3 + outer4;
        
        /* Use remat candidates in outer loop too */
        result += cand1 << (i & 3);
        result += cand2 >> (i & 3);
        result += cand3 + (i * 2);
        result += cand4 - (i * 3);
    }
    
    /* Final computation using all variables and candidates */
    /* This ensures everything stays live until the end */
    unsigned long long final = 0;
    final += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    final += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    final += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    final += cand1 + cand2 + cand3 + cand4 + cand5 + cand6 + cand7 + cand8;
    
    return result ^ final;  /* Combine both results */
}

/* Wrapper to add another layer of function call */
static __attribute__((noinline)) unsigned long long
compute_hash(int *data, int size) {
    unsigned long long hash = 0;
    
    /* Process data in chunks to create more register pressure */
    for (int i = 0; i < size; i += NUM_VARS) {
        int chunk[NUM_VARS];
        for (int j = 0; j < NUM_VARS && (i + j) < size; j++) {
            chunk[j] = data[i + j];
        }
        
        /* Call high-pressure function */
        hash += high_pressure_computation(chunk);
        
        /* Additional computation to prevent optimization */
        hash = (hash << 5) | (hash >> 59);  /* Rotate */
    }
    
    return hash;
}

int main(void) {
    /* Create input data */
    const int data_size = 1000;
    int *data = (int*)malloc(data_size * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < data_size; i++) {
        data[i] = (i * 12345 + 6789) & 0xFFFF;
    }
    
    /* Perform computation */
    unsigned long long result = compute_hash(data, data_size);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %llu\n", result);
    
    free(data);
    return 0;
}
