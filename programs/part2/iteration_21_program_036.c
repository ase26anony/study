/* test-mcf-coverage.c - Program to trigger MCF solver special node printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c) {
    volatile int sink = *a + (int)*b + (int)*c;
    (void)sink;
}

/* Complex test function with high register pressure */
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
    float f9 = seed * 1.9f, f10 = seed * 2.0f;
    
    double d1 = seed * 0.1, d2 = seed * 0.2, d3 = seed * 0.3, d4 = seed * 0.4;
    double d5 = seed * 0.5, d6 = seed * 0.6, d7 = seed * 0.7, d8 = seed * 0.8;
    
    char* p1 = (char*)&v1, *p2 = (char*)&v2, *p3 = (char*)&v3, *p4 = (char*)&v4;
    
    int result = 0;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 20;
        
        /* Large switch statement creating complex CFG */
        switch (mod) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                /* Inline asm clobbering registers */
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
                f10 = f1 + f2;
                d2 = d3 * d4;
                asm volatile ("" : : : "memory", "esi", "edi");
                break;
            case 4:
                v13 = v14 | v15;
                f3 = f4 / 2.0f;
                d5 = d6 / 2.0;
                break;
            case 5:
                v16 = v17 & v18;
                f5 = f6 * 3.0f;
                d7 = d8 + 1.0;
                break;
            case 6:
                v19 = v20 << 2;
                f7 = f8 - 1.0f;
                d1 = d2 * 0.5;
                break;
            case 7:
                v1 = v2 >> 1;
                f9 = f10 / 3.0f;
                d3 = d4 + d5;
                asm volatile ("" : : : "memory");
                break;
            case 8:
                v3 = v4 + i;
                f1 = f2 * i;
                d6 = d7 - i;
                break;
            case 9:
                v5 = v6 - i;
                f3 = f4 + i;
                d8 = d1 * i;
                break;
            case 10:
                v7 = v8 ^ i;
                f5 = f6 / i;
                d2 = d3 + i;
                break;
            case 11:
                v9 = v10 | i;
                f7 = f8 * i;
                d4 = d5 - i;
                asm volatile ("" : : : "memory", "eax", "ebx");
                break;
            case 12:
                v11 = v12 & i;
                f9 = f10 + i;
                d6 = d7 * i;
                break;
            case 13:
                v13 = v14 << i;
                f1 = f2 - i;
                d8 = d1 / (i + 1);
                break;
            case 14:
                v15 = v16 >> (i % 4);
                f3 = f4 * (i % 5);
                d2 = d3 + (i % 6);
                break;
            case 15:
                v17 = v18 + v19;
                f5 = f6 + f7;
                d4 = d5 + d6;
                break;
            case 16:
                v20 = v1 * v2;
                f8 = f9 * f10;
                d7 = d8 * d1;
                asm volatile ("" : : : "memory", "ecx", "edx");
                break;
            case 17:
                v2 = v3 - v4;
                f2 = f3 - f4;
                d3 = d4 - d5;
                break;
            case 18:
                v5 = v6 ^ v7;
                f6 = f7 + f8;
                d6 = d7 * d8;
                break;
            case 19:
                v8 = v9 | v10;
                f10 = f1 / f2;
                d1 = d2 + d3;
                break;
        }
        
        /* Force variables live across function call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1);
            asm volatile ("" : : : "memory");
        }
        
        /* Mix results */
        result += v1 + v2 + v3 + v4 + v5 + (int)f1 + (int)d1;
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v1 += j;
            v2 -= j;
            f1 += j * 0.5f;
            if (j % 2 == 0) {
                asm volatile ("" : : : "memory");
                v3 *= (j + 1);
            }
        }
    }
    
    /* Final computation using all variables */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += (int)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10);
    result += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
    result += (int)((long)p1 + (long)p2 + (long)p3 + (long)p4);
    
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
