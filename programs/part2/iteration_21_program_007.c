/* test-mcf-nodes.c - Trigger MCF special node printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variable liveness across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    volatile int sink = *a + (int)*b + (int)*c + (int)(long)*d;
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
    
    char* s1 = (char*)(long)(seed + 100);
    char* s2 = (char*)(long)(seed + 200);
    char* s3 = (char*)(long)(seed + 300);
    char* s4 = (char*)(long)(seed + 400);
    
    int result = 0;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 15;
        
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
                s1 = s2;
                break;
            case 2:
                v7 = v8 - v9;
                f7 = f8 - f9;
                d7 = d8 - d9;
                asm volatile ("" : : : "memory", "esi", "edi");
                break;
            case 3:
                v10 = v11 ^ v12;
                v13 = v14 | v15;
                break;
            case 4:
                f1 = f4 + f7;
                f2 = f5 + f8;
                f3 = f6 + f9;
                break;
            case 5:
                d1 = d4 * d7;
                d2 = d5 * d8;
                d3 = d6 * d9;
                asm volatile ("" : : : "memory", "r8", "r9", "r10", "r11");
                break;
            case 6:
                v16 = v17 & v18;
                v19 = v20 | v1;
                s2 = s3;
                break;
            case 7:
                f4 = f7 * f1;
                f5 = f8 * f2;
                f6 = f9 * f3;
                break;
            case 8:
                d4 = d7 / 2.0;
                d5 = d8 / 3.0;
                d6 = d9 / 4.0;
                break;
            case 9:
                v2 = v3 << 2;
                v5 = v6 >> 1;
                asm volatile ("" : : : "memory", "xmm0", "xmm1", "xmm2");
                break;
            case 10:
                f2 = f3 * 1.5f;
                f5 = f6 * 2.5f;
                f8 = f9 * 3.5f;
                break;
            case 11:
                d3 = d1 + d2;
                d6 = d4 + d5;
                d9 = d7 + d8;
                s3 = s4;
                break;
            case 12:
                v8 = v9 * v10;
                v11 = v12 + v13;
                asm volatile ("" : : : "memory", "xmm3", "xmm4", "xmm5");
                break;
            case 13:
                f7 = f1 - f2;
                f8 = f3 - f4;
                f9 = f5 - f6;
                break;
            case 14:
                d7 = d1 * d2 * d3;
                d8 = d4 * d5 * d6;
                v14 = v15 * v16 * v17;
                break;
        }
        
        /* Force variables to be live across this call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
            use_vars(&v10, &f5, &d6, &s3);
        }
        
        /* Mix results */
        result += v1 + v5 + v10 + v15 + (int)f1 + (int)f5 + (int)d1 + (int)d5;
    }
    
    /* Final mixing to prevent elimination */
    result += v2 + v6 + v11 + v16 + v20;
    result += (int)(f2 + f6 + f8);
    result += (int)(d2 + d6 + d8);
    result += (int)(long)(s1 + s2 + s3 + s4);
    
    return result;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    int result = test_function(iterations, seed);
    
    printf("Result: %d\n", result);
    
    /* Use result to affect return code (prevents optimization) */
    return result % 256;
}
