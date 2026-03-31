/* test-mcf-coverage.c */
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
    
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3, d4 = seed * 2.4;
    double d5 = seed * 2.5, d6 = seed * 2.6;
    
    char* s1 = (char*)(long)(seed + 100);
    char* s2 = (char*)(long)(seed + 200);
    char* s3 = (char*)(long)(seed + 300);
    char* s4 = (char*)(long)(seed + 400);
    
    /* Additional variables for more pressure */
    int v21 = 0, v22 = 0, v23 = 0, v24 = 0, v25 = 0;
    float f11 = 0.0f, f12 = 0.0f;
    double d7 = 0.0, d8 = 0.0;
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        __asm__ volatile (
            "# Force register clobbering\n"
            : : : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi"
        );
        
        /* Large switch statement creating complex CFG */
        switch (i % 15) {
            case 0:
                v1 = v2 + v3; f1 = f2 + f3; d1 = d2 + d3;
                break;
            case 1:
                v4 = v5 * v6; f4 = f5 * f6; d4 = d5 * d6;
                break;
            case 2:
                v7 = v8 - v9; f7 = f8 - f9; d1 = d2 - d3;
                break;
            case 3:
                v10 = v11 / (v12 ? v12 : 1); 
                f10 = f1 / (f2 ? f2 : 1.0f);
                break;
            case 4:
                v13 = v14 | v15; v16 = v17 & v18;
                break;
            case 5:
                v19 = v20 ^ v1; v2 = v3 << 2; v4 = v5 >> 1;
                break;
            case 6:
                f2 = f3 * 2.0f; f4 = f5 / 2.0f; f6 = f7 + f8;
                break;
            case 7:
                d2 = d3 * 3.0; d4 = d5 / 3.0; d6 = d7 + d8;
                break;
            case 8:
                v21 = v22 + v23; f11 = f12 * 1.5f; d7 = d8 * 2.5;
                break;
            case 9:
                v24 = v25 * v1; v2 = v3 + v4; v5 = v6 - v7;
                break;
            case 10:
                /* Force spill by using all variables */
                v1 += v2 + v3 + v4 + v5;
                f1 += f2 + f3 + f4 + f5;
                d1 += d2 + d3 + d4 + d5;
                break;
            case 11:
                v8 = v9 * v10; v11 = v12 + v13; v14 = v15 - v16;
                break;
            case 12:
                f6 = f7 * f8; f9 = f10 + f1; f2 = f3 - f4;
                break;
            case 13:
                d3 = d4 * d5; d6 = d7 + d8; d1 = d2 - d3;
                break;
            case 14:
                /* Mix pointer and integer operations */
                v17 = (int)(long)s1 + (int)(long)s2;
                v18 = (int)(long)s3 - (int)(long)s4;
                break;
        }
        
        /* Call external function to keep variables live across calls */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        }
        
        /* Another inline assembly with different clobbers */
        __asm__ volatile (
            "# More register pressure\n"
            : : : "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v1 += j; v2 -= j; v3 *= (j + 1);
            f1 += (float)j; f2 -= (float)j;
            d1 += (double)j; d2 -= (double)j;
        }
    }
    
    /* Compute checksum from all variables */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                   (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10 +
                   (int)f11 + (int)f12 +
                   (int)d1 + (int)d2 + (int)d3 + (int)d4 +
                   (int)d5 + (int)d6 + (int)d7 + (int)d8 +
                   (int)(long)s1 + (int)(long)s2 + (int)(long)s3 + (int)(long)s4;
    
    return checksum;
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
