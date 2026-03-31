/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* arr, int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += arr[i];
    }
    (void)sum;
}

/* Complex function with high register pressure and control flow */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed + 5, v6 = seed + 6, v7 = seed + 7, v8 = seed + 8;
    int v9 = seed + 9, v10 = seed + 10, v11 = seed + 11, v12 = seed + 12;
    int v13 = seed + 13, v14 = seed + 14, v15 = seed + 15, v16 = seed + 16;
    int v17 = seed + 17, v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    int v21 = seed + 21, v22 = seed + 22, v23 = seed + 23, v24 = seed + 24;
    int v25 = seed + 25, v26 = seed + 26, v27 = seed + 27, v28 = seed + 28;
    int v29 = seed + 29, v30 = seed + 30;
    
    /* Floating point variables to use FP registers */
    float f1 = seed * 0.1f, f2 = seed * 0.2f, f3 = seed * 0.3f;
    float f4 = seed * 0.4f, f5 = seed * 0.5f, f6 = seed * 0.6f;
    
    double d1 = seed * 0.01, d2 = seed * 0.02, d3 = seed * 0.03;
    double d4 = seed * 0.04, d5 = seed * 0.05;
    
    /* Pointer variables */
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    
    /* Array to pass to external function */
    int live_vars[10];
    
    /* Complex control flow with loops */
    for (int i = 0; i < iterations; i++) {
        /* Mix computations to create data dependencies */
        v1 = v2 + v3;
        v2 = v4 - v5;
        v3 = v6 * v7;
        v4 = v8 / (v9 ? v9 : 1);
        v5 = v10 ^ v11;
        v6 = v12 | v13;
        v7 = v14 & v15;
        
        f1 = f2 + f3;
        f2 = f4 - f5;
        f3 = f6 * f1;
        
        d1 = d2 + d3;
        d2 = d4 - d5;
        d3 = d1 * d2;
        
        /* Large switch statement creating many basic blocks */
        switch (i % 15) {
            case 0: v8 = v16 + v17; f4 = f1 * 2.0f; d4 = d1 / 2.0; break;
            case 1: v9 = v18 - v19; f5 = f2 / 3.0f; d5 = d2 * 3.0; break;
            case 2: v10 = v20 * v21; f6 = f3 + 4.0f; d1 = d3 - 4.0; break;
            case 3: v11 = v22 ^ v23; f1 = f4 - 5.0f; d2 = d4 + 5.0; break;
            case 4: v12 = v24 | v25; f2 = f5 * 6.0f; d3 = d5 * 6.0; break;
            case 5: v13 = v26 & v27; f3 = f6 / 7.0f; d4 = d1 / 7.0; break;
            case 6: v14 = v28 + v29; f4 = f1 + 8.0f; d5 = d2 - 8.0; break;
            case 7: v15 = v30 - v1; f5 = f2 - 9.0f; d1 = d3 + 9.0; break;
            case 8: v16 = v2 * v3; f6 = f3 * 10.0f; d2 = d4 * 10.0; break;
            case 9: v17 = v4 ^ v5; f1 = f4 / 11.0f; d3 = d5 / 11.0; break;
            case 10: v18 = v6 | v7; f2 = f5 + 12.0f; d4 = d1 + 12.0; break;
            case 11: v19 = v8 & v9; f3 = f6 - 13.0f; d5 = d2 - 13.0; break;
            case 12: v20 = v10 + v11; f4 = f1 * 14.0f; d1 = d3 * 14.0; break;
            case 13: v21 = v12 - v13; f5 = f2 / 15.0f; d2 = d4 / 15.0; break;
            case 14: v22 = v14 * v15; f6 = f3 + 16.0f; d3 = d5 + 16.0; break;
        }
        
        /* Inline assembly that clobbers registers */
        /* For x86: */
        #if defined(__i386__) || defined(__x86_64__)
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                                           "esi", "edi", "xmm0", "xmm1", 
                                           "xmm2", "xmm3", "xmm4", "xmm5");
        #endif
        
        /* For ARM: */
        #if defined(__arm__)
        asm volatile ("" : : : "memory", "r0", "r1", "r2", "r3", 
                                           "r4", "r5", "r6", "r7", 
                                           "r8", "r9", "r10");
        #endif
        
        /* For RISC-V: */
        #if defined(__riscv)
        asm volatile ("" : : : "memory", "t0", "t1", "t2", "t3", 
                                           "t4", "t5", "t6", "a0", 
                                           "a1", "a2", "a3", "a4", 
                                           "a5", "a6", "a7");
        #endif
        
        /* Prepare variables to pass to external function */
        live_vars[0] = v1; live_vars[1] = v2; live_vars[2] = v3;
        live_vars[3] = v4; live_vars[4] = v5; live_vars[5] = v6;
        live_vars[6] = v7; live_vars[7] = v8; live_vars[8] = v9;
        live_vars[9] = v10;
        
        /* Call external function - forces many variables to be live */
        use_vars(live_vars, 10);
        
        /* More computations after call */
        v23 = v16 + v17 + v18;
        v24 = v19 - v20 - v21;
        v25 = v22 * v23 * v24;
        
        f1 = f1 + f2 + f3;
        f2 = f4 - f5 - f6;
        
        d1 = d1 * d2 * d3;
        d2 = d4 / d5 / (d1 ? d1 : 1.0);
        
        /* Another inline assembly barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
                 (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    
    return result;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    int result = test_function(iterations, seed);
    printf("Result: %d\n", result);
    
    return 0;
}
