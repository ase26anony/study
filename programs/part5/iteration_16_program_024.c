#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent optimization */
static int __attribute__((noinline)) 
high_pressure_computation(int *inputs) 
{
    /* Declare many distinct variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31;
    
    /* Initialize from inputs to prevent constant propagation */
    v0 = inputs[0] + 1;
    v1 = inputs[1] * 2;
    v2 = inputs[2] & 0xFF;
    v3 = inputs[3] | 0x100;
    v4 = inputs[4] ^ 0x55;
    v5 = inputs[5] << 1;
    v6 = inputs[6] >> 2;
    v7 = inputs[7] + inputs[0];
    v8 = inputs[8] - inputs[1];
    v9 = inputs[9] * 3;
    
    v10 = inputs[10] & 0xF0;
    v11 = inputs[11] | 0x0F;
    v12 = inputs[12] ^ 0xAA;
    v13 = inputs[13] << 2;
    v14 = inputs[14] >> 1;
    v15 = inputs[15] + inputs[2];
    v16 = inputs[16] - inputs[3];
    v17 = inputs[17] * 5;
    v18 = inputs[18] & 0x0F;
    v19 = inputs[19] | 0xF0;
    
    v20 = inputs[20] ^ 0x33;
    v21 = inputs[21] << 3;
    v22 = inputs[22] >> 3;
    v23 = inputs[23] + inputs[4];
    v24 = inputs[24] - inputs[5];
    v25 = inputs[25] * 7;
    v26 = inputs[26] & 0x3F;
    v27 = inputs[27] | 0xC0;
    v28 = inputs[28] ^ 0xCC;
    v29 = inputs[29] << 1;
    
    v30 = inputs[30] >> 4;
    v31 = inputs[31] + inputs[6];
    
    /* Create rematerialization candidates - pure functions of inputs */
    /* These will have long live ranges across the loop */
    int cand1 = v0 + v1;      /* Cheap: addition */
    int cand2 = v2 & 0x7F;    /* Cheap: bitwise AND with constant */
    int cand3 = v3 << 2;      /* Cheap: shift */
    int cand4 = v4 ^ 0x99;    /* Cheap: XOR with constant */
    int cand5 = v5 + 42;      /* Cheap: addition with constant */
    int cand6 = v6 | 0x01;    /* Cheap: bitwise OR */
    
    int result = 0;
    
    /* Complex loop to create complicated liveness patterns */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use the candidates inside the loop - they must stay live */
        int t1 = cand1 + i;
        int t2 = cand2 - i;
        int t3 = cand3 * i;
        int t4 = cand4 ^ i;
        int t5 = cand5 & i;
        int t6 = cand6 | i;
        
        /* Many independent computations to increase register pressure */
        int a = v7 + v8 + v9;
        int b = v10 - v11 - v12;
        int c = v13 * v14 * v15;
        int d = v16 & v17 & v18;
        int e = v19 | v20 | v21;
        int f = v22 ^ v23 ^ v24;
        int g = v25 + v26 + v27;
        int h = v28 - v29 - v30;
        
        /* Conditional to create merging points with many live values */
        if (i & 1) {
            /* Use different combinations in different branches */
            a = a + t1;
            b = b + t2;
            c = c + t3;
            d = d + t4;
        } else {
            a = a - t1;
            b = b - t2;
            c = c - t3;
            d = d - t4;
        }
        
        /* Nested loop to further complicate liveness analysis */
        for (int j = 0; j < 3; j++) {
            /* Use all candidates and many variables here */
            int inner = (t5 + j) * (t6 - j);
            inner += v31 * j;
            inner += a * b * c * d;
            inner += e + f + g + h;
            
            /* Use more variables to keep them live */
            inner += v0 * v1 * v2;
            inner += v3 + v4 + v5;
            inner += v6 & v7 & v8;
            inner += v9 | v10 | v11;
            inner += v12 ^ v13 ^ v14;
            
            result += inner;
        }
        
        /* Modify some variables to prevent dead code elimination */
        v0 += 1;
        v1 -= 1;
        v2 ^= 0x01;
        v3 |= 0x02;
    }
    
    /* Final combination that uses all candidates and variables */
    result += cand1 + cand2 + cand3 + cand4 + cand5 + cand6;
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6;
    result += v7 - v8 - v9 - v10;
    result += v11 * v12 * v13;
    result += v14 & v15 & v16;
    result += v17 | v18 | v19;
    result += v20 ^ v21 ^ v22;
    result += v23 + v24 + v25;
    result += v26 - v27 - v28;
    result += v29 * v30 * v31;
    
    return result;
}

/* Another function to create more pressure through multiple calls */
static int __attribute__((noinline))
secondary_pressure(int seed) 
{
    /* Different computation pattern */
    int a = seed * 3;
    int b = seed + 17;
    int c = seed & 0x7FFF;
    int d = seed | 0x8000;
    int e = seed ^ 0xABCD;
    int f = seed << 3;
    int g = seed >> 2;
    
    /* Candidates with different expressions */
    int cand7 = a * 2;
    int cand8 = b + 255;
    int cand9 = c & 0x3F;
    int cand10 = d << 1;
    
    int sum = 0;
    
    /* Loop with conditional that uses candidates */
    for (int i = 0; i < 50; i++) {
        if (i % 3 == 0) {
            sum += cand7 + i;
        } else if (i % 3 == 1) {
            sum += cand8 - i;
        } else {
            sum += cand9 * i;
        }
        
        /* Keep many variables live */
        sum += a + b + c + d + e + f + g;
        sum += cand10;
        
        /* More computations */
        a += i;
        b -= i;
        c ^= i;
        d |= i;
    }
    
    return sum;
}

int main(void) 
{
    /* Initialize input array with non-zero values */
    int inputs[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = i + 1;
    }
    
    /* Perform high-pressure computation */
    int result1 = high_pressure_computation(inputs);
    
    /* Perform secondary computation */
    int result2 = secondary_pressure(result1);
    
    /* Combine results */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Verify with expected value for this specific input */
    if (final_result == 2147483647) {
        printf("Unexpected sentinel value\n");
    }
    
    return 0;
}
