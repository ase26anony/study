/* test_mcf_coverage.c - Trigger MCF solver special node printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variable liveness across calls */
void __attribute__((noinline)) use_vars(int *a, float *b, double *c) {
    volatile int sink = *a;
    sink += (int)*b;
    sink += (int)*c;
    (void)sink;
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
    
    float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f, f4 = seed * 1.4f;
    float f5 = seed * 1.5f, f6 = seed * 1.6f, f7 = seed * 1.7f, f8 = seed * 1.8f;
    
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3, d4 = seed * 2.4;
    double d5 = seed * 2.5, d6 = seed * 2.6, d7 = seed * 2.7, d8 = seed * 2.8;
    
    char *p1 = (char*)&v1, *p2 = (char*)&v2, *p3 = (char*)&v3, *p4 = (char*)&v4;
    
    int result = 0;
    
    /* Complex loop with nested control flow */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers - forces spills */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                                       "esi", "edi", "xmm0", "xmm1", "xmm2");
        
        /* Large switch statement creating complex CFG */
        switch (i % 12) {
            case 0:
                v1 = v2 + v3; f1 = f2 + f3; d1 = d2 + d3;
                use_vars(&v1, &f1, &d1);
                break;
            case 1:
                v4 = v5 * v6; f4 = f5 * f6; d4 = d5 * d6;
                asm volatile ("" : : : "xmm3", "xmm4", "xmm5");
                break;
            case 2:
                v7 = v8 ^ v9; f7 = f8 - f9; d7 = d8 - d9;
                use_vars(&v7, &f7, &d7);
                break;
            case 3:
                v10 = v11 | v12; f10 = f11 / 2.0f; d10 = d11 / 2.0;
                break;
            case 4:
                v13 = v14 & v15; f13 = f14 * 3.0f; d13 = d14 * 3.0;
                asm volatile ("" : : : "xmm6", "xmm7", "xmm8", "xmm9");
                break;
            case 5:
                v16 = v17 << 2; f16 = f17 + 1.0f; d16 = d17 + 1.0;
                use_vars(&v16, &f16, &d16);
                break;
            case 6:
                v18 = v19 >> 1; f18 = f19 - 2.0f; d18 = d19 - 2.0;
                break;
            case 7:
                v20 = ~v1; f20 = -f1; d20 = -d1;
                asm volatile ("" : : : "xmm10", "xmm11", "xmm12", "xmm13");
                break;
            case 8:
                v2 = v3 + v4; f2 = f3 + f4; d2 = d3 + d4;
                use_vars(&v2, &f2, &d2);
                break;
            case 9:
                v5 = v6 * v7; f5 = f6 * f7; d5 = d6 * d7;
                break;
            case 10:
                v8 = v9 ^ v10; f8 = f9 - f10; d8 = d9 - d10;
                asm volatile ("" : : : "xmm14", "xmm15");
                break;
            case 11:
                v11 = v12 | v13; f11 = f12 / 3.0f; d11 = d12 / 3.0;
                use_vars(&v11, &f11, &d11);
                break;
        }
        
        /* Additional computation mixing all variables */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        result += (int)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8);
        result += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            int temp = v1 + j;
            float ftemp = f1 + j;
            double dtemp = d1 + j;
            
            if (temp % 2) {
                v1 = temp ^ v2;
                f1 = ftemp * f2;
                d1 = dtemp * d2;
            } else {
                v1 = temp & v3;
                f1 = ftemp / f3;
                d1 = dtemp / d3;
            }
        }
    }
    
    /* Final aggregation to prevent dead code elimination */
    result = (result + v1 + v2 + v3 + v4 + v5) ^ 
             (v6 + v7 + v8 + v9 + v10) ^ 
             (v11 + v12 + v13 + v14 + v15) ^ 
             (v16 + v17 + v18 + v19 + v20);
    
    return result;
}

int main(int argc, char **argv) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) seed = atoi(argv[2]);
    
    int result = test_function(iterations, seed);
    
    printf("Result: %d\n", result);
    
    /* Use result to affect return code */
    return result % 256;
}
