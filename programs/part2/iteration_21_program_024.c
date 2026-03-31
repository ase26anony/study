/* test-mcf-coverage.c
 * Designed to trigger MCF solver during register allocation
 * and invoke node printing with special indices (ENTRY/EXIT blocks)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    volatile int sink = *a;
    sink += (int)*b;
    sink += (int)*c;
    sink += (int)(long)*d;
    (void)sink;
}

/* Another external function to prevent optimization */
void __attribute__((noinline)) clobber_all(void) {
    /* Inline asm that clobbers many registers */
    #ifdef __x86_64__
    asm volatile ("" : : : 
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15", "memory");
    #elif defined(__i386__)
    asm volatile ("" : : : 
        "eax", "ebx", "ecx", "edx", "esi", "edi",
        "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
        "memory");
    #else
    /* Generic clobber for other architectures */
    asm volatile ("" : : : "memory");
    #endif
}

/* Main test function with high register pressure */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed - 4;
    int v5 = seed ^ 0x55;
    int v6 = seed | 0xAA;
    int v7 = seed & 0xF0;
    int v8 = seed << 2;
    int v9 = seed >> 1;
    int v10 = ~seed;
    
    float f1 = (float)seed * 1.1f;
    float f2 = (float)seed * 2.2f;
    float f3 = (float)seed * 3.3f;
    float f4 = (float)seed * 4.4f;
    float f5 = (float)seed * 5.5f;
    
    double d1 = (double)seed * 1.11;
    double d2 = (double)seed * 2.22;
    double d3 = (double)seed * 3.33;
    double d4 = (double)seed * 4.44;
    double d5 = (double)seed * 5.55;
    
    char* s1 = (char*)(long)(seed + 100);
    char* s2 = (char*)(long)(seed + 200);
    char* s3 = (char*)(long)(seed + 300);
    char* s4 = (char*)(long)(seed + 400);
    char* s5 = (char*)(long)(seed + 500);
    
    /* Additional variables for more pressure */
    int v11 = v1 + v2;
    int v12 = v3 + v4;
    int v13 = v5 + v6;
    int v14 = v7 + v8;
    int v15 = v9 + v10;
    int v16 = v1 * v3;
    int v17 = v2 * v4;
    int v18 = v5 * v7;
    int v19 = v6 * v8;
    int v20 = v9 * v10;
    
    float f6 = f1 + f2;
    float f7 = f3 + f4;
    float f8 = f5 * 2.0f;
    float f9 = f1 * f3;
    float f10 = f2 * f4;
    
    double d6 = d1 + d2;
    double d7 = d3 + d4;
    double d8 = d5 * 2.0;
    double d9 = d1 * d3;
    double d10 = d2 * d4;
    
    int result = 0;
    
    /* Complex loop with switch to create control flow */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 20;
        
        /* Large switch statement for complex CFG */
        switch (mod) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * 1.5f;
                d1 = d2 / 2.0;
                break;
            case 1:
                v2 = v3 - v4;
                f2 = f3 + 2.0f;
                d2 = d3 * 1.1;
                break;
            case 2:
                v3 = v4 * v5;
                f3 = f4 - 1.0f;
                d3 = d4 + 0.5;
                break;
            case 3:
                v4 = v5 / (v6 ? v6 : 1);
                f4 = f5 * 3.0f;
                d4 = d5 - 0.2;
                break;
            case 4:
                v5 = v6 ^ v7;
                f5 = f1 / 2.0f;
                d5 = d1 * 1.5;
                break;
            case 5:
                v6 = v7 | v8;
                f6 = f2 + f3;
                d6 = d2 - d3;
                break;
            case 6:
                v7 = v8 & v9;
                f7 = f3 * f4;
                d7 = d3 / d4;
                break;
            case 7:
                v8 = v9 << 1;
                f8 = f4 - f5;
                d8 = d4 + 1.0;
                break;
            case 8:
                v9 = v10 >> 2;
                f9 = f5 * 0.5f;
                d9 = d5 * 0.8;
                break;
            case 9:
                v10 = ~v1;
                f10 = f1 + 10.0f;
                d10 = d1 - 5.0;
                break;
            case 10:
                v11 = v12 + i;
                f6 = f7 * 1.2f;
                d6 = d7 / 1.3;
                break;
            case 11:
                v12 = v13 - i;
                f7 = f8 + 2.2f;
                d7 = d8 * 1.4;
                break;
            case 12:
                v13 = v14 * i;
                f8 = f9 - 0.8f;
                d8 = d9 + 0.6;
                break;
            case 13:
                v14 = v15 / (i ? i : 1);
                f9 = f10 * 1.7f;
                d9 = d10 - 0.3;
                break;
            case 14:
                v15 = v16 ^ i;
                f10 = f6 / 1.1f;
                d10 = d6 * 1.2;
                break;
            case 15:
                v16 = v17 | i;
                f1 = f6 + f7;
                d1 = d6 - d7;
                break;
            case 16:
                v17 = v18 & i;
                f2 = f7 * f8;
                d2 = d7 / d8;
                break;
            case 17:
                v18 = v19 << (i & 3);
                f3 = f8 - f9;
                d3 = d8 + 2.0;
                break;
            case 18:
                v19 = v20 >> (i & 2);
                f4 = f9 * 0.7f;
                d4 = d9 * 0.9;
                break;
            case 19:
                v20 = ~v11;
                f5 = f10 + 20.0f;
                d5 = d10 - 10.0;
                break;
        }
        
        /* Force variables live across call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
            clobber_all();
        }
        
        if (i % 5 == 0) {
            use_vars(&v2, &f2, &d2, &s2);
            clobber_all();
        }
        
        if (i % 3 == 0) {
            use_vars(&v3, &f3, &d3, &s3);
            clobber_all();
        }
        
        /* More register pressure with computations */
        v1 = (v1 * 1103515245 + 12345) & 0x7fffffff;
        v2 = (v2 * 1103515245 + 12345) & 0x7fffffff;
        f1 = f1 * 1.01f + 0.5f;
        f2 = f2 * 0.99f - 0.5f;
        d1 = d1 * 1.001 + 0.1;
        d2 = d2 * 0.999 - 0.1;
        
        /* Accumulate result to prevent elimination */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
        result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
        result += (int)(long)s1 + (int)(long)s2 + (int)(long)s3;
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
    
    printf("Result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0x12345678) {
        printf("Impossible!\n");
    }
    
    return 0;
}
