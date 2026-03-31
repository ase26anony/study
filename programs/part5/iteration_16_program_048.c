#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 50
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static __attribute__((noinline)) 
unsigned long long high_pressure_computation(int* inputs) {
    /* Declare many distinct variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39;
    int v40, v41, v42, v43, v44, v45, v46, v47, v48, v49;
    
    /* Initialize from inputs to avoid constant propagation */
    v0 = inputs[0]; v1 = inputs[1]; v2 = inputs[2]; v3 = inputs[3]; v4 = inputs[4];
    v5 = inputs[5]; v6 = inputs[6]; v7 = inputs[7]; v8 = inputs[8]; v9 = inputs[9];
    v10 = inputs[10]; v11 = inputs[11]; v12 = inputs[12]; v13 = inputs[13]; v14 = inputs[14];
    v15 = inputs[15]; v16 = inputs[16]; v17 = inputs[17]; v18 = inputs[18]; v19 = inputs[19];
    v20 = inputs[20]; v21 = inputs[21]; v22 = inputs[22]; v23 = inputs[23]; v24 = inputs[24];
    v25 = inputs[25]; v26 = inputs[26]; v27 = inputs[27]; v28 = inputs[28]; v29 = inputs[29];
    v30 = inputs[30]; v31 = inputs[31]; v32 = inputs[32]; v33 = inputs[33]; v34 = inputs[34];
    v35 = inputs[35]; v36 = inputs[36]; v37 = inputs[37]; v38 = inputs[38]; v39 = inputs[39];
    v40 = inputs[40]; v41 = inputs[41]; v42 = inputs[42]; v43 = inputs[43]; v44 = inputs[44];
    v45 = inputs[45]; v46 = inputs[46]; v47 = inputs[47]; v48 = inputs[48]; v49 = inputs[49];
    
    /* Phase 1: Create many rematerialization candidates (pure computations) */
    /* These will have long live ranges and be cheap to recompute */
    int r0 = v0 * 37 + v1;      /* Candidate 1 */
    int r1 = v2 & 0xFF00FF;     /* Candidate 2 */
    int r2 = v3 << 3;           /* Candidate 3 */
    int r3 = v4 + 0xABCD;       /* Candidate 4 */
    int r4 = v5 | 0x12345678;   /* Candidate 5 */
    int r5 = v6 - 999;          /* Candidate 6 */
    int r6 = v7 ^ 0xDEADBEEF;   /* Candidate 7 */
    int r8 = v9 % 17;           /* Candidate 8 */
    int r9 = v10 * 2 + 1;       /* Candidate 9 */
    int r10 = v11 & ~0xF;       /* Candidate 10 */
    
    /* Keep these results live by using them later */
    unsigned long long accumulator = 0;
    
    /* Complex loop structure with conditional branches */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use the remat candidates inside the loop - they're defined outside */
        if (i & 1) {
            /* Branch A: Use some candidates with other variables */
            int t0 = r0 + v12 + i;
            int t1 = r1 | v13;
            int t2 = r2 - v14;
            int t3 = r3 ^ v15;
            int t4 = r4 & v16;
            
            /* More independent computations to increase pressure */
            int t5 = v17 * v18 + v19;
            int t6 = v20 / (v21 + 1);
            int t7 = v22 << (v23 & 3);
            int t8 = v24 ^ v25 ^ v26;
            int t9 = v27 - v28 + v29;
            
            accumulator += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
        } else {
            /* Branch B: Use different candidates and variables */
            int t0 = r5 + v30 * i;
            int t1 = r6 & v31;
            int t2 = r8 ^ v32;
            int t3 = r9 - v33;
            int t4 = r10 | v34;
            
            /* More independent computations */
            int t5 = v35 * 3 + v36;
            int t6 = v37 / 2 - v38;
            int t7 = v39 << 1;
            int t8 = v40 ^ v41 ^ v42;
            int t9 = v43 + v44 - v45;
            
            accumulator += t0 * 2 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
        }
        
        /* Nested loop to further complicate liveness analysis */
        for (int j = 0; j < 3; j++) {
            /* Use even more variables and remat candidates */
            int n0 = r0 + j;
            int n1 = r1 - j;
            int n2 = r2 * (j + 1);
            int n3 = r3 & (0xFF << j);
            int n4 = r4 ^ j;
            
            /* Additional computations with remaining variables */
            int n5 = v46 + v47 * j;
            int n6 = v48 - v49 + j;
            int n7 = (v0 + v1) * (j + 2);
            int n8 = (v2 | v3) << j;
            int n9 = (v4 ^ v5) + j;
            
            accumulator += n0 + n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8 + n9;
        }
        
        /* Modify some variables to prevent dead code elimination */
        v0 += i;
        v1 -= i;
        v2 ^= i;
        v3 |= i;
        v4 &= ~i;
    }
    
    /* Final combination using all remat candidates to ensure they stay live */
    int final_compute = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r8 + r9 + r10;
    
    return accumulator + final_compute;
}

/* Wrapper to add another layer of control flow */
static __attribute__((noinline))
unsigned long long compute_with_branches(int* data, int selector) {
    unsigned long long result = 0;
    
    if (selector & 1) {
        result = high_pressure_computation(data);
    } else {
        /* Alternative path with different computation pattern */
        int temp_data[NUM_VARS];
        for (int i = 0; i < NUM_VARS; i++) {
            temp_data[i] = data[i] ^ selector;
        }
        result = high_pressure_computation(temp_data) + selector;
    }
    
    return result;
}

int main() {
    /* Initialize with non-zero values to avoid constant folding */
    int data[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Call with different selectors to exercise both paths */
    unsigned long long total = 0;
    for (int s = 0; s < 10; s++) {
        total += compute_with_branches(data, s);
    }
    
    printf("Result: %llu\n", total);
    return 0;
}
