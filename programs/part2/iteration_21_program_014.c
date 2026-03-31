/* test-mcf-coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables stay live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c) {
    volatile int sink = *a;
    sink += (int)*b;
    sink += (int)*c;
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
    
    double d1 = seed * 2.1;
    double d2 = seed * 2.2;
    double d3 = seed * 2.3;
    double d4 = seed * 2.4;
    double d5 = seed * 2.5;
    
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    
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
                break;
            case 1:
                v2 = v3 - v4;
                f2 = f3 - f4;
                d2 = d3 - d4;
                break;
            case 2:
                v3 = v4 * v5;
                f3 = f4 * f5;
                d3 = d4 * d5;
                break;
            case 3:
                v4 = v5 / (v6 ? v6 : 1);
                f4 = f5 / (f6 ? f6 : 1.0f);
                d4 = d5 / (d1 ? d1 : 1.0);
                break;
            case 4:
                v5 = v6 ^ v7;
                f5 = f6 + f7;
                d5 = d1 + d2;
                break;
            case 5:
                v6 = v7 | v8;
                f6 = f7 - f8;
                d1 = d2 - d3;
                break;
            case 6:
                v7 = v8 & v9;
                f7 = f8 * f1;
                d2 = d3 * d4;
                break;
            case 7:
                v8 = v9 << 2;
                f8 = f1 / 2.0f;
                d3 = d4 / 2.0;
                break;
            case 8:
                v9 = v10 >> 1;
                f1 = f2 + f3 + f4;
                d4 = d5 + d1 + d2;
                break;
            case 9:
                v10 = v11 + v12 + v13;
                f2 = f3 * f4 * f5;
                d5 = d1 * d2 * d3;
                break;
            case 10:
                v11 = v12 - v13 - v14;
                f3 = f4 - f5 - f6;
                d1 = d2 - d3 - d4;
                break;
            case 11:
                v12 = v13 * v14 * v15;
                f4 = f5 * f6 * f7;
                d2 = d3 * d4 * d5;
                break;
            case 12:
                v13 = v14 / (v15 ? v15 : 1) / (v16 ? v16 : 1);
                f5 = f6 / (f7 ? f7 : 1.0f) / (f8 ? f8 : 1.0f);
                d3 = d4 / (d5 ? d5 : 1.0) / (d1 ? d1 : 1.0);
                break;
            case 13:
                v14 = v15 ^ v16 ^ v17;
                f6 = f7 + f8 + f1;
                d4 = d5 + d1 + d2;
                break;
            case 14:
                v15 = v16 | v17 | v18;
                f7 = f8 - f1 - f2;
                d5 = d1 - d2 - d3;
                break;
            case 15:
                v16 = v17 & v18 & v19;
                f8 = f1 * f2 * f3;
                d1 = d2 * d3 * d4;
                break;
            case 16:
                v17 = v18 << 3;
                f1 = f2 / 3.0f;
                d2 = d3 / 3.0;
                break;
            case 17:
                v18 = v19 >> 2;
                f2 = f3 + f4 + f5 + f6;
                d3 = d4 + d5 + d1 + d2;
                break;
            case 18:
                v19 = v20 + v1 + v2 + v3;
                f3 = f4 * f5 * f6 * f7;
                d4 = d5 * d1 * d2 * d3;
                break;
            case 19:
                v20 = v1 - v2 - v3 - v4;
                f4 = f5 - f6 - f7 - f8;
                d5 = d1 - d2 - d3 - d4;
                break;
        }
        
        /* Force variables to be live across this call */
        use_vars(&v1, &f1, &d1);
        use_vars(&v2, &f2, &d2);
        use_vars(&v3, &f3, &d3);
        
        /* Inline assembly to clobber registers */
        #ifdef __x86_64__
        asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        #elif __i386__
        asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
        #elif __riscv
        asm volatile ("" : : : "t0", "t1", "t2", "t3", "t4", "t5", "t6",
                       "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7");
        #endif
        
        /* More computations to keep variables alive */
        if (i % 3 == 0) {
            v1 = v2 + v3;
            f1 = f2 + f3;
            d1 = d2 + d3;
        } else if (i % 3 == 1) {
            v4 = v5 + v6;
            f4 = f5 + f6;
            d4 = d5 + d1;
        } else {
            v7 = v8 + v9;
            f7 = f8 + f1;
            d2 = d3 + d4;
        }
    }
    
    /* Compute checksum from all variables */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
             v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
             (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8 +
             (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
             (int)(long)p1 + (int)(long)p2 + (int)(long)p3;
    
    return result;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 1000) iterations = 1000;
    }
    
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    int result = test_function(iterations, seed);
    printf("Result: %d\n", result);
    
    return 0;
}
