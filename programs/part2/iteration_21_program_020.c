/* test_mcf_coverage.c - Trigger MCF solver special node printing */

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
int test_mcf_function(int iterations, int seed) {
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
        int mod = (i + seed) % 12;
        
        /* Large switch statement creates complex CFG */
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
                asm volatile ("" : : : "memory", "esi", "edi", "ebp");
                break;
            case 2:
                v7 = v8 ^ v9;
                f7 = f8 - f9;
                d7 = d8 - d9;
                break;
            case 3:
                v10 = v11 | v12;
                f1 = f4 + f7;
                d1 = d4 + d7;
                asm volatile ("" : : : "memory", "r8", "r9", "r10", "r11");
                break;
            case 4:
                v13 = v14 & v15;
                f2 = f5 * f8;
                d2 = d5 * d8;
                break;
            case 5:
                v16 = v17 << 2;
                f3 = f6 / 2.0f;
                d3 = d6 / 2.0;
                asm volatile ("" : : : "memory", "xmm0", "xmm1", "xmm2");
                break;
            case 6:
                v18 = v19 >> 1;
                f4 = f7 + 1.0f;
                d4 = d7 + 1.0;
                break;
            case 7:
                v20 = v1 + v4;
                f5 = f2 - f3;
                d5 = d2 - d3;
                asm volatile ("" : : : "memory", "xmm3", "xmm4", "xmm5");
                break;
            case 8:
                v2 = v3 * v5;
                f6 = f1 * f4;
                d6 = d1 * d4;
                break;
            case 9:
                v6 = v7 ^ v8;
                f7 = f5 + f6;
                d7 = d5 + d6;
                asm volatile ("" : : : "memory", "xmm6", "xmm7", "xmm8");
                break;
            case 10:
                v9 = v10 | v11;
                f8 = f3 * f7;
                d8 = d3 * d7;
                break;
            case 11:
                v12 = v13 & v14;
                f1 = f8 - f2;
                d1 = d8 - d2;
                asm volatile ("" : : : "memory", "xmm9", "xmm10", "xmm11");
                break;
        }
        
        /* Force variables to be live across function call */
        if (i % 4 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        } else if (i % 4 == 1) {
            use_vars(&v5, &f5, &d5, &s2);
        } else if (i % 4 == 2) {
            use_vars(&v10, &f3, &d7, &s3);
        } else {
            use_vars(&v15, &f7, &d4, &s4);
        }
        
        /* Mix all variables to prevent dead code elimination */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8 +
                  (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6 + (int)d7 + (int)d8 +
                  (int)(long)s1 + (int)(long)s2 + (int)(long)s3 + (int)(long)s4;
    }
    
    return result;
}

/* Another complex function to create interprocedural pressure */
__attribute__((noinline, optimize("O3")))
int secondary_function(int x, int y) {
    int a = x, b = y, c = x * y, d = x + y;
    float fa = x * 0.5f, fb = y * 0.25f;
    double da = x * 0.125, db = y * 0.0625;
    
    for (int i = 0; i < 100; i++) {
        a = (a * b + c) % 1000;
        b = (b * c + d) % 1000;
        c = (c * d + a) % 1000;
        d = (d * a + b) % 1000;
        
        fa = fa * 1.1f + fb;
        fb = fb * 0.9f - fa;
        da = da * 1.01 + db;
        db = db * 0.99 - da;
        
        /* More inline asm clobbering */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi");
    }
    
    return a + b + c + d + (int)fa + (int)fb + (int)da + (int)db;
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
    
    /* Call complex functions multiple times */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += test_mcf_function(iterations, seed + i);
        sum += secondary_function(seed + i, iterations);
    }
    
    printf("Result checksum: %d\n", sum);
    return 0;
}
