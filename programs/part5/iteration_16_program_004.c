#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent optimization */
static int __attribute__((noinline)) 
high_pressure_computation(int *inputs) 
{
    /* Declare many distinct variables to create register pressure */
    register int v0, v1, v2, v3, v4, v5, v6, v7;
    register int v8, v9, v10, v11, v12, v13, v14, v15;
    register int v16, v17, v18, v19, v20, v21, v22, v23;
    register int v24, v25, v26, v27, v28, v29, v30, v31;
    
    /* Initialize from inputs to prevent constant propagation */
    v0 = inputs[0] + 1;      /* Candidate for remat: v0 = inputs[0] + 1 */
    v1 = inputs[1] * 2;      /* Candidate: v1 = inputs[1] * 2 */
    v2 = inputs[2] & 0xFF;   /* Candidate: v2 = inputs[2] & 0xFF */
    v3 = inputs[3] << 3;     /* Candidate: v3 = inputs[3] << 3 */
    v4 = inputs[4] | 0x80;   /* Candidate: v4 = inputs[4] | 0x80 */
    v5 = inputs[5] ^ 0x55;   /* Candidate: v5 = inputs[5] ^ 0x55 */
    v6 = inputs[6] - 42;     /* Candidate: v6 = inputs[6] - 42 */
    v7 = inputs[7] + 100;    /* Candidate: v7 = inputs[7] + 100 */
    
    /* More variables with different computations */
    v8 = inputs[8] * 3 + 1;
    v9 = inputs[9] / 2 + 7;
    v10 = inputs[10] & 0xF0;
    v11 = inputs[11] << 1;
    v12 = inputs[12] | 0x0F;
    v13 = inputs[13] ^ 0xAA;
    v14 = inputs[14] - 17;
    v15 = inputs[15] + 255;
    
    v16 = inputs[16] * 5 - 3;
    v17 = inputs[17] / 3 + 11;
    v18 = inputs[18] & 0x3F;
    v19 = inputs[19] << 2;
    v20 = inputs[20] | 0xC0;
    v21 = inputs[21] ^ 0x33;
    v22 = inputs[22] - 29;
    v23 = inputs[23] + 512;
    
    v24 = inputs[24] * 7 + 5;
    v25 = inputs[25] / 4 + 13;
    v26 = inputs[26] & 0x1F;
    v27 = inputs[27] << 4;
    v28 = inputs[28] | 0xE0;
    v29 = inputs[29] ^ 0xCC;
    v30 = inputs[30] - 53;
    v31 = inputs[31] + 1024;
    
    /* Complex loop with conditional branches to create merging points */
    int result = 0;
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use all variables inside loop to keep them live */
        if (i & 1) {
            /* Branch 1: use first half of variables */
            result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
            result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
            
            /* Additional computations to increase pressure */
            int t0 = v0 * v1 + v2;  /* Creates more temporaries */
            int t1 = v3 & v4 | v5;
            int t2 = v6 << (v7 & 3);
            int t3 = v8 ^ v9 + v10;
            
            result += t0 + t1 + t2 + t3;
        } else {
            /* Branch 2: use second half of variables */
            result += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
            result += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
            
            /* Different computations in this branch */
            int t4 = v16 / (v17 + 1) + v18;
            int t5 = v19 | v20 & v21;
            int t6 = v22 - v23 * 2;
            int t7 = v24 ^ v25 + v26;
            
            result += t4 + t5 + t6 + t7;
        }
        
        /* Use some variables in both branches to ensure liveness across merge */
        result += (v0 & 1) ? v31 : v30;
        result += (v16 & 2) ? v15 : v14;
        
        /* Nested loop to further complicate liveness analysis */
        for (int j = 0; j < 3; j++) {
            /* Mix variables from different sets */
            int mix = (v0 + v16) * (v8 + v24) >> j;
            result += mix & 0xFF;
            
            /* More computations using candidate variables */
            mix = (v1 & v17) | (v9 ^ v25);
            result += mix % 256;
        }
        
        /* Modify some variables to prevent CSE */
        v0 += i & 0x1;
        v16 += i & 0x2;
    }
    
    /* Final combination using all variables */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    result += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
    result += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
    
    return result;
}

/* Another layer to prevent optimization */
static int __attribute__((noinline))
compute_hash(int *data, int size) 
{
    int hash = 0;
    
    /* Process data in chunks to create more register pressure */
    for (int i = 0; i < size; i += NUM_VARS) {
        int chunk[NUM_VARS];
        
        /* Load chunk with computations */
        for (int j = 0; j < NUM_VARS && (i + j) < size; j++) {
            chunk[j] = data[i + j] * 7919 + j; /* Prime multiplier */
        }
        
        /* Call high-pressure function */
        hash ^= high_pressure_computation(chunk);
        
        /* Additional mixing */
        hash = (hash << 13) | (hash >> 19);
    }
    
    return hash;
}

int main() 
{
    /* Initialize test data */
    int test_data[256];
    for (int i = 0; i < 256; i++) {
        test_data[i] = i * 3 + 7;
    }
    
    /* Perform computation */
    int result = compute_hash(test_data, 256);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
