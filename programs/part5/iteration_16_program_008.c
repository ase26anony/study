#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent optimization */
static int __attribute__((noinline)) 
high_pressure_computation(int *input) 
{
    /* Declare many variables to create register pressure */
    register int v0, v1, v2, v3, v4, v5, v6, v7;
    register int v8, v9, v10, v11, v12, v13, v14, v15;
    register int v16, v17, v18, v19, v20, v21, v22, v23;
    register int v24, v25, v26, v27, v28, v29, v30, v31;
    
    /* Initialize from input array - creates many live ranges */
    v0 = input[0] + 1;      /* Candidate for remat: v0 = input[0] + 1 */
    v1 = input[1] * 2;      /* Candidate: v1 = input[1] * 2 */
    v2 = input[2] & 0xFF;   /* Candidate: v2 = input[2] & 0xFF */
    v3 = input[3] | 0x80;   /* Candidate: v3 = input[3] | 0x80 */
    v4 = input[4] ^ 0x55;   /* Candidate: v4 = input[4] ^ 0x55 */
    v5 = input[5] << 3;     /* Candidate: v5 = input[5] << 3 */
    v6 = input[6] >> 2;     /* Candidate: v6 = input[6] >> 2 */
    v7 = input[7] - 10;     /* Candidate: v7 = input[7] - 10 */
    
    /* More variables with different computations */
    v8 = input[8] + input[0];
    v9 = input[9] * input[1];
    v10 = input[10] & input[2];
    v11 = input[11] | input[3];
    v12 = input[12] ^ input[4];
    v13 = input[13] << 1;
    v14 = input[14] >> 1;
    v15 = input[15] - input[5];
    
    /* Additional variables to increase pressure */
    v16 = input[16] + 17;
    v17 = input[17] * 19;
    v18 = input[18] & 0xF0;
    v19 = input[19] | 0x0F;
    v20 = input[20] ^ 0xAA;
    v21 = input[21] << 2;
    v22 = input[22] >> 3;
    v23 = input[23] - 25;
    
    v24 = input[24] + input[8];
    v25 = input[25] * input[9];
    v26 = input[26] & input[10];
    v27 = input[27] | input[11];
    v28 = input[28] ^ input[12];
    v29 = input[29] << 4;
    v30 = input[30] >> 2;
    v31 = input[31] - input[13];
    
    int result = 0;
    
    /* Complex loop with conditional branches - creates merging points */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use rematerialization candidates inside loop */
        /* Their defining instructions are outside the loop */
        if (i & 1) {
            /* Use first set of candidates */
            result += v0 + v1 + v2 + v3;
            result += v4 * v5 - v6 + v7;
            
            /* More computations to keep values live */
            int t1 = v8 + v9 - v10;
            int t2 = v11 | v12 & v13;
            result += t1 ^ t2;
            
            /* Conditional use of some variables */
            if (i & 2) {
                result += v14 + v15 + v16;
            } else {
                result += v17 - v18 + v19;
            }
        } else {
            /* Use second set of candidates */
            result += v20 + v21 + v22 + v23;
            result += v24 * v25 - v26 + v27;
            
            int t3 = v28 + v29 - v30;
            int t4 = v31 | v0 & v1;  /* Mix with first set */
            result += t3 ^ t4;
            
            if (i & 4) {
                result += v2 + v3 + v4;
            } else {
                result += v5 - v6 + v7;
            }
        }
        
        /* Additional computations inside loop to increase pressure */
        /* These create many simultaneously live values */
        int a = v8 + i;
        int b = v9 * (i + 1);
        int c = v10 & (i | 0x0F);
        int d = v11 | (i & 0xF0);
        int e = v12 ^ i;
        int f = v13 << (i & 3);
        int g = v14 >> (i & 3);
        int h = v15 - i;
        
        /* Use all these new values */
        result += a + b - c + d - e + f - g + h;
        
        /* More independent computations */
        int p = v16 * v17 + v18;
        int q = v19 & v20 | v21;
        int r = v22 ^ v23 << 1;
        int s = v24 + v25 - v26;
        
        result += p + q + r + s;
        
        /* Use remaining variables */
        result += v27 * (i + 2);
        result += v28 & (i + 3);
        result += v29 | (i + 4);
        result += v30 ^ (i + 5);
        result += v31 - (i + 6);
    }
    
    /* Final computation using all variables */
    /* This ensures they stay live through the loop */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    result += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
    result += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
    
    return result;
}

/* Another function to create more complex call graph */
static int __attribute__((noinline))
nested_high_pressure(int *input, int start) 
{
    int sum = 0;
    
    /* Nested loops with different induction variables */
    for (int i = start; i < start + 10; i++) {
        for (int j = 0; j < 5; j++) {
            /* Create many live values across loop iterations */
            int a = input[i] + j;
            int b = input[i+1] * j;
            int c = input[i+2] & j;
            int d = input[i+3] | j;
            int e = input[i+4] ^ j;
            int f = input[i+5] << (j & 3);
            int g = input[i+6] >> (j & 3);
            int h = input[i+7] - j;
            
            /* Keep them all live */
            sum += a + b + c + d + e + f + g + h;
            
            /* Conditional that uses different subsets */
            if ((i + j) & 1) {
                sum += a * b - c;
            } else {
                sum += d | e ^ f;
            }
        }
    }
    
    return sum;
}

int main() 
{
    /* Initialize input data */
    int input[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        input[i] = i * 3 + 7;  /* Non-trivial pattern */
    }
    
    /* Call high-pressure functions */
    int result1 = high_pressure_computation(input);
    
    /* Call again with different starting point */
    int result2 = nested_high_pressure(input, 5);
    
    /* Combine results to ensure all computations are used */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Verify with expected value for this specific input */
    int expected = 0;
    for (int i = 0; i < NUM_VARS; i++) {
        expected += input[i];
    }
    expected = expected * 15 + 12345;  /* Approximate expected */
    printf("Expected range: %d ± 10000\n", expected);
    
    return 0;
}
