#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static int __attribute__((noinline)) 
high_pressure_computation(int *inputs) 
{
    /* Declare many distinct variables to increase register pressure */
    register int v0, v1, v2, v3, v4, v5, v6, v7;
    register int v8, v9, v10, v11, v12, v13, v14, v15;
    register int v16, v17, v18, v19, v20, v21, v22, v23;
    register int v24, v25, v26, v27, v28, v29, v30, v31;
    
    /* Initialize from inputs to prevent constant propagation */
    v0 = inputs[0];  v1 = inputs[1];  v2 = inputs[2];  v3 = inputs[3];
    v4 = inputs[4];  v5 = inputs[5];  v6 = inputs[6];  v7 = inputs[7];
    v8 = inputs[8];  v9 = inputs[9];  v10 = inputs[10]; v11 = inputs[11];
    v12 = inputs[12]; v13 = inputs[13]; v14 = inputs[14]; v15 = inputs[15];
    v16 = inputs[16]; v17 = inputs[17]; v18 = inputs[18]; v19 = inputs[19];
    v20 = inputs[20]; v21 = inputs[21]; v22 = inputs[22]; v23 = inputs[23];
    v24 = inputs[24]; v25 = inputs[25]; v26 = inputs[26]; v27 = inputs[27];
    v28 = inputs[28]; v29 = inputs[29]; v30 = inputs[30]; v31 = inputs[31];
    
    /* Phase 1: Create rematerialization candidates outside loops */
    /* These are cheap to recompute but have long live ranges */
    int cand1 = v0 + 0x1234;      /* Simple addition - remat candidate */
    int cand2 = v1 & 0xFFFF00FF;  /* Bitwise AND - remat candidate */
    int cand3 = v2 << 3;          /* Shift - remat candidate */
    int cand4 = v3 ^ 0xAA55AA55;  /* XOR - remat candidate */
    int cand5 = v4 * 7 + 13;      /* Multiply-add - remat candidate */
    int cand6 = v5 | 0x0000FF00;  /* Bitwise OR - remat candidate */
    int cand7 = ~v6;              /* Bitwise NOT - remat candidate */
    int cand8 = v7 - 0x1000;      /* Subtraction - remat candidate */
    
    /* Keep these candidates live across complex control flow */
    int sum = 0;
    
    /* Outer loop with complex control flow */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use remat candidates inside loop - forcing them to stay live */
        int temp1 = cand1 + cand2;
        int temp2 = cand3 ^ cand4;
        int temp3 = cand5 | cand6;
        int temp4 = cand7 & cand8;
        
        /* Nested loop with different induction variables */
        for (int j = 0; j < 5; j++) {
            /* More computations using many variables simultaneously */
            int t0 = v8 + v9 * j;
            int t1 = v10 - v11 / (j + 1);
            int t2 = v12 << (j & 3);
            int t3 = v13 >> (j % 4);
            int t4 = v14 & v15;
            int t5 = v16 | v17;
            int t6 = v18 ^ v19;
            int t7 = v20 * v21;
            
            /* Conditional branch creating merge points with many live values */
            if (j & 1) {
                /* Use remat candidates in one branch */
                t0 += cand1;
                t1 += cand2;
                t2 ^= cand3;
                t3 |= cand4;
            } else {
                /* Use different remat candidates in other branch */
                t4 += cand5;
                t5 += cand6;
                t6 ^= cand7;
                t7 |= cand8;
            }
            
            /* Use all temporaries to keep them live */
            sum += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
        }
        
        /* More independent computations between loop iterations */
        /* These create additional register pressure */
        v8 = v8 * 3 + 1;
        v9 = v9 / 2 + v10;
        v10 = v10 ^ v11;
        v11 = v11 | v12;
        v12 = v12 & v13;
        v13 = v13 << 1;
        v14 = v14 >> 1;
        v15 = v15 + v16;
        v16 = v16 - v17;
        v17 = v17 * v18;
        v18 = v18 ^ 0x5555;
        v19 = v19 | 0xAAAA;
        v20 = v20 & 0x3333;
        v21 = v21 + 0x1111;
        v22 = v22 - 0x2222;
        v23 = v23 * 3;
        v24 = v24 / 3;
        v25 = v25 << 2;
        v26 = v26 >> 2;
        v27 = v27 ^ v28;
        v28 = v28 | v29;
        v29 = v29 & v30;
        v30 = v30 + v31;
        v31 = v31 - v0;
    }
    
    /* Final use of rematerialization candidates to ensure they stay live */
    sum += cand1 + cand2 + cand3 + cand4 + cand5 + cand6 + cand7 + cand8;
    
    /* Use all variables in final computation to prevent dead code elimination */
    sum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    sum += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    sum += v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
    sum += v24 + v25 + v26 + v27 + v28 + v29 + v30 + v31;
    
    return sum;
}

/* Helper function to create more complex call graph */
static int __attribute__((noinline))
process_with_branches(int *inputs, int selector) 
{
    int result = 0;
    
    /* Different control flow paths to complicate liveness analysis */
    if (selector & 1) {
        for (int i = 0; i < 10; i++) {
            /* Create more remat candidates with different expressions */
            int r1 = inputs[i] + i * 17;
            int r2 = inputs[i+1] & (0xFF << (i % 8));
            int r3 = inputs[i+2] << (i & 7);
            int r4 = inputs[i+3] ^ (i * 0x12345);
            
            /* Keep them live across loop iterations */
            result += r1 + r2 + r3 + r4;
            
            /* Additional computations to increase pressure */
            for (int j = 0; j < 3; j++) {
                int t1 = r1 * j;
                int t2 = r2 / (j + 1);
                int t3 = r3 | j;
                int t4 = r4 & j;
                result += t1 + t2 + t3 + t4;
            }
        }
    } else {
        /* Alternative path with different computations */
        for (int i = 0; i < 8; i += 2) {
            int s1 = inputs[i] * inputs[i+1];
            int s2 = inputs[i+2] - inputs[i+3];
            int s3 = inputs[i+4] | inputs[i+5];
            int s4 = inputs[i+6] & inputs[i+7];
            
            result += s1 * s2 + s3 - s4;
        }
    }
    
    return result;
}

int main(void) 
{
    /* Initialize input array with non-constant values */
    int inputs[NUM_VARS];
    
    /* Fill with pseudo-random but deterministic values */
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Call high-pressure function multiple times to increase optimization opportunities */
    int total = 0;
    for (int iter = 0; iter < 3; iter++) {
        total += high_pressure_computation(inputs);
        total += process_with_branches(inputs, iter);
        
        /* Modify inputs slightly between iterations */
        for (int i = 0; i < NUM_VARS; i++) {
            inputs[i] = (inputs[i] * 1664525 + 1013904223) & 0x7FFFFFFF;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
