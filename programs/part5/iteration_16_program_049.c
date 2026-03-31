/* Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat -S -o remat_test.s remat_test.c */
/* For even higher pressure: add -m32 or -fno-schedule-insns */

#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 40
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static int __attribute__((noinline,noipa))
high_pressure_computation(int *input) {
    /* Declare many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39;
    
    /* Initialize from input array - creates many live ranges */
    v0 = input[0] + 1;      /* Candidate for remat: v0 = input[0] + 1 */
    v1 = input[1] * 2;
    v2 = input[2] & 0xFF;
    v3 = input[3] | 0x80;
    v4 = input[4] ^ 0x55;
    v5 = input[5] << 1;
    v6 = input[6] >> 2;
    v7 = input[7] + input[0];
    v8 = input[8] - input[1];
    v9 = input[9] * 3;
    
    v10 = input[10] & 0xF0;
    v11 = input[11] | 0x0F;
    v12 = input[12] ^ 0xAA;
    v13 = input[13] << 2;
    v14 = input[14] >> 1;
    v15 = input[15] + 100;
    v16 = input[16] - 50;
    v17 = input[17] * 5;
    v18 = input[18] & 0x3F;
    v19 = input[19] | 0xC0;
    
    v20 = input[20] ^ 0x33;
    v21 = input[21] << 3;
    v22 = input[22] >> 4;
    v23 = input[23] + 200;
    v24 = input[24] - 100;
    v25 = input[25] * 7;
    v26 = input[26] & 0x1F;
    v27 = input[27] | 0xE0;
    v28 = input[28] ^ 0xCC;
    v29 = input[29] << 1;
    
    v30 = input[30] >> 3;
    v31 = input[31] + 300;
    v32 = input[32] - 150;
    v33 = input[33] * 11;
    v34 = input[34] & 0x7F;
    v35 = input[35] | 0x88;
    v36 = input[36] ^ 0x66;
    v37 = input[37] << 2;
    v38 = input[38] >> 5;
    v39 = input[39] + 400;
    
    /* Create complex control flow with nested loops */
    int sum = 0;
    
    /* Outer loop - many variables live across loop */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        int temp_sum = 0;
        
        /* Use rematerialization candidates inside loop */
        /* v0, v1, v2 are defined outside but used inside - good remat candidates */
        if (i & 1) {
            temp_sum += v0 + v1;  /* Uses values defined outside loop */
            temp_sum += v2 * i;
        } else {
            temp_sum += v3 - v4;
            temp_sum += v5 ^ v6;
        }
        
        /* Inner loop with more computations */
        for (int j = 0; j < 5; j++) {
            /* Use many different variables to keep them all live */
            temp_sum += v7 + v8 + v9;
            temp_sum += v10 & v11;
            temp_sum += v12 | v13;
            temp_sum += v14 ^ v15;
            
            /* Conditional that uses different variable sets */
            if (j & 1) {
                temp_sum += v16 + v17 + v18;
                temp_sum += v19 * v20;
            } else {
                temp_sum += v21 - v22;
                temp_sum += v23 & v24;
            }
            
            /* More computations to increase pressure */
            temp_sum += v25 >> (j + 1);
            temp_sum += v26 << (j & 3);
            temp_sum += v27 ^ (v28 + j);
            temp_sum += v29 & (v30 | j);
        }
        
        /* Use another set of variables after inner loop */
        temp_sum += v31 + v32;
        temp_sum += v33 - v34;
        temp_sum += v35 & v36;
        temp_sum += v37 | v38;
        temp_sum += v39 ^ i;
        
        /* Complex conditional with many live variables */
        if (temp_sum > 1000) {
            sum += temp_sum & 0xFF;
            /* Use more variables in this path */
            sum += v0 * v1;  /* v0, v1 used again - good remat candidate */
            sum += v2 | v3;
        } else {
            sum += temp_sum | 0x80;
            /* Different variables in else path */
            sum += v4 ^ v5;
            sum += v6 & v7;
        }
        
        /* Force all variables to stay live by using them in final computation */
        sum += (v8 + v9 + v10) * i;
        sum += (v11 & v12) >> 1;
        sum += (v13 | v14) << 2;
        sum += (v15 ^ v16) + v17;
    }
    
    /* Final computation using ALL variables to ensure none are optimized away */
    int final_result = sum;
    final_result += v0 + v1 + v2 + v3 + v4;
    final_result += v5 - v6 - v7 - v8 - v9;
    final_result += v10 & v11 & v12 & v13 & v14;
    final_result += v15 | v16 | v17 | v18 | v19;
    final_result += v20 ^ v21 ^ v22 ^ v23 ^ v24;
    final_result += v25 * v26 * v27 / 1000;
    final_result += (v28 << 2) + (v29 >> 1);
    final_result += v30 & 0x7F;
    final_result += v31 | 0x80;
    final_result += v32 ^ 0x55;
    final_result += v33 + v34 + v35 + v36;
    final_result += v37 - v38 - v39;
    
    return final_result;
}

/* Another layer to increase optimization opportunities */
static int __attribute__((noinline))
compute_hash(int *data, int size) {
    int hash = 0;
    
    for (int i = 0; i < size; i += NUM_VARS) {
        /* Process in chunks, each call creates high register pressure */
        int chunk_result = high_pressure_computation(data + i);
        
        /* Mix results in a way that prevents dead code elimination */
        hash = (hash * 31) ^ chunk_result;
        
        /* Additional computations to increase pressure between calls */
        for (int j = 0; j < 10; j++) {
            hash += (data[i % size] << j);
            hash ^= (chunk_result >> (j & 7));
        }
    }
    
    return hash;
}

int main(void) {
    /* Initialize test data */
    int test_data[100];
    
    for (int i = 0; i < 100; i++) {
        test_data[i] = i * 3 + 7;  /* Simple but non-trivial pattern */
    }
    
    /* Perform computation that should trigger early remat */
    int result = compute_hash(test_data, 100);
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Also use in a conditional to prevent other optimizations */
    if (result > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
