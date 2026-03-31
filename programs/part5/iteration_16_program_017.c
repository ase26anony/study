#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent optimization */
static int __attribute__((noinline)) 
high_pressure_computation(int *inputs) 
{
    /* Declare many variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31;
    
    /* Initialize from inputs to prevent constant propagation */
    v0 = inputs[0];   v1 = inputs[1];   v2 = inputs[2];   v3 = inputs[3];
    v4 = inputs[4];   v5 = inputs[5];   v6 = inputs[6];   v7 = inputs[7];
    v8 = inputs[8];   v9 = inputs[9];   v10 = inputs[10]; v11 = inputs[11];
    v12 = inputs[12]; v13 = inputs[13]; v14 = inputs[14]; v15 = inputs[15];
    v16 = inputs[16]; v17 = inputs[17]; v18 = inputs[18]; v19 = inputs[19];
    v20 = inputs[20]; v21 = inputs[21]; v22 = inputs[22]; v23 = inputs[23];
    v24 = inputs[24]; v25 = inputs[25]; v26 = inputs[26]; v27 = inputs[27];
    v28 = inputs[28]; v29 = inputs[29]; v30 = inputs[30]; v31 = inputs[31];
    
    /* Phase 1: Create rematerialization candidates outside loops */
    /* These are pure computations that can be recomputed cheaply */
    int cand1 = v0 + 0x1234;      /* Simple addition - remat candidate */
    int cand2 = v1 & 0xFFFF00FF;  /* Bitwise AND - remat candidate */
    int cand3 = v2 << 3;          /* Shift - remat candidate */
    int cand4 = v3 ^ 0xAA55AA55;  /* XOR - remat candidate */
    int cand5 = v4 * 7;           /* Multiplication by constant - remat candidate */
    int cand6 = v5 | 0x00FF00FF;  /* Bitwise OR - remat candidate */
    int cand7 = v6 - 0x1000;      /* Subtraction - remat candidate */
    int cand8 = v7 + v8;          /* Addition of two vars - remat candidate */
    
    /* Keep these candidates live by using them in complex control flow */
    int result = 0;
    
    /* Outer loop with many live values */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use candidates early in loop - forcing them to stay live */
        int temp = cand1 + cand2;
        
        /* Inner loop with conditional that uses different variable sets */
        for (int j = 0; j < 5; j++) {
            /* Complex computation using many variables */
            int comp1 = v9 * v10 + v11 - v12;
            int comp2 = v13 & v14 | v15 ^ v16;
            int comp3 = (v17 << 2) + (v18 >> 1);
            int comp4 = v19 * 3 + v20 * 5 - v21;
            
            /* Use candidates inside inner loop */
            if (j % 2 == 0) {
                temp += cand3 + cand4 + comp1;
            } else {
                temp += cand5 + cand6 + comp2;
            }
            
            /* More computations to increase pressure */
            int comp5 = v22 + v23 * 2 + v24 / 2;
            int comp6 = v25 | v26 & v27 ^ v28;
            int comp7 = (v29 << 1) + (v30 >> 2) * 3;
            int comp8 = v31 * 7 - v0 * 3 + v1;
            
            /* Conditional that uses different sets of variables */
            if (temp > 1000) {
                temp -= comp3 + comp5 + cand7;
            } else {
                temp += comp4 + comp6 + cand8;
            }
            
            /* Use all computations to prevent elimination */
            result += temp + comp7 + comp8;
        }
        
        /* Modify some variables to prevent loop-invariant code motion */
        v9 = v9 + 1;
        v10 = v10 - 1;
        v11 = v11 ^ i;
        v12 = v12 | (i & 0xFF);
    }
    
    /* Final computation using all candidates and variables */
    result = result + cand1 * 2 - cand2 / 4 + cand3 + cand4 - cand5 + cand6 * 3 + cand7 - cand8;
    
    /* Use all variables in final result to keep them live */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    result += v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    result += v17 + v18 + v19 + v20 + v21 + v22 + v23 + v24;
    result += v25 + v26 + v27 + v28 + v29 + v30 + v31;
    
    return result;
}

/* Another function to create more complex call graph */
static int __attribute__((noinline))
nested_pressure(int *inputs, int selector) 
{
    int a = inputs[selector % 32];
    int b = inputs[(selector + 1) % 32];
    int c = inputs[(selector + 2) % 32];
    
    /* Create more remat candidates */
    int r1 = a + 0x100;
    int r2 = b & 0xFF00FF;
    int r3 = c << 2;
    
    /* Complex conditional with many live values */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        if (i % 3 == 0) {
            sum += r1 + a * i;
        } else if (i % 3 == 1) {
            sum += r2 + b * (i & 0xF);
        } else {
            sum += r3 + c * (i >> 1);
        }
        
        /* Additional computations to increase pressure */
        int t1 = a * b + c;
        int t2 = (a & b) | (c ^ i);
        int t3 = (a << 1) + (b >> 1) - c;
        
        sum += t1 + t2 + t3;
        
        /* Modify variables slightly */
        a = a ^ (i & 0xFF);
        b = b + (i % 7);
        c = c - (i % 5);
    }
    
    return sum + r1 - r2 + r3;
}

int main() 
{
    /* Initialize with non-constant values */
    int inputs[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = rand() % 1000 + 1;
    }
    
    /* Perform computation with high register pressure */
    int result1 = high_pressure_computation(inputs);
    
    /* Additional nested computation */
    int result2 = nested_pressure(inputs, result1 % 32);
    
    /* Final result uses both computations */
    int final_result = result1 + result2 * 2;
    
    printf("Result: %d\n", final_result);
    
    /* Verify with deterministic check */
    if (final_result != 0) {
        printf("Computation successful (non-zero result)\n");
    }
    
    return 0;
}
