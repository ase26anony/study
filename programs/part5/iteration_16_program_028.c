#include <stdio.h>
#include <stdlib.h>

/* Force no inlining to keep RTL complex */
__attribute__((noinline))
static unsigned int high_pressure_computation(unsigned int *inputs) {
    /* Many distinct variables to create register pressure */
    register unsigned int v0, v1, v2, v3, v4, v5, v6, v7;
    register unsigned int v8, v9, v10, v11, v12, v13, v14, v15;
    register unsigned int v16, v17, v18, v19, v20, v21, v22, v23;
    
    /* Initialize from inputs to avoid constant propagation */
    v0 = inputs[0] ^ 0x12345678;
    v1 = inputs[1] + 0x89ABCDEF;
    v2 = inputs[2] * 0x13579BDF;
    v3 = inputs[3] | 0xF0F0F0F0;
    v4 = inputs[4] & 0x0F0F0F0F;
    v5 = inputs[5] << 3;
    v6 = inputs[6] >> 2;
    v7 = inputs[7] ^ v0;
    
    /* Create rematerialization candidates - pure functions of inputs */
    /* These will have long live ranges across the loop */
    unsigned int cand1 = v0 + 0x11111111;  /* Cheap: v0 + constant */
    unsigned int cand2 = v1 & 0xAAAAAAAA;  /* Cheap: v1 & mask */
    unsigned int cand3 = v2 << 2;          /* Cheap: v2 << shift */
    unsigned int cand4 = v3 ^ 0xCCCCCCCC;  /* Cheap: v3 ^ constant */
    unsigned int cand5 = v4 | 0x33333333;  /* Cheap: v4 | constant */
    unsigned int cand6 = v5 + v6;          /* Cheap: addition */
    unsigned int cand7 = v7 * 3;           /* Cheap: multiplication by small constant */
    
    /* More variables to increase pressure */
    v8 = v0 + v1;
    v9 = v2 - v3;
    v10 = v4 * v5;
    v11 = v6 / (v7 ? v7 : 1);
    v12 = v8 ^ v9;
    v13 = v10 & v11;
    v14 = v12 | v13;
    v15 = v14 << 1;
    
    /* Complex control flow with nested loops */
    unsigned int result = 0;
    
    /* Outer loop - cand1..cand7 must stay live */
    for (int i = 0; i < 3; i++) {
        v16 = inputs[i] + i;
        
        /* Inner loop with conditional */
        for (int j = 0; j < 4; j++) {
            v17 = v16 * j;
            v18 = v17 + cand1;  /* Use cand1 */
            v19 = v18 & cand2;  /* Use cand2 */
            
            /* Conditional branch using different sets */
            if (j & 1) {
                v20 = v19 | cand3;  /* Use cand3 */
                v21 = v20 ^ cand4;  /* Use cand4 */
                result += v21;
            } else {
                v22 = v19 & cand5;  /* Use cand5 */
                v23 = v22 + cand6;  /* Use cand6 */
                result += v23 * cand7;  /* Use cand7 */
            }
            
            /* More computations to keep values live */
            v0 = v0 + 1;
            v1 = v1 - 1;
            v2 = v2 ^ v3;
            v3 = v3 | v4;
            v4 = v4 & v5;
            v5 = v5 << 1;
            v6 = v6 >> 1;
            v7 = v7 * 2;
        }
    }
    
    /* Final computation using all candidates to ensure they're live */
    result += cand1 + cand2 + cand3 + cand4 + cand5 + cand6 + cand7;
    
    /* Use all variables in final result to prevent dead code elimination */
    result ^= v0 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7;
    result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    return result;
}

/* Another noinline function to create more pressure */
__attribute__((noinline))
static unsigned int create_more_pressure(unsigned int seed) {
    /* Many independent computations */
    unsigned int a = seed * 0x9E3779B9;
    unsigned int b = seed + 0x7F4A7C15;
    unsigned int c = seed ^ 0x3C6EF372;
    unsigned int d = seed & 0xFFFFFFFF;
    unsigned int e = seed | 0xAAAAAAAA;
    unsigned int f = seed << 5;
    unsigned int g = seed >> 3;
    unsigned int h = seed % 997;
    
    /* Chain computations to create dependencies */
    for (int i = 0; i < 8; i++) {
        a = a * 1103515245 + 12345;
        b = b ^ (b << 13);
        b = b ^ (b >> 17);
        b = b ^ (b << 5);
        c = c + d;
        d = d ^ e;
        e = e | f;
        f = f & g;
        g = g + h;
        h = h * 3;
    }
    
    /* Force all to be used */
    return a + b + c + d + e + f + g + h;
}

int main(void) {
    /* Initialize with non-zero values */
    unsigned int inputs[32];
    for (int i = 0; i < 32; i++) {
        inputs[i] = i * 0x1234567 + 0x89ABCDEF;
    }
    
    /* Call high-pressure functions */
    unsigned int result1 = high_pressure_computation(inputs);
    unsigned int result2 = create_more_pressure(result1);
    
    /* Combine results deterministically */
    unsigned int final_result = result1 ^ result2;
    
    printf("Result: 0x%08X\n", final_result);
    
    /* Verify with a simple check */
    if (final_result == 0) {
        printf("Unexpected zero result\n");
        return 1;
    }
    
    return 0;
}
