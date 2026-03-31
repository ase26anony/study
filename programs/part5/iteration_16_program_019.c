#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 50
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent optimization */
static int __attribute__((noinline)) 
high_pressure_computation(int *input) 
{
    /* Declare many local variables to create register pressure */
    register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    register int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    register int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    register int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39;
    register int v40, v41, v42, v43, v44, v45, v46, v47, v48, v49;
    
    /* Initialize from input array - creates many independent values */
    v0 = input[0] + 1;      /* Candidate for remat: v0 = input[0] + 1 */
    v1 = input[1] * 2;      /* Candidate for remat: v1 = input[1] * 2 */
    v2 = input[2] & 0xFF;   /* Candidate for remat: v2 = input[2] & 0xFF */
    v3 = input[3] | 0x80;   /* Candidate for remat: v3 = input[3] | 0x80 */
    v4 = input[4] ^ 0x55;   /* Candidate for remat: v4 = input[4] ^ 0x55 */
    v5 = input[5] << 3;     /* Candidate for remat: v5 = input[5] << 3 */
    v6 = input[6] >> 2;     /* Candidate for remat: v6 = input[6] >> 2 */
    v7 = input[7] + 0x100;  /* Candidate for remat: v7 = input[7] + 0x100 */
    v8 = input[8] * 3;      /* Candidate for remat: v8 = input[8] * 3 */
    v9 = input[9] - 42;     /* Candidate for remat: v9 = input[9] - 42 */
    
    /* More initializations with different operations */
    v10 = input[10] + input[0];
    v11 = input[11] * input[1];
    v12 = input[12] & input[2];
    v13 = input[13] | input[3];
    v14 = input[14] ^ input[4];
    v15 = input[15] << 1;
    v16 = input[16] >> 4;
    v17 = input[17] + 0x200;
    v18 = input[18] * 5;
    v19 = input[19] - 100;
    
    v20 = input[20] + 0x300;
    v21 = input[21] * 7;
    v22 = input[22] & 0xF0;
    v23 = input[23] | 0x0F;
    v24 = input[24] ^ 0xAA;
    v25 = input[25] << 2;
    v26 = input[26] >> 1;
    v27 = input[27] + 0x400;
    v28 = input[28] * 11;
    v29 = input[29] - 200;
    
    v30 = input[30] + input[10];
    v31 = input[31] * input[11];
    v32 = input[32] & input[12];
    v33 = input[33] | input[13];
    v34 = input[34] ^ input[14];
    v35 = input[35] << 3;
    v36 = input[36] >> 2;
    v37 = input[37] + 0x500;
    v38 = input[38] * 13;
    v39 = input[39] - 300;
    
    v40 = input[40] + 0x600;
    v41 = input[41] * 17;
    v42 = input[42] & 0xCC;
    v43 = input[43] | 0x33;
    v44 = input[44] ^ 0x99;
    v45 = input[45] << 4;
    v46 = input[46] >> 3;
    v47 = input[47] + 0x700;
    v48 = input[48] * 19;
    v49 = input[49] - 400;
    
    int result = 0;
    
    /* Complex loop with conditional branches - keeps many values live */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use rematerialization candidates inside loop */
        if (i % 3 == 0) {
            /* Use early-defined values v0-v9 inside loop */
            result += v0 + v1 + v2 + v3 + v4;
            result ^= v5 | v6 | v7 | v8 | v9;
        } else if (i % 3 == 1) {
            /* Use another set of values */
            result += v10 * v11 - v12 + v13 ^ v14;
            result |= v15 & v16 & v17 & v18 & v19;
        } else {
            /* Use more values */
            result += v20 << (i % 4);
            result ^= v21 >> (i % 4);
            result += v22 & v23;
            result |= v24 ^ v25;
        }
        
        /* More computations that use different variable sets */
        int temp = 0;
        if (i % 2 == 0) {
            temp = v30 + v31 - v32 * v33 / (v34 + 1);
            result += temp & v35;
        } else {
            temp = v40 | v41 ^ v42 & v43;
            result ^= temp + v44 - v45;
        }
        
        /* Nested loop to create more complex control flow */
        for (int j = 0; j < 3; j++) {
            /* Use even more variables inside nested loop */
            if (j == 0) {
                result += v26 + v27 - v28 * v29;
            } else if (j == 1) {
                result ^= v36 | v37 & v38 ^ v39;
            } else {
                result += v46 << j;
                result ^= v47 >> j;
                result |= v48 & v49;
            }
        }
        
        /* Prevent loop invariant code motion */
        result += input[i % 50];
    }
    
    /* Final computation using ALL variables - ensures they stay live */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    result += v30 + v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39;
    result += v40 + v41 + v42 + v43 + v44 + v45 + v46 + v47 + v48 + v49;
    
    return result;
}

/* Another function to create more pressure */
static int __attribute__((noinline))
secondary_pressure(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j)
{
    /* Create many intermediate computations */
    int t1 = a * b + c;
    int t2 = d << 2 | e;
    int t3 = f & g ^ h;
    int t4 = i - j * 3;
    int t5 = a + b << 1;
    int t6 = c ^ d & e;
    int t7 = f | g >> 2;
    int t8 = h * i - j;
    int t9 = a & b | c;
    int t10 = d ^ e + f;
    
    /* Keep them all live through a loop */
    int sum = 0;
    for (int k = 0; k < 50; k++) {
        if (k % 4 == 0) sum += t1 + t2;
        else if (k % 4 == 1) sum += t3 - t4;
        else if (k % 4 == 2) sum += t5 | t6;
        else sum += t7 ^ t8;
        
        sum += t9 * t10;
    }
    
    return sum;
}

int main(void) 
{
    /* Initialize input data */
    int input[50];
    for (int i = 0; i < 50; i++) {
        input[i] = i * 3 + 1;  /* Non-zero, non-uniform values */
    }
    
    /* Call high-pressure function */
    int result1 = high_pressure_computation(input);
    
    /* Call secondary function with many arguments */
    int result2 = secondary_pressure(
        input[0], input[1], input[2], input[3], input[4],
        input[5], input[6], input[7], input[8], input[9]
    );
    
    /* Combine results to ensure both are used */
    int final_result = result1 ^ result2;
    
    printf("Result: %d\n", final_result);
    
    /* Verify with a simple computation */
    int verify = 0;
    for (int i = 0; i < 50; i++) {
        verify += input[i];
    }
    printf("Verification sum: %d\n", verify);
    
    return 0;
}
