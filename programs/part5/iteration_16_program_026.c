#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 40
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent optimization */
static int __attribute__((noinline)) 
high_pressure_computation(int *input) 
{
    /* Declare many distinct variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39;
    
    /* Initialize from input array - creates many live ranges */
    v0 = input[0] + 1;      /* Candidate for remat: v0 = input[0] + 1 */
    v1 = input[1] ^ 0x55AA; /* Candidate: bitwise operation */
    v2 = input[2] << 3;     /* Candidate: shift operation */
    v3 = input[3] * 7;      /* Candidate: multiplication */
    v4 = input[4] & 0xFF;   /* Candidate: mask operation */
    v5 = input[5] | 0x80;   /* Candidate: bitwise OR */
    v6 = input[6] - 42;     /* Candidate: subtraction */
    v7 = input[7] + v0;     /* Depends on v0 - creates chain */
    v8 = input[8] * v1;     /* Depends on v1 */
    v9 = input[9] ^ v2;     /* Depends on v2 */
    
    /* More independent computations */
    v10 = input[10] + 100;
    v11 = input[11] * 13;
    v12 = input[12] << 1;
    v13 = input[13] & 0xF0;
    v14 = input[14] | 0x0F;
    v15 = input[15] - 99;
    v16 = input[16] + v3;
    v17 = input[17] * v4;
    v18 = input[18] ^ v5;
    v19 = input[19] + v6;
    
    /* Additional computations to increase pressure */
    v20 = v0 + v1;          /* Uses early candidates */
    v21 = v2 * v3;
    v22 = v4 & v5;
    v23 = v6 | v7;
    v24 = v8 ^ v9;
    v25 = v10 - v11;
    v26 = v12 + v13;
    v27 = v14 * v15;
    v28 = v16 & v17;
    v29 = v18 | v19;
    
    /* More computations to keep values live */
    v30 = v20 << 2;
    v31 = v21 >> 1;
    v32 = v22 * 3;
    v33 = v23 + 17;
    v34 = v24 ^ 0xCC;
    v35 = v25 & 0xAA;
    v36 = v26 | 0x11;
    v37 = v27 - 23;
    v38 = v28 + v29;
    v39 = v30 * v31;
    
    int result = 0;
    
    /* Complex loop with conditional branches - creates merging points */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use many of the candidate variables inside the loop */
        /* This keeps them live across loop iterations */
        if (i & 1) {
            /* Branch 1 uses one set of variables */
            result += v0 + v2 + v4 + v6 + v8;  /* Uses candidates defined before loop */
            result ^= v10 * v12;
            result |= v14 & v16;
        } else {
            /* Branch 2 uses a different set */
            result += v1 + v3 + v5 + v7 + v9;  /* Alternative candidates */
            result ^= v11 * v13;
            result |= v15 & v17;
        }
        
        /* More operations that use additional variables */
        result += v18 * (i % 16);
        result ^= v19 << (i & 3);
        result |= v20 >> ((i + 1) & 3);
        
        /* Nested loop to further complicate liveness */
        for (int j = 0; j < 3; j++) {
            /* Use different combinations in nested scope */
            int temp = v21 + v22 * j;
            temp ^= v23 << (j + 1);
            temp &= v24 | (j * 0x11);
            result += temp;
            
            /* Conditional inside nested loop */
            if (j == 1) {
                result ^= v25 + v26;
            } else {
                result |= v27 - v28;
            }
        }
        
        /* Use more variables to keep them live */
        result += v29 * (result & 0xFF);
        result ^= v30 + v31;
        result |= v32 & v33;
        
        /* Additional computations that might get rematerialized */
        int tmp1 = v34 + i;      /* Simple recomputation candidate */
        int tmp2 = v35 ^ (i * 2); /* Another candidate */
        int tmp3 = v36 & 0x7F;    /* Mask operation candidate */
        
        result += tmp1 * tmp2;
        result ^= tmp3 << 4;
        
        /* Use the remaining variables */
        result += v37 * (i % 7);
        result ^= v38 + v39;
        result |= (v0 ^ v1) & (v2 | v3);  /* Reuse earliest candidates */
    }
    
    /* Final computation using all variables to ensure they're not dead */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    result += v30 + v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39;
    
    return result;
}

/* Wrapper function to add another layer of complexity */
static int __attribute__((noinline))
compute_hash(int *data, int size) 
{
    int hash = 0x5A5A5A5A;
    
    /* Process data in chunks, creating more register pressure */
    for (int i = 0; i < size; i += 10) {
        int chunk[10];
        for (int j = 0; j < 10 && (i + j) < size; j++) {
            chunk[j] = data[i + j] ^ (hash >> (j * 3));
        }
        
        /* Call high-pressure function on each chunk */
        hash ^= high_pressure_computation(chunk);
        
        /* Additional computations to keep values live across calls */
        hash = (hash << 13) | (hash >> 19);  /* Rotate */
        hash += i * 0x9E3779B9;              /* Golden ratio mix */
    }
    
    return hash;
}

int main() 
{
    /* Create test data */
    int test_data[100];
    for (int i = 0; i < 100; i++) {
        test_data[i] = i * 3 + 7;
    }
    
    /* Perform computation */
    int result = compute_hash(test_data, 100);
    
    /* Print result to prevent optimization */
    printf("Result: %d (0x%08X)\n", result, result);
    
    /* Verify with a simple check */
    if (result != 0) {
        printf("Computation successful\n");
    }
    
    return 0;
}
