/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables are live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    volatile int sink = *a + (int)*b + (int)*c + (int)(long)*d;
    (void)sink;
}

/* Complex test function with high register pressure */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed + 2;
    int v3 = seed + 3;
    int v4 = seed + 4;
    int v5 = seed + 5;
    int v6 = seed + 6;
    int v7 = seed + 7;
    int v8 = seed + 8;
    int v9 = seed + 9;
    int v10 = seed + 10;
    int v11 = seed + 11;
    int v12 = seed + 12;
    int v13 = seed + 13;
    int v14 = seed + 14;
    int v15 = seed + 15;
    int v16 = seed + 16;
    int v17 = seed + 17;
    int v18 = seed + 18;
    int v19 = seed + 19;
    int v20 = seed + 20;
    
    float f1 = seed * 1.1f;
    float f2 = seed * 1.2f;
    float f3 = seed * 1.3f;
    float f4 = seed * 1.4f;
    float f5 = seed * 1.5f;
    float f6 = seed * 1.6f;
    float f7 = seed * 1.7f;
    float f8 = seed * 1.8f;
    float f9 = seed * 1.9f;
    float f10 = seed * 2.0f;
    
    double d1 = seed * 0.1;
    double d2 = seed * 0.2;
    double d3 = seed * 0.3;
    double d4 = seed * 0.4;
    double d5 = seed * 0.5;
    double d6 = seed * 0.6;
    double d7 = seed * 0.7;
    double d8 = seed * 0.8;
    double d9 = seed * 0.9;
    double d10 = seed * 1.0;
    
    char* s1 = (char*)(long)(seed + 100);
    char* s2 = (char*)(long)(seed + 200);
    char* s3 = (char*)(long)(seed + 300);
    char* s4 = (char*)(long)(seed + 400);
    char* s5 = (char*)(long)(seed + 500);
    
    int result = 0;
    
    /* Complex loop with nested control flow */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile ("" : : : "memory");
        
        /* Large switch statement creating complex CFG */
        switch (i % 13) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 1:
                v4 = v5 * v6;
                f4 = f5 * f6;
                d4 = d5 * d6;
                break;
            case 2:
                v7 = v8 - v9;
                f7 = f8 - f9;
                d7 = d8 - d9;
                break;
            case 3:
                v10 = v11 ^ v12;
                s1 = (char*)(long)(v10 + (int)f10);
                break;
            case 4:
                v13 = v14 | v15;
                f2 = f3 * f4;
                d2 = d3 / d4;
                break;
            case 5:
                v16 = v17 & v18;
                f5 = f6 - f7;
                d5 = d6 + d7;
                break;
            case 6:
                v19 = v20 << 2;
                f8 = f9 * 2.0f;
                d8 = d9 / 2.0;
                break;
            case 7:
                v2 = v3 >> 1;
                f3 = f4 + 1.0f;
                d3 = d4 - 1.0;
                break;
            case 8:
                v5 = v6 + i;
                f6 = f7 * i;
                d6 = d7 + i;
                break;
            case 9:
                v8 = v9 * i;
                f9 = f10 / (i + 1);
                d9 = d10 * i;
                break;
            case 10:
                v11 = v12 - i;
                f10 = f1 + i;
                d10 = d1 - i;
                break;
            case 11:
                v14 = v15 ^ i;
                s2 = (char*)(long)(v14 + i);
                break;
            case 12:
                v17 = v18 | i;
                f1 = f2 * i;
                d1 = d2 / (i + 1);
                break;
        }
        
        /* Call external function making variables live across call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        }
        
        /* More inline assembly with register clobbering */
        #ifdef __i386__
        asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
        #elif __x86_64__
        asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        #endif
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            if (j == 0) {
                v1 += v2;
                f1 += f2;
                d1 += d2;
            } else if (j == 1) {
                v3 += v4;
                f3 += f4;
                d3 += d4;
            } else {
                v5 += v6;
                f5 += f6;
                d5 += d6;
            }
        }
        
        /* Compute checksum */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                  (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10 +
                  (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
                  (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10 +
                  (int)(long)s1 + (int)(long)s2 + (int)(long)s3 +
                  (int)(long)s4 + (int)(long)s5;
    }
    
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
