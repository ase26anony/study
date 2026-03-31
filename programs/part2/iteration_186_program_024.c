/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
static volatile int sink;

/* Large test function with high register pressure and complex control flow */
static int __attribute__((noinline)) test_mcf(int seed) {
    /* Declare many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39;
    volatile int result = seed; /* Prevent optimizations */
    
    /* Initialize variables with dependent computations */
    v0 = seed * 1;
    v1 = seed * 2 + v0;
    v2 = seed * 3 + v1;
    v3 = seed * 4 + v2;
    v4 = seed * 5 + v3;
    v5 = seed * 6 + v4;
    v6 = seed * 7 + v5;
    v7 = seed * 8 + v6;
    v8 = seed * 9 + v7;
    v9 = seed * 10 + v8;
    v10 = seed * 11 + v9;
    v11 = seed * 12 + v10;
    v12 = seed * 13 + v11;
    v13 = seed * 14 + v12;
    v14 = seed * 15 + v13;
    v15 = seed * 16 + v14;
    v16 = seed * 17 + v15;
    v17 = seed * 18 + v16;
    v18 = seed * 19 + v17;
    v19 = seed * 20 + v18;
    v20 = seed * 21 + v19;
    v21 = seed * 22 + v20;
    v22 = seed * 23 + v21;
    v23 = seed * 24 + v22;
    v24 = seed * 25 + v23;
    v25 = seed * 26 + v24;
    v26 = seed * 27 + v25;
    v27 = seed * 28 + v26;
    v28 = seed * 29 + v27;
    v29 = seed * 30 + v28;
    v30 = seed * 31 + v29;
    v31 = seed * 32 + v30;
    v32 = seed * 33 + v31;
    v33 = seed * 34 + v32;
    v34 = seed * 35 + v33;
    v35 = seed * 36 + v34;
    v36 = seed * 37 + v35;
    v37 = seed * 38 + v36;
    v38 = seed * 39 + v37;
    v39 = seed * 40 + v38;
    
    /* Complex control flow with many basic blocks */
    if (seed & 1) {
        v0 = v1 + v2;
        /* Inline assembly to force register clobbering */
        asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
    } else {
        v0 = v3 - v4;
        asm volatile("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    }
    
    /* Nested loops with breaks/continues */
    for (int i = 0; i < 10; i++) {
        if (i & 1) {
            v5 = v6 * v7;
            if (v5 > 1000) {
                v8 = v9 / 2;
                continue;
            }
        } else {
            v10 = v11 % v12;
            if (v10 == 0) {
                v13 = v14 | v15;
                break;
            }
        }
        
        /* More register pressure */
        v16 = v17 ^ v18;
        v19 = v20 & v21;
        v22 = v23 << 2;
        v24 = v25 >> 1;
        
        /* Another inline assembly with different clobbers */
        asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    }
    
    /* Large switch statement creating many basic blocks */
    switch (seed % 20) {
        case 0: v26 = v27 + v28; break;
        case 1: v26 = v29 - v30; break;
        case 2: v26 = v31 * v32; break;
        case 3: v26 = v33 / (v34 ? v34 : 1); break;
        case 4: v26 = v35 % (v36 ? v36 : 1); break;
        case 5: v26 = v37 & v38; break;
        case 6: v26 = v39 | v0; break;
        case 7: v26 = v1 ^ v2; break;
        case 8: v26 = v3 << v4; break;
        case 9: v26 = v5 >> v6; break;
        case 10: v26 = v7 + v8 + v9; break;
        case 11: v26 = v10 - v11 - v12; break;
        case 12: v26 = v13 * v14 * v15; break;
        case 13: v26 = v16 / (v17 ? v17 : 1) / (v18 ? v18 : 1); break;
        case 14: v26 = v19 % (v20 ? v20 : 1) % (v21 ? v21 : 1); break;
        case 15: v26 = v22 & v23 & v24; break;
        case 16: v26 = v25 | v26 | v27; break;
        case 17: v26 = v28 ^ v29 ^ v30; break;
        case 18: v26 = ~v31; break;
        case 19: v26 = -v32; break;
        default: v26 = seed;
    }
    
    /* Goto labels creating additional control flow edges */
    if (v26 > 1000000) goto large_value;
    if (v26 < -1000000) goto small_value;
    
    /* Normal path */
    v33 = v34 * v35 + v36;
    goto compute_final;
    
large_value:
    v33 = v37 / 2 + v38;
    goto compute_final;
    
small_value:
    v33 = v39 * 3 - v0;
    /* Fall through */
    
compute_final:
    /* Final computation using all variables */
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
             v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
             v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
             v30 + v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39;
    
    /* One more inline assembly to ensure graph transformations */
    asm volatile("" : "+r"(result) : : "memory", "cc", 
                 "eax", "ebx", "ecx", "edx", "esi", "edi",
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    
    return result;
}

/* Second complex function to increase overall compilation complexity */
static int __attribute__((noinline)) another_complex_func(int x, int y) {
    volatile int a = x, b = y;
    int sum = 0;
    
    /* Loop with complex exit conditions */
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            a = a * 2 + b;
            asm volatile("" : : : "eax", "ebx");
        } else if (i % 3 == 1) {
            b = b * 3 - a;
            asm volatile("" : : : "ecx", "edx");
        } else {
            sum += a * b;
            asm volatile("" : : : "esi", "edi");
        }
        
        /* Early exit with goto */
        if (sum > 1000000) goto early_exit;
    }
    
early_exit:
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += test_mcf(i);
        total += another_complex_func(i, i * 2);
        
        /* Prevent loop unrolling */
        sink = total;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
