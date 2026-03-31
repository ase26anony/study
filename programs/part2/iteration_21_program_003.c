/* test_mcf_coverage.c - Trigger MCF solver special node printing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c) {
    volatile int sink = *a + (int)*b + (int)*c;
    (void)sink;
}

/* Complex function with high register pressure */
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
    
    char* p1 = (char*)&v1, *p2 = (char*)&v2, *p3 = (char*)&v3, *p4 = (char*)&v4;
    
    int result = 0;
    
    /* Complex loop with nested control flow */
    for (int i = 0; i < iterations; i++) {
        /* Large switch statement creating many basic blocks */
        switch (i % 13) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                /* Inline assembly clobbering registers */
                asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx");
                break;
            case 1:
                v4 = v5 * v6;
                f4 = f5 * f6;
                d4 = d5 * d6;
                break;
            case 2:
                v7 = v8 - v9;
                f7 = f8 - f9;
                d7 = d8 - d1;
                break;
            case 3:
                v10 = v11 ^ v12;
                f2 = f3 / f4;
                d2 = d3 / d4;
                asm volatile ("" : : : "memory", "esi", "edi");
                break;
            case 4:
                v13 = v14 | v15;
                f5 = f6 * f7;
                d5 = d6 * d7;
                break;
            case 5:
                v16 = v17 & v18;
                f8 = f1 + f2;
                d8 = d1 + d2;
                break;
            case 6:
                v19 = v20 << 2;
                f3 = f4 - f5;
                d3 = d4 - d5;
                break;
            case 7:
                v1 = v2 >> 1;
                f6 = f7 / f8;
                d6 = d7 / d8;
                asm volatile ("" : : : "memory", "r8", "r9", "r10", "r11");
                break;
            case 8:
                v3 = v4 + v5 + v6;
                f1 = f2 * f3 * f4;
                d1 = d2 * d3 * d4;
                break;
            case 9:
                v7 = v8 - v9 - v10;
                f5 = f6 / f7 / f8;
                d5 = d6 / d7 / d8;
                break;
            case 10:
                v11 = v12 * v13 * v14;
                f2 = f3 + f4 + f5;
                d2 = d3 + d4 + d5;
                break;
            case 11:
                v15 = v16 ^ v17 ^ v18;
                f6 = f7 - f8 - f1;
                d6 = d7 - d8 - d1;
                asm volatile ("" : : : "memory", "r12", "r13", "r14", "r15");
                break;
            case 12:
                v19 = v20 | v1 | v2;
                f3 = f4 * f5 * f6;
                d3 = d4 * d5 * d6;
                break;
        }
        
        /* Force variables live across function call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1);
            use_vars(&v10, &f2, &d2);
        }
        
        /* More computations to extend live ranges */
        v2 = v3 + v4;
        v5 = v6 - v7;
        v8 = v9 * v10;
        v11 = v12 ^ v13;
        v14 = v15 | v16;
        v17 = v18 & v19;
        v20 = v1 << (i % 4);
        
        f3 = f4 + f5;
        f6 = f7 - f8;
        f1 = f2 * f3;
        
        d4 = d5 + d6;
        d7 = d8 - d1;
        d2 = d3 * d4;
        
        /* Mix pointer operations */
        *p1 = (char)(v1 & 0xFF);
        *p2 = (char)(v2 & 0xFF);
        *p3 = (char)(v3 & 0xFF);
        *p4 = (char)(v4 & 0xFF);
        
        /* Accumulate result */
        result += v1 + v2 + v3 + v4 + v5 + (int)f1 + (int)d1;
    }
    
    /* Final mixing */
    result ^= v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result += (int)(f2 + f3 + f4 + f5 + f6 + f7 + f8);
    result += (int)(d2 + d3 + d4 + d5 + d6 + d7 + d8);
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    
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
    
    printf("Result: %d\n", result);
    
    /* Use result to prevent optimization */
    if (result == 0) {
        printf("Zero result (unlikely)\n");
    }
    
    return 0;
}
