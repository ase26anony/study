#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 50
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static int __attribute__((noinline)) 
high_pressure_computation(int *inputs, int n) 
{
    /* Declare many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39;
    int v40, v41, v42, v43, v44, v45, v46, v47, v48, v49;
    
    /* Initialize variables with distinct computations to avoid CSE */
    v0 = inputs[0] + 1;
    v1 = inputs[1] * 2 - 3;
    v2 = inputs[2] & 0xFF00FF;
    v3 = inputs[3] | 0x00FF00;
    v4 = inputs[4] ^ 0x12345678;
    v5 = inputs[5] << 3;
    v6 = inputs[6] >> 2;
    v7 = inputs[7] + inputs[0];
    v8 = inputs[8] - inputs[1];
    v9 = inputs[9] * inputs[2];
    
    v10 = (inputs[10] + 17) * 3;
    v11 = (inputs[11] - 23) & 0x0F0F0F0F;
    v12 = inputs[12] | (1 << 16);
    v13 = inputs[13] ^ v0;
    v14 = v1 << 1;
    v15 = v2 >> 4;
    v16 = v3 + 0x1000;
    v17 = v4 - 0x2000;
    v18 = v5 * 5;
    v19 = v6 & 0x00FFFFFF;
    
    v20 = v7 | 0x80000000;
    v21 = v8 ^ 0x40000000;
    v22 = v9 << 2;
    v23 = v10 >> 1;
    v24 = v11 + v12;
    v25 = v13 - v14;
    v26 = v15 * v16;
    v27 = v17 & v18;
    v28 = v19 | v20;
    v29 = v21 ^ v22;
    
    v30 = v23 + 0x33333333;
    v31 = v24 - 0x22222222;
    v32 = v25 * 7;
    v33 = v26 & 0xAAAAAAAA;
    v34 = v27 | 0x55555555;
    v35 = v28 ^ 0x99999999;
    v36 = v29 << 5;
    v37 = v30 >> 3;
    v38 = v31 + v32;
    v39 = v33 - v34;
    
    v40 = v35 * v36;
    v41 = v37 & v38;
    v42 = v39 | v40;
    v43 = v41 ^ v42;
    v44 = v43 << 1;
    v45 = v44 >> 2;
    v46 = v45 + 0xDEADBEEF;
    v47 = v46 - 0xCAFEBABE;
    v48 = v47 * 0x1234567;
    v49 = v48 & 0xFEDCBA98;
    
    /* Create complex control flow with loops to extend live ranges */
    int i, j;
    int accumulator = 0;
    
    /* Outer loop - many variables remain live across iterations */
    for (i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use many variables in conditional computations */
        if (i & 1) {
            /* Branch 1: use one set of variables */
            accumulator += v0 + v2 + v4 + v6 + v8;
            accumulator ^= v10 | v12 | v14 | v16 | v18;
        } else {
            /* Branch 2: use different set of variables */
            accumulator -= v1 + v3 + v5 + v7 + v9;
            accumulator |= v11 & v13 & v15 & v17 & v19;
        }
        
        /* Inner loop with more variable usage */
        for (j = 0; j < 5; j++) {
            /* Mix in more variables - all must remain available */
            accumulator += (v20 >> j) + (v21 << j);
            accumulator ^= (v22 + j) & (v23 - j);
            
            /* Use variables defined outside loops - these are remat candidates */
            if ((j & 3) == 0) {
                accumulator += v24 * v25;
            } else {
                accumulator ^= v26 | v27;
            }
        }
        
        /* Use another set of variables periodically */
        if (i % 3 == 0) {
            accumulator += v28 + v29 + v30;
            accumulator ^= v31 & v32 & v33;
        }
        
        /* Force all variables to be live by using them in a complex expression */
        int temp = 0;
        temp += v34 + v35 + v36 + v37 + v38 + v39;
        temp ^= v40 | v41 | v42 | v43 | v44;
        temp &= v45 + v46 + v47 + v48 + v49;
        
        accumulator = (accumulator * 1103515245 + 12345) ^ temp;
    }
    
    /* Final computation using all variables to prevent dead code elimination */
    int result = accumulator;
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result ^= v10 | v11 | v12 | v13 | v14 | v15 | v16 | v17 | v18 | v19;
    result += v20 * v21 * v22 * v23 * v24;
    result ^= v25 & v26 & v27 & v28 & v29;
    result += v30 - v31 + v32 - v33 + v34;
    result ^= v35 | v36 | v37 | v38 | v39;
    result += v40 << 2;
    result ^= v41 >> 3;
    result += v42 & 0xFFFF;
    result ^= v43 | 0xFF0000;
    result += v44 * v45;
    result ^= v46 & v47;
    result += v48 - v49;
    
    return result;
}

/* Another function to create additional pressure */
static int __attribute__((noinline))
secondary_pressure(int x, int y, int z) 
{
    /* More independent computations */
    int a = x * 3 + 7;
    int b = y & 0xF0F0F0F0;
    int c = z | 0x0F0F0F0F;
    int d = a << 4;
    int e = b >> 2;
    int f = c ^ 0x12345678;
    int g = d + e;
    int h = f - g;
    int i = a * b;
    int j = c & d;
    
    /* Keep these live across a loop */
    int sum = 0;
    for (int k = 0; k < 20; k++) {
        if (k & 1) {
            sum += a + b + c + d;
        } else {
            sum ^= e | f | g | h;
        }
        sum = (sum * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    return sum + i + j;
}

int main(void) 
{
    /* Initialize input array with pseudo-random values */
    int inputs[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Perform high-pressure computation */
    int result1 = high_pressure_computation(inputs, NUM_VARS);
    
    /* Perform secondary computation to increase overall pressure */
    int result2 = secondary_pressure(inputs[0], inputs[1], inputs[2]);
    
    /* Combine results deterministically */
    int final_result = result1 ^ result2;
    
    printf("Result: %d\n", final_result);
    
    /* Verify with a simple check to ensure computation wasn't optimized away */
    if (final_result == 0) {
        printf("Warning: Result is zero\n");
    }
    
    return 0;
}
