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
    
    /* Initialize from inputs to avoid constant propagation */
    v0 = inputs[0];  v1 = inputs[1];  v2 = inputs[2];  v3 = inputs[3];
    v4 = inputs[4];  v5 = inputs[5];  v6 = inputs[6];  v7 = inputs[7];
    v8 = inputs[8];  v9 = inputs[9];  v10 = inputs[10]; v11 = inputs[11];
    v12 = inputs[12]; v13 = inputs[13]; v14 = inputs[14]; v15 = inputs[15];
    v16 = inputs[16]; v17 = inputs[17]; v18 = inputs[18]; v19 = inputs[19];
    v20 = inputs[20]; v21 = inputs[21]; v22 = inputs[22]; v23 = inputs[23];
    v24 = inputs[24]; v25 = inputs[25]; v26 = inputs[26]; v27 = inputs[27];
    v28 = inputs[28]; v29 = inputs[29];
    
    /* Create rematerialization candidates - pure expressions kept live */
    int cand1 = v0 + 0x1234;      /* Cheap to recompute */
    int cand2 = v1 & 0xFFFF;      /* Bitmask operation */
    int cand3 = v2 << 3;          /* Shift operation */
    int cand4 = v3 * 7;           /* Multiplication by small constant */
    int cand5 = v4 ^ 0xAA55;      /* XOR operation */
    int cand6 = v5 - 42;          /* Simple subtraction */
    int cand7 = v6 | 0xFF00;      /* OR operation */
    int cand8 = v7 + v8;          /* Two-variable addition */
    
    /* Complex control flow to extend liveness */
    int sum = 0;
    
    /* Outer loop - cand1..cand8 must stay live through this */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use remat candidates inside loop */
        if (i & 1) {
            sum += cand1 + cand3 + cand5;
        } else {
            sum += cand2 + cand4 + cand6;
        }
        
        /* Nested loop with different computations */
        for (int j = 0; j < 5; j++) {
            /* Many independent computations to increase pressure */
            int t0 = v9 + v10 * j;
            int t1 = v11 - v12 / (j + 1);
            int t2 = v13 << (j & 3);
            int t3 = v14 ^ v15;
            int t4 = v16 | v17;
            int t5 = v18 & v19;
            int t6 = v20 * 3 + j;
            int t7 = v21 - v22 * 2;
            int t8 = v23 / (v24 + 1);
            int t9 = v25 % (v26 + 1);
            
            /* Use remat candidates here too */
            if (j & 1) {
                sum += cand7 + t0 + t2 + t4 + t6 + t8;
            } else {
                sum += cand8 + t1 + t3 + t5 + t7 + t9;
            }
            
            /* More computations to keep values live */
            v9 = v9 ^ t0;
            v10 = v10 + t1;
            v11 = v11 | t2;
            v12 = v12 & t3;
        }
        
        /* Conditional branch using different variable sets */
        if (sum & 1) {
            /* Use another set of variables */
            int u0 = v27 * v28;
            int u1 = v29 << 2;
            int u2 = v0 & v1;
            int u3 = v2 | v3;
            
            sum += u0 + u1 + u2 + u3 + cand1;
        } else {
            /* Alternative path */
            int w0 = v4 ^ v5;
            int w1 = v6 - v7;
            int w2 = v8 * 9;
            int w3 = v9 & 0xF;
            
            sum += w0 + w1 + w2 + w3 + cand2;
        }
        
        /* Rotate values to create data dependencies */
        int tmp = v0;
        v0 = v1; v1 = v2; v2 = v3; v3 = v4; v4 = v5;
        v5 = v6; v6 = v7; v7 = v8; v8 = v9; v9 = v10;
        v10 = v11; v11 = v12; v12 = v13; v13 = v14; v14 = v15;
        v15 = v16; v16 = v17; v17 = v18; v18 = v19; v19 = v20;
        v20 = v21; v21 = v22; v22 = v23; v23 = v24; v24 = v25;
        v25 = v26; v26 = v27; v27 = v28; v28 = v29; v29 = tmp;
    }
    
    /* Final use of all rematerialization candidates */
    return sum + cand1 + cand2 + cand3 + cand4 + cand5 + cand6 + cand7 + cand8;
}

/* Wrapper to ensure computations aren't optimized away */
static int __attribute__((noinline))
compute_hash(int seed) 
{
    int inputs[NUM_VARS];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = seed * 1103515245 + 12345 + i;
    }
    
    return high_pressure_computation(inputs);
}

int main() 
{
    int total = 0;
    
    /* Multiple calls to increase optimization opportunities */
    for (int i = 0; i < 10; i++) {
        total += compute_hash(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
