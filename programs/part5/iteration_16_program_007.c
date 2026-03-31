#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 50
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static int __attribute__((noinline)) 
high_pressure_computation(int *inputs) 
{
    /* Declare many distinct variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39;
    int v40, v41, v42, v43, v44, v45, v46, v47, v48, v49;
    
    /* Initialize from inputs to prevent constant propagation */
    v0 = inputs[0];  v1 = inputs[1];  v2 = inputs[2];  v3 = inputs[3];  v4 = inputs[4];
    v5 = inputs[5];  v6 = inputs[6];  v7 = inputs[7];  v8 = inputs[8];  v9 = inputs[9];
    v10 = inputs[10]; v11 = inputs[11]; v12 = inputs[12]; v13 = inputs[13]; v14 = inputs[14];
    v15 = inputs[15]; v16 = inputs[16]; v17 = inputs[17]; v18 = inputs[18]; v19 = inputs[19];
    v20 = inputs[20]; v21 = inputs[21]; v22 = inputs[22]; v23 = inputs[23]; v24 = inputs[24];
    v25 = inputs[25]; v26 = inputs[26]; v27 = inputs[27]; v28 = inputs[28]; v29 = inputs[29];
    v30 = inputs[30]; v31 = inputs[31]; v32 = inputs[32]; v33 = inputs[33]; v34 = inputs[34];
    v35 = inputs[35]; v36 = inputs[36]; v37 = inputs[37]; v38 = inputs[38]; v39 = inputs[39];
    v40 = inputs[40]; v41 = inputs[41]; v42 = inputs[42]; v43 = inputs[43]; v44 = inputs[44];
    v45 = inputs[45]; v46 = inputs[46]; v47 = inputs[47]; v48 = inputs[48]; v49 = inputs[49];
    
    /* Phase 1: Create many rematerialization candidates - pure expressions */
    /* These will have long live ranges and be cheap to recompute */
    int r0 = v0 + 12345;      /* Candidate 1: v0 + constant */
    int r1 = v1 & 0xFFFF;     /* Candidate 2: bitmask operation */
    int r2 = v2 << 3;         /* Candidate 3: shift operation */
    int r3 = v3 * 7;          /* Candidate 4: multiplication by constant */
    int r4 = v4 ^ 0xAA55;     /* Candidate 5: XOR with constant */
    int r5 = v5 - 999;        /* Candidate 6: subtraction */
    int r6 = v6 | 0x00FF;     /* Candidate 7: OR with constant */
    int r7 = v7 + v8;         /* Candidate 8: addition of two vars */
    int r8 = v9 * 13;         /* Candidate 9: another multiplication */
    int r9 = v10 >> 2;        /* Candidate 10: right shift */
    
    /* Keep these results live across complex control flow */
    int sum = 0;
    
    /* Outer loop - creates complex liveness patterns */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use remat candidates inside loop - they're defined outside */
        int t0 = r0 + i;      /* Use candidate 1 */
        int t1 = r1 ^ i;      /* Use candidate 2 */
        int t2 = r2 - i;      /* Use candidate 3 */
        
        /* Inner loop with more computations */
        for (int j = 0; j < 5; j++) {
            /* More uses of remat candidates */
            int u0 = r3 + j;
            int u1 = r4 ^ j;
            int u2 = r5 - j;
            
            /* Conditional inside inner loop - creates merge points */
            if ((i + j) & 1) {
                /* Use different sets of variables in each branch */
                sum += t0 + u0 + v11 + v12 + v13;
                sum += r6 + r7 + v14 + v15;
            } else {
                sum += t1 + u1 + v16 + v17 + v18;
                sum += r8 + r9 + v19 + v20;
            }
            
            /* More independent computations to increase pressure */
            int w0 = v21 * v22 + v23;
            int w1 = v24 / (v25 + 1) + v26;
            int w2 = v27 << (v28 & 3);
            int w3 = v29 ^ v30 ^ v31;
            int w4 = v32 + v33 * 2;
            int w5 = v34 - v35 / 2;
            int w6 = v36 & v37 | v38;
            int w7 = v39 + v40 - v41;
            int w8 = v42 * 3 + v43;
            int w9 = v44 ^ v45 << 1;
            
            /* Use all these computed values to keep them live */
            sum += w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9;
        }
        
        /* More computations between loop iterations */
        int x0 = v46 + i * 2;
        int x1 = v47 - i / 2;
        int x2 = v48 ^ (i << 1);
        int x3 = v49 * (i + 1);
        
        /* Use remat candidates again */
        sum += r0 + r1 + r2 + r3 + r4;
        sum += r5 + r6 + r7 + r8 + r9;
        sum += x0 + x1 + x2 + x3;
        
        /* Force spill/reload candidates by using many other vars */
        sum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        sum += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    }
    
    /* Final use of all rematerialization candidates */
    int final = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    
    return sum + final;
}

/* Another high-pressure function with different pattern */
static int __attribute__((noinline))
another_high_pressure_func(int a, int b, int c, int d, int e)
{
    /* Create many intermediate values */
    int t1 = a * b + 123;
    int t2 = c ^ d ^ 0x55AA;
    int t3 = e << 4;
    int t4 = a + b * 2;
    int t5 = c & 0xFF00;
    int t6 = d | 0x00FF;
    int t7 = e * 3 - 1;
    int t8 = a ^ b ^ c;
    int t9 = d + e * 5;
    int t10 = a & b & c;
    
    /* Keep them all live across a switch statement */
    int result = 0;
    for (int i = 0; i < 50; i++) {
        switch (i % 4) {
            case 0:
                result += t1 + t2 + t3;
                break;
            case 1:
                result += t4 + t5 + t6;
                break;
            case 2:
                result += t7 + t8 + t9;
                break;
            case 3:
                result += t10 + t1 - t2;
                break;
        }
        
        /* More computations that use all temps */
        result += (t1 >> 2) + (t2 & 0xF) + (t3 << 1);
        result += (t4 * 2) + (t5 / 2) + (t6 ^ 0xFF);
        result += (t7 - 1) + (t8 | 0xAA) + (t9 & 0x55);
    }
    
    return result;
}

int main(void) 
{
    /* Initialize input array with pseudo-random but deterministic values */
    int inputs[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = (i * 37 + 123) & 0xFFF;  /* Generate varied values */
    }
    
    /* Call high-pressure functions */
    int result1 = high_pressure_computation(inputs);
    int result2 = another_high_pressure_func(
        inputs[0], inputs[1], inputs[2], inputs[3], inputs[4]);
    
    /* Combine results to ensure all computations are used */
    int final_result = result1 + result2;
    
    printf("Result: %d\n", final_result);
    
    /* Verify with expected value for this specific input */
    if (final_result == 2147483647) {  /* Just a placeholder */
        printf("Verification passed\n");
    }
    
    return 0;
}
