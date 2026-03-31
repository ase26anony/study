/* Compile with: gcc -O2 -fno-omit-frame-pointer -fdump-rtl-early_remat -S -o test.s test.c */
/* For even higher pressure: gcc -O3 -funroll-loops -fno-schedule-insns -m32 -fdump-rtl-early_remat -S test.c */

#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

/* Core function with high register pressure - marked noinline to prevent simplification */
static int __attribute__((noinline)) 
high_pressure_computation(int *inputs) 
{
    /* Declare many distinct variables to create register pressure */
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
    
    /* Phase 1: Create rematerialization candidates - pure computations kept live */
    /* These are cheap to recompute but will have long live ranges */
    int cand1 = (v0 + 12345) * (v1 - 6789);      /* Candidate 1 */
    int cand2 = (v2 & 0xFFFF) | (v3 << 8);       /* Candidate 2 */
    int cand3 = (v4 ^ 0xAAAAAAAA) + (v5 * 3);    /* Candidate 3 */
    int cand4 = (v6 >> 4) | (v7 << 28);          /* Candidate 4 */
    int cand5 = (v8 % 17) + (v9 & 0xF0F0F0F0);   /* Candidate 5 */
    
    /* Phase 2: Perform many independent computations to create register pressure */
    /* Each computation uses different combinations to avoid CSE */
    int r0 = v10 * v11 + v12 - v13;
    int r1 = v14 / (v15 + 1) + v16 * 2;
    int r2 = (v17 << 3) | (v18 >> 5);
    int r3 = v19 ^ v20 ^ v21 ^ 0x12345678;
    int r4 = v22 * 7 + v23 * 13 - v24;
    int r5 = (v25 & v26) | (v27 & ~v28);
    int r6 = v29 * v30 - v31 * 2;
    int r7 = (v0 << 1) + (v1 >> 1) - v2;
    int r8 = v3 * v4 / (v5 + 2) + v6;
    int r9 = (v7 ^ v8) & (v9 | v10);
    int r10 = v11 * 3 + v12 * 5 - v13 * 7;
    int r11 = (v14 & 0xFF) << 24 | (v15 & 0xFF) << 16;
    int r12 = v16 * v17 + v18 * v19 - v20;
    int r13 = (v21 >> v22) | (v23 << v24);
    int r14 = v25 ^ 0xDEADBEEF + v26;
    int r15 = v27 * 11 + v28 * 13 - v29 * 17;
    
    /* Phase 3: Complex control flow with nested loops */
    /* The candidates are used inside loops, creating long live ranges */
    int outer_acc = 0;
    int inner_acc = 0;
    
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Use candidates inside loop - they may need rematerialization */
        if (i % 3 == 0) {
            outer_acc += cand1 + (i & 0xFF);
        } else if (i % 3 == 1) {
            outer_acc += cand2 ^ (i << 2);
        } else {
            outer_acc += cand3 | (i * 7);
        }
        
        /* Inner loop with more computations */
        for (int j = 0; j < 5; j++) {
            /* Use different sets of variables in conditional branches */
            if ((i + j) % 2 == 0) {
                inner_acc += r0 + r1 + r2 + cand4 * j;
                inner_acc ^= r3 + (cand5 >> j);
            } else {
                inner_acc += r4 + r5 + r6 - cand1 * j;
                inner_acc |= r7 + (cand2 << (j & 3));
            }
            
            /* More computations to keep values live */
            int t0 = r8 + r9 * j;
            int t1 = r10 - r11 / (j + 1);
            int t2 = r12 ^ r13 | (j * 11);
            int t3 = r14 + r15 * (j + 2);
            
            inner_acc += t0 + t1 + t2 + t3;
            
            /* Use candidates again */
            if (j % 2 == 0) {
                inner_acc ^= cand3 + (i * j);
            } else {
                inner_acc &= ~(cand4 - (i >> j));
            }
        }
        
        /* More computations between loop iterations */
        r0 = r0 ^ (i * 3);
        r1 = r1 + (i % 7);
        r2 = r2 | (0x55 << (i & 7));
        r3 = r3 - (i * 5);
        r4 = r4 ^ (0xAA >> (i & 7));
        r5 = r5 + (i * 11);
    }
    
    /* Phase 4: Final combination using all live values */
    /* This ensures nothing gets optimized away as dead */
    int final_result = 0;
    
    /* Use all candidates one last time */
    final_result += cand1 + cand2 + cand3 + cand4 + cand5;
    
    /* Use all computation results */
    final_result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
    final_result += r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    
    /* Combine with loop accumulators */
    final_result += outer_acc * 3 + inner_acc / 2;
    
    /* Use original variables one more time */
    final_result ^= v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    final_result |= v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    final_result &= v16 + v17 + v18 + v19 + v20 + v21 + v22 + v23;
    final_result += v24 * v25 + v26 * v27 - v28 * v29 + v30 ^ v31;
    
    return final_result;
}

/* Wrapper to create different execution paths */
static int __attribute__((noinline))
complex_wrapper(int *inputs, int selector) 
{
    int result = high_pressure_computation(inputs);
    
    /* Create different control flow paths */
    if (selector % 4 == 0) {
        return result ^ 0x12345678;
    } else if (selector % 4 == 1) {
        return result + 0x87654321;
    } else if (selector % 4 == 2) {
        return result * 3;
    } else {
        return result | 0xAAAAAAAA;
    }
}

int main() 
{
    /* Initialize with non-zero values to prevent constant folding */
    int inputs[NUM_VARS];
    for (int i = 0; i < NUM_VARS; i++) {
        inputs[i] = (i * 37 + 123) & 0x7FFFFFFF;
    }
    
    /* Call multiple times with different selectors */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += complex_wrapper(inputs, i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
