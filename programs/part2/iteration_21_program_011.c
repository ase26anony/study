/* test-mcf-special-nodes.c
 * This program creates high register pressure and complex control flow
 * to trigger GCC's MCF solver during register allocation, specifically
 * exercising the special node printing logic in mcf.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables stay live across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
    }
    /* Prevent optimization */
    asm volatile("" : : "r"(sum) : "memory");
}

/* Another external function to create more call pressure */
float __attribute__((noinline)) compute_float(float a, float b, float c) {
    volatile float result = a * b + c;
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* Main test function with high register pressure and complex CFG */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed + 5, v6 = seed + 6, v7 = seed + 7, v8 = seed + 8;
    int v9 = seed + 9, v10 = seed + 10, v11 = seed + 11, v12 = seed + 12;
    int v13 = seed + 13, v14 = seed + 14, v15 = seed + 15, v16 = seed + 16;
    int v17 = seed + 17, v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    
    /* Float variables to use floating point registers */
    float f1 = seed * 0.1f, f2 = seed * 0.2f, f3 = seed * 0.3f;
    float f4 = seed * 0.4f, f5 = seed * 0.5f, f6 = seed * 0.6f;
    float f7 = seed * 0.7f, f8 = seed * 0.8f, f9 = seed * 0.9f;
    
    /* Double variables for more pressure */
    double d1 = seed * 1.1, d2 = seed * 1.2, d3 = seed * 1.3;
    double d4 = seed * 1.4, d5 = seed * 1.5;
    
    /* Pointer variables */
    int* p1 = &v1, *p2 = &v2, *p3 = &v3, *p4 = &v4, *p5 = &v5;
    int* ptrs[20];
    
    /* Initialize pointer array */
    ptrs[0] = &v1; ptrs[1] = &v2; ptrs[2] = &v3; ptrs[3] = &v4; ptrs[4] = &v5;
    ptrs[5] = &v6; ptrs[6] = &v7; ptrs[7] = &v8; ptrs[8] = &v9; ptrs[9] = &v10;
    ptrs[10] = &v11; ptrs[11] = &v12; ptrs[12] = &v13; ptrs[13] = &v14;
    ptrs[14] = &v15; ptrs[15] = &v16; ptrs[16] = &v17; ptrs[17] = &v18;
    ptrs[18] = &v19; ptrs[19] = &v20;
    
    /* Complex loop with nested control flow */
    for (int i = 0; i < iterations; i++) {
        /* Large switch statement to create complex CFG */
        switch ((i + seed) % 12) {
            case 0:
                v1 = v2 + v3;
                f1 = compute_float(f2, f3, f4);
                /* Clobber registers with inline asm */
                asm volatile("" : : : "eax", "ebx", "ecx", "edx", "memory");
                break;
            case 1:
                v4 = v5 * v6;
                d1 = d2 - d3;
                asm volatile("" : : : "esi", "edi", "ebp", "memory");
                break;
            case 2:
                v7 = v8 ^ v9;
                f2 = f3 * f4;
                asm volatile("" : : : "xmm0", "xmm1", "xmm2", "memory");
                break;
            case 3:
                v10 = v11 | v12;
                d2 = d3 / d4;
                asm volatile("" : : : "xmm3", "xmm4", "xmm5", "memory");
                break;
            case 4:
                v13 = v14 & v15;
                f3 = f4 + f5;
                /* Call external function with many live variables */
                use_vars(ptrs, 10);
                break;
            case 5:
                v16 = v17 << 2;
                d3 = d4 * d5;
                asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
                break;
            case 6:
                v18 = v19 >> 1;
                f4 = f5 - f6;
                use_vars(ptrs + 10, 10);
                break;
            case 7:
                v20 = v1 + v2;
                d4 = d5 + d1;
                asm volatile("" : : : "r8", "r9", "r10", "r11", "memory");
                break;
            case 8:
                v2 = v3 - v4;
                f5 = f6 * f7;
                asm volatile("" : : : "r12", "r13", "r14", "r15", "memory");
                break;
            case 9:
                v5 = v6 / (v7 + 1);
                d5 = d1 - d2;
                f6 = compute_float(f7, f8, f9);
                break;
            case 10:
                v8 = v9 % (v10 + 1);
                f7 = f8 / f9;
                asm volatile("" : : : "xmm6", "xmm7", "xmm8", "xmm9", "memory");
                break;
            case 11:
                v11 = v12 * v13;
                f8 = f9 + f1;
                use_vars(ptrs + 5, 15);
                break;
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 5 == 0) {
                v1 += v2;
                f1 += f2;
                d1 += d2;
            } else if ((i + j) % 5 == 1) {
                v3 -= v4;
                f3 -= f4;
                d3 -= d4;
            } else if ((i + j) % 5 == 2) {
                v5 *= v6;
                f5 *= f6;
                d5 *= d1;
            } else if ((i + j) % 5 == 3) {
                v7 /= (v8 + 1);
                f7 /= (f8 + 0.1f);
                d2 /= (d3 + 0.1);
            } else {
                v9 = v10 ^ v11;
                f9 = f1 - f2;
                d4 = d5 * 0.5;
            }
        }
        
        /* Conditional with many live variables */
        if (i % 7 == 0) {
            /* Another call with different arguments */
            f1 = compute_float(f3, f5, f7);
            asm volatile("" : : : "xmm10", "xmm11", "xmm12", "xmm13", "memory");
        }
    }
    
    /* Compute checksum from all variables to prevent elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                   (int)f6 + (int)f7 + (int)f8 + (int)f9 +
                   (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    
    return checksum;
}

int main(int argc, char* argv[]) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF test with iterations=%d, seed=%d\n", iterations, seed);
    
    int result = test_function(iterations, seed);
    
    printf("Result checksum: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Zero result (unlikely)\n");
    }
    
    return 0;
}
