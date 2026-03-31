#include <stdio.h>
#include <stdlib.h>

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
    
    /* Phase 1: Create rematerialization candidates outside loops */
    /* These are pure functions of their inputs - cheap to recompute */
    int cand1 = v0 + 0x1234;      /* Candidate 1: v0 + constant */
    int cand2 = v1 & 0xFFFF00FF;  /* Candidate 2: v1 & mask */
    int cand3 = v2 << 3;          /* Candidate 3: v2 << shift */
    int cand4 = v3 * 7;           /* Candidate 4: v3 * small constant */
    int cand5 = v4 ^ 0xAA55AA55;  /* Candidate 5: v4 ^ constant */
    int cand6 = v5 - 0x1000;      /* Candidate 6: v5 - constant */
    int cand7 = v6 | 0x00FF00FF;  /* Candidate 7: v6 | mask */
    int cand8 = v7 + v8;          /* Candidate 8: v7 + v8 */
    
    /* Keep these candidates live across many operations */
    /* by using them in final computation */
    
    int result = 0;
    
    /* Outer loop with complex control flow */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use candidates inside loop - forcing them to stay live */
        int temp = cand1 + cand2;
        
        /* Inner loop with different induction variable */
        for (int j = 0; j < 5; j++) {
            /* Perform many independent computations to increase pressure */
            int t1 = v9 * v10 + v11;
            int t2 = v12 / (v13 + 1) - v14;
            int t3 = v15 << (v16 & 3);
            int t4 = v17 ^ v18 ^ v19;
            int t5 = v20 | v21 | v22;
            int t6 = v23 - v24 * 2;
            int t7 = v25 + (v26 >> 1);
            int t8 = v27 & v28 & v29;
            
            /* Use candidates inside inner loop */
            t1 += cand3;
            t2 ^= cand4;
            t3 |= cand5;
            t4 -= cand6;
            
            /* Conditional branch creating merge points */
            if ((i + j) & 1) {
                /* Use different sets of variables */
                t5 += v0 * v1;
                t6 ^= v2 | v3;
                t7 -= v4 & v5;
                t8 |= v6 ^ v7;
                
                /* Use more candidates */
                t5 += cand7;
                t6 ^= cand8;
            } else {
                /* Alternative path with different computations */
                t5 += v8 * v9;
                t6 ^= v10 | v11;
                t7 -= v12 & v13;
                t8 |= v14 ^ v15;
                
                /* Still use candidates */
                t5 -= cand1;
                t6 |= cand2;
            }
            
            /* Combine all temporaries - ensures they're not dead */
            temp += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
            
            /* Modify some variables to prevent CSE */
            v9 = (v9 * 1103515245 + 12345) & 0x7FFFFFFF;
            v10 = (v10 * 1664525 + 1013904223) & 0x7FFFFFFF;
        }
        
        /* Use candidates in outer loop computation */
        result += temp + cand1 - cand2 + cand3 ^ cand4;
        
        /* More independent computations between loop iterations */
        int a = v0 * 3 + v1 * 5;
        int b = v2 / 2 - v3 * 7;
        int c = v4 << 1 | v5 >> 2;
        int d = v6 ^ v7 ^ v8;
        int e = v9 + v10 - v11;
        int f = v12 & v13 & v14;
        int g = v15 | v16 | v17;
        int h = v18 - v19 + v20;
        
        /* Use all these to ensure they stay live */
        result ^= a + b + c + d + e + f + g + h;
        
        /* Modify base variables to prevent optimization */
        v0 = (v0 + 1) & 0xFFF;
        v1 = (v1 * 3) & 0xFFF;
        v2 = (v2 ^ 0x555) & 0xFFF;
        v3 = (v3 - 1) & 0xFFF;
    }
    
    /* Final use of all candidates to ensure they're live until the end */
    result = result + cand1 + cand2 + cand3 + cand4 + 
                    cand5 + cand6 + cand7 + cand8;
    
    return result;
}

/* Wrapper to add another layer of control flow */
static int __attribute__((noinline))
compute_hash(int *data, int size) 
{
    int hash = 0xDEADBEEF;
    
    for (int i = 0; i < size; i += 2) {
        /* Create local pressure in wrapper too */
        int x = data[i];
        int y = data[i + 1];
        
        /* Call high-pressure function with different inputs */
        int slice[30];
        for (int j = 0; j < 30; j++) {
            slice[j] = data[(i + j) % size] + j;
        }
        
        int partial = high_pressure_computation(slice);
        
        /* Complex merging of results */
        if (i % 4 == 0) {
            hash = (hash << 3) ^ partial;
        } else if (i % 4 == 1) {
            hash = (hash >> 1) | partial;
        } else if (i % 4 == 2) {
            hash = hash + partial * 0x9E3779B9;
        } else {
            hash = hash ^ partial;
        }
        
        /* More computations to keep values live */
        int t1 = x * y;
        int t2 = x + y;
        int t3 = x ^ y;
        int t4 = x - y;
        
        hash += t1 + t2 + t3 + t4;
    }
    
    return hash;
}

int main() 
{
    /* Initialize with pseudo-random but deterministic data */
    int data[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Perform computation that should trigger early remat */
    int result = compute_hash(data, NUM_VARS);
    
    printf("Result: %d (0x%08X)\n", result, result);
    
    /* Verify with simple computation to ensure correctness */
    int verify = 0;
    for (int i = 0; i < NUM_VARS; i++) {
        verify ^= data[i] * (i + 1);
    }
    printf("Verify: %d (0x%08X)\n", verify, verify);
    
    return 0;
}
