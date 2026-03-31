/* test_mcf_coverage.c - Trigger MCF solver special node printing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variable liveness across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
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
    
    /* Floating point variables to use different register sets */
    float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f;
    float f4 = seed * 1.4f, f5 = seed * 1.5f, f6 = seed * 1.6f;
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3;
    
    /* Pointer variables */
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    
    /* Array to pass pointers to external function */
    int* ptr_array[10];
    
    int result = 0;
    
    /* Complex loop with switch inside */
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
        d2 = d1 * 0.5;
        d3 = d2 - d1;
        
        /* Large switch statement creating complex CFG */
        switch (i % 13) {
            case 0:
                v8 = v16 + v17;
                v9 = v18 - v19;
                /* Inline asm clobbering registers */
                asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx");
                break;
            case 1:
                v10 = v20 * v21;
                v11 = v22 / (v23 ? v23 : 1);
                asm volatile ("" : : : "memory", "esi", "edi");
                break;
            case 2:
                v12 = v24 ^ v25;
                v13 = v26 | v27;
                f4 = f5 + f6;
                break;
            case 3:
                v14 = v28 & v29;
                v15 = v30 ^ v1;
                f5 = f1 - f2;
                break;
            case 4:
                v16 = v2 + v3;
                v17 = v4 - v5;
                d1 = d2 * 1.1;
                break;
            case 5:
                v18 = v6 * v7;
                v19 = v8 / (v9 ? v9 : 1);
                asm volatile ("" : : : "memory", "r8", "r9", "r10", "r11");
                break;
            case 6:
                v20 = v10 ^ v11;
                v21 = v12 | v13;
                f6 = f3 * f4;
                break;
            case 7:
                v22 = v14 & v15;
                v23 = v16 ^ v17;
                d2 = d3 + 2.0;
                break;
            case 8:
                v24 = v18 + v19;
                v25 = v20 - v21;
                asm volatile ("" : : : "memory", "xmm0", "xmm1", "xmm2");
                break;
            case 9:
                v26 = v22 * v23;
                v27 = v24 / (v25 ? v25 : 1);
                f1 = f5 + f6;
                break;
            case 10:
                v28 = v26 ^ v27;
                v29 = v28 & v29;
                d3 = d1 - d2;
                break;
            case 11:
                v30 = v1 + v2;
                v1 = v3 - v4;
                asm volatile ("" : : : "memory", "xmm3", "xmm4", "xmm5");
                break;
            case 12:
                v2 = v5 * v6;
                v3 = v7 / (v8 ? v8 : 1);
                f2 = f4 - f3;
                break;
        }
        
        /* Prepare pointers for external call */
        ptr_array[0] = &v1; ptr_array[1] = &v2; ptr_array[2] = &v3;
        ptr_array[3] = &v4; ptr_array[4] = &v5; ptr_array[5] = &v6;
        ptr_array[6] = &v7; ptr_array[7] = &v8; ptr_array[8] = &v9;
        ptr_array[9] = &v10;
        
        /* Call external function - forces variables to be live across call */
        use_vars(ptr_array, 10);
        
        /* More computations after call */
        v1 = v1 ^ v2;
        v2 = v2 + v3;
        v3 = v3 * v4;
        v4 = v4 - v5;
        
        f1 = f1 + f2;
        f2 = f2 * f3;
        f3 = f3 - f4;
        
        d1 = d1 + d2;
        d2 = d2 * d3;
        d3 = d3 - d1;
        
        /* Another asm clobber */
        asm volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx",
                                     "rsi", "rdi", "r8", "r9", "r10", "r11");
    }
    
    /* Combine all variables into final result */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
             v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
             v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
             (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
             (int)d1 + (int)d2 + (int)d3;
    
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
