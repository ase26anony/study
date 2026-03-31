/* test_mcf_coverage.c - Trigger MCF special node printing in GCC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    /* Prevent optimization */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
}

/* Complex function with high register pressure and control flow */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed - 5;
    int v4 = seed ^ 0x1234;
    int v5 = seed | 0xABCD;
    int v6 = seed & 0xF0F0;
    int v7 = seed << 3;
    int v8 = seed >> 2;
    int v9 = ~seed;
    int v10 = seed % 17;
    
    float f1 = seed * 0.1f;
    float f2 = seed * 0.2f;
    float f3 = seed * 0.3f;
    float f4 = seed * 0.4f;
    float f5 = seed * 0.5f;
    
    double d1 = seed * 0.01;
    double d2 = seed * 0.02;
    double d3 = seed * 0.03;
    double d4 = seed * 0.04;
    double d5 = seed * 0.05;
    
    char* s1 = (char*)malloc(32);
    char* s2 = (char*)malloc(32);
    char* s3 = (char*)malloc(32);
    char* s4 = (char*)malloc(32);
    
    /* Initialize strings to prevent optimization */
    sprintf(s1, "str%d", seed);
    sprintf(s2, "str%d", seed + 1);
    sprintf(s3, "str%d", seed + 2);
    sprintf(s4, "str%d", seed + 3);
    
    /* More variables for additional pressure */
    int v11 = v1 + v2;
    int v12 = v3 * v4;
    int v13 = v5 ^ v6;
    int v14 = v7 | v8;
    int v15 = v9 & v10;
    int v16 = v11 << 1;
    int v17 = v12 >> 1;
    int v18 = v13 + v14;
    int v19 = v15 * v16;
    int v20 = v17 ^ v18;
    
    float f6 = f1 + f2;
    float f7 = f3 * f4;
    float f8 = f5 - f6;
    float f9 = f7 / (f8 + 1.0f);
    float f10 = f9 * 2.0f;
    
    double d6 = d1 + d2;
    double d7 = d3 * d4;
    double d8 = d5 - d6;
    double d9 = d7 / (d8 + 1.0);
    double d10 = d9 * 2.0;
    
    /* Complex loop with switch to create control flow */
    int result = 0;
    for (int i = 0; i < iterations; i++) {
        /* Force variables to be live across inline asm */
        asm volatile("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                     "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* Large switch statement for complex CFG */
        switch ((i + seed) % 15) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * 3.14f;
                d1 = d2 / 2.0;
                result += v1;
                break;
            case 1:
                v2 = v3 * v4;
                f2 = f3 + 2.0f;
                d2 = d3 - 1.0;
                result += v2;
                break;
            case 2:
                v3 = v4 ^ v5;
                f3 = f4 / 3.0f;
                d3 = d4 * 1.5;
                result += v3;
                break;
            case 3:
                v4 = v5 | v6;
                f4 = f5 - 0.5f;
                d4 = d5 + 0.25;
                result += v4;
                break;
            case 4:
                v5 = v6 & v7;
                f5 = f1 * f2;
                d5 = d1 / d2;
                result += v5;
                break;
            case 5:
                v6 = v7 << 2;
                f6 = f3 + f4;
                d6 = d3 * d4;
                result += v6;
                break;
            case 6:
                v7 = v8 >> 1;
                f7 = f5 - f6;
                d7 = d5 / d6;
                result += v7;
                break;
            case 7:
                v8 = ~v9;
                f8 = f7 * 2.0f;
                d8 = d7 + 0.1;
                result += v8;
                break;
            case 8:
                v9 = v10 % 13;
                f9 = f8 / 1.5f;
                d9 = d8 * 0.9;
                result += v9;
                break;
            case 9:
                v10 = v11 + v12;
                f10 = f9 + f1;
                d10 = d9 - d1;
                result += v10;
                break;
            case 10:
                v11 = v12 * v13;
                f1 = f10 * f2;
                d1 = d10 * d2;
                result += v11;
                break;
            case 11:
                v12 = v13 ^ v14;
                f2 = f1 / f3;
                d2 = d1 + d3;
                result += v12;
                break;
            case 12:
                v13 = v14 | v15;
                f3 = f2 - f4;
                d3 = d2 - d4;
                result += v13;
                break;
            case 13:
                v14 = v15 & v16;
                f4 = f3 * f5;
                d4 = d3 / d5;
                result += v14;
                break;
            case 14:
                v15 = v16 << 1;
                f5 = f4 + f6;
                d5 = d4 * 1.1;
                result += v15;
                break;
        }
        
        /* Call external function to force register saves/restores */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        }
        
        /* More inline asm with clobbered registers */
        asm volatile("" : : : "memory", "r8", "r9", "r10", "r11",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* Additional computations to use all variables */
        v16 = v17 + v18;
        v17 = v19 * v20;
        v18 = v16 ^ v17;
        v19 = v18 | v1;
        v20 = v19 & v2;
        
        f6 = f7 + f8;
        f7 = f9 * f10;
        f8 = f6 - f7;
        f9 = f8 / (f1 + 1.0f);
        f10 = f9 * 3.0f;
        
        d6 = d7 + d8;
        d7 = d9 * d10;
        d8 = d6 - d7;
        d9 = d8 / (d1 + 1.0);
        d10 = d9 * 3.0;
    }
    
    /* Final computation using all variables */
    int final_result = result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                      (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                      (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10 +
                      (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
                      (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10 +
                      strlen(s1) + strlen(s2) + strlen(s3) + strlen(s4);
    
    /* Cleanup */
    free(s1);
    free(s2);
    free(s3);
    free(s4);
    
    return final_result;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    int result = test_function(iterations, seed);
    printf("Result: %d\n", result);
    
    return 0;
}
