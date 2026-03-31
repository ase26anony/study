/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables stay live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    /* Prevent optimization */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
}

/* Main test function with high register pressure and complex CFG */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed - 5;
    int v4 = seed ^ 0x1234;
    int v5 = seed << 2;
    int v6 = seed >> 1;
    int v7 = seed | 0xFF;
    int v8 = seed & 0xFFFF;
    int v9 = seed % 100;
    int v10 = seed + 1000;
    
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
    
    char* s1 = "string1";
    char* s2 = "string2";
    char* s3 = "string3";
    char* s4 = "string4";
    char* s5 = "string5";
    
    /* Additional variables for more pressure */
    int v11 = v1 + v2;
    int v12 = v3 * v4;
    int v13 = v5 ^ v6;
    int v14 = v7 & v8;
    int v15 = v9 + v10;
    int v16 = v1 * v3;
    int v17 = v2 ^ v4;
    int v18 = v5 & v7;
    int v19 = v6 + v8;
    int v20 = v9 * v10;
    
    float f6 = f1 + f2;
    float f7 = f3 * f4;
    float f8 = f5 - f1;
    float f9 = f2 * f3;
    float f10 = f4 / f5;
    
    double d6 = d1 + d2;
    double d7 = d3 * d4;
    double d8 = d5 - d1;
    double d9 = d2 * d3;
    double d10 = d4 / d5;
    
    int result = 0;
    
    /* Complex loop with nested control flow */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers registers */
        asm volatile("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                     "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* Large switch statement creating complex CFG */
        switch (i % 15) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * f3;
                d1 = d2 - d3;
                break;
            case 1:
                v2 = v3 * v4;
                f2 = f3 / f4;
                d2 = d3 + d4;
                break;
            case 2:
                v3 = v4 ^ v5;
                f3 = f4 - f5;
                d3 = d4 * d5;
                break;
            case 3:
                v4 = v5 & v6;
                f4 = f5 + f1;
                d4 = d5 / d1;
                break;
            case 4:
                v5 = v6 | v7;
                f5 = f1 * f2;
                d5 = d1 - d2;
                break;
            case 5:
                v6 = v7 << 2;
                f6 = f2 / f3;
                d6 = d2 * d3;
                break;
            case 6:
                v7 = v8 >> 1;
                f7 = f3 - f4;
                d7 = d3 + d4;
                break;
            case 7:
                v8 = v9 + v10;
                f8 = f4 * f5;
                d8 = d4 / d5;
                break;
            case 8:
                v9 = v10 * v1;
                f9 = f5 + f1;
                d9 = d5 - d1;
                break;
            case 9:
                v10 = v1 ^ v2;
                f10 = f1 / f2;
                d10 = d1 * d2;
                break;
            case 10:
                v11 = v12 + v13;
                f6 = f7 * f8;
                d6 = d7 - d8;
                break;
            case 11:
                v12 = v13 * v14;
                f7 = f8 / f9;
                d7 = d8 + d9;
                break;
            case 12:
                v13 = v14 ^ v15;
                f8 = f9 - f10;
                d8 = d9 * d10;
                break;
            case 13:
                v14 = v15 & v16;
                f9 = f10 + f6;
                d9 = d10 / d6;
                break;
            case 14:
                v15 = v16 | v17;
                f10 = f6 * f7;
                d10 = d6 - d7;
                break;
        }
        
        /* Call external function to keep variables live across call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        }
        
        /* More computations mixing variables */
        v16 = v17 + v18 - v19 * v20;
        f6 = f7 * f8 + f9 / f10;
        d6 = d7 - d8 * d9 + d10;
        
        /* Another inline assembly clobber */
        asm volatile("" : : : "memory", "r8", "r9", "r10", "r11",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v17 += v18 * j;
            f7 += f8 * j;
            d7 += d8 * j;
            
            if (j % 2 == 0) {
                v18 = v19 ^ (j + seed);
                f8 = f9 * (j * 0.1f);
                d8 = d9 / (j + 1.0);
            }
        }
        
        /* Accumulate result to prevent dead code elimination */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                  (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    }
    
    return result;
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
    
    /* Print checksum to prevent optimization */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
