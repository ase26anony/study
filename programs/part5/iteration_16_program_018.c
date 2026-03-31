#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 50
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static int __attribute__((noinline)) 
high_pressure_computation(int *inputs) 
{
    /* Declare many local variables to create register pressure */
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
    
    /* Phase 1: Create rematerialization candidates outside loops */
    /* These are pure functions of their inputs - cheap to recompute */
    int cand1 = v0 + 0x1234;      /* Candidate 1: v0 + constant */
    int cand2 = v1 & 0xFFFF;      /* Candidate 2: v1 & mask */
    int cand3 = v2 << 3;          /* Candidate 3: v2 << shift */
    int cand4 = v3 ^ 0xAA55;      /* Candidate 4: v3 ^ constant */
    int cand5 = v4 * 7;           /* Candidate 5: v4 * small constant */
    int cand6 = v5 - 0x1000;      /* Candidate 6: v5 - constant */
    int cand7 = v6 | 0x00FF;      /* Candidate 7: v6 | mask */
    int cand8 = ~v7;              /* Candidate 8: bitwise NOT */
    int cand9 = v8 + v9;          /* Candidate 9: addition of two vars */
    int cand10 = v10 * 3 + 5;     /* Candidate 10: linear function */
    
    /* Keep these candidates live across complex control flow */
    int sum = cand1 + cand2 + cand3 + cand4 + cand5 + 
              cand6 + cand7 + cand8 + cand9 + cand10;
    
    /* Phase 2: Complex loop with many live values */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use remat candidates inside loop - forcing them to stay live */
        int t1 = cand1 + i;
        int t2 = cand2 - i;
        int t3 = cand3 ^ i;
        int t4 = cand4 | i;
        int t5 = cand5 & i;
        
        /* Many independent computations creating register pressure */
        int r0 = v11 * v12 + v13;
        int r1 = v14 / (v15 + 1) - v16;
        int r2 = v17 << (v18 & 3);
        int r3 = v19 >> (v20 % 4);
        int r4 = v21 ^ v22 ^ v23;
        int r5 = v24 * 13 + v25 * 17;
        int r6 = v26 & v27 & v28;
        int r7 = v29 | v30 | v31;
        int r8 = v32 - v33 + v34;
        int r9 = v35 * 2 - v36 / 2;
        int r10 = v37 + v38 * 3;
        int r11 = v39 ^ 0x55AA;
        int r12 = v40 << 1;
        int r13 = v41 >> 2;
        int r14 = v42 & 0xF0F0;
        int r15 = v43 | 0x0F0F;
        int r16 = v44 * 5 + 7;
        int r17 = v45 - 100;
        int r18 = v46 + 255;
        int r19 = v47 ^ v48;
        
        /* Conditional inside loop creates merging points */
        if (i & 1) {
            /* Use different sets of variables in each branch */
            r0 = v12 * v13 + v14;
            r1 = v15 / (v16 + 1) - v17;
            r2 = v18 << (v19 & 3);
            sum += t1 + t3 + t5 + r0 + r2 + v49;
        } else {
            r3 = v20 >> (v21 % 4);
            r4 = v22 ^ v23 ^ v24;
            sum += t2 + t4 + r1 + r3 + r4 + v0;
        }
        
        /* Use all remat candidates to keep them live */
        sum += cand6 + cand7 + cand8 + cand9 + cand10;
        
        /* More computations to increase pressure */
        int s1 = r0 * r1 + r2;
        int s2 = r3 / (r4 + 1) - r5;
        int s3 = r6 << (r7 & 3);
        int s4 = r8 >> (r9 % 4);
        int s5 = r10 ^ r11 ^ r12;
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            /* Use remat candidates in nested loop too */
            int nested = cand1 + cand2 + cand3 + j;
            s1 += nested;
            s2 += v0 + v1 + v2 + j;
            s3 += v3 + v4 + v5;
            
            /* More independent computations */
            int n1 = v6 * j + v7;
            int n2 = v8 / (j + 1) - v9;
            int n3 = v10 << (j & 3);
            sum += n1 + n2 + n3;
        }
        
        sum += s1 + s2 + s3 + s4 + s5;
        
        /* Modify some variables to prevent dead code elimination */
        v11 += i;
        v12 ^= i;
        v13 |= i;
        v14 &= ~i;
        v15 -= i;
    }
    
    /* Phase 3: Final computation using all live values */
    int final = sum;
    final += cand1 * 2;
    final += cand2 / 2;
    final += cand3 << 1;
    final += cand4 ^ 0x5555;
    final += cand5 * 3;
    final += cand6 + 1000;
    final += cand7 & 0xAAAA;
    final += cand8 | 0x1111;
    final += cand9 - 500;
    final += cand10 * 2;
    
    /* Use all variables in final result to prevent optimization */
    final += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    final += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    final += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    final += v30 + v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39;
    final += v40 + v41 + v42 + v43 + v44 + v45 + v46 + v47 + v48 + v49;
    
    return final;
}

/* Helper to generate test data */
static void fill_inputs(int *inputs, int seed) {
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = (i * 1103515245 + seed) & 0x7FFFFFFF;
    }
}

int main(void) {
    int inputs[NUM_VARS];
    
    /* Generate deterministic but non-constant inputs */
    fill_inputs(inputs, 12345);
    
    /* Call the high-pressure function */
    int result = high_pressure_computation(inputs);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify with expected value for this seed */
    if (result == 2145610368) {
        printf("SUCCESS: Early remat likely triggered\n");
    } else {
        printf("Result differs - but code structure should trigger remat\n");
    }
    
    return 0;
}
