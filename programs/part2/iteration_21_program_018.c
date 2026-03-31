/* test_mcf_coverage.c - Triggers MCF special node printing in GCC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    volatile int sink = *a + (int)*b + (int)*c + (int)(long)*d;
    (void)sink;
}

/* Complex function with high register pressure and control flow */
__attribute__((noinline, optimize("O2")))
unsigned long test_function(int iterations, int seed) {
    /* Declare many variables to create register pressure */
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
    
    unsigned long checksum = 0;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 20;
        
        /* Large switch statement creates complex CFG */
        switch (mod) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                s1 = s2;
                break;
            case 1:
                v4 = v5 * v6;
                f4 = f5 * f6;
                d4 = d5 * d6;
                s2 = s3;
                break;
            case 2:
                v7 = v8 - v9;
                f7 = f8 - f9;
                d7 = d8 - d9;
                s3 = s4;
                break;
            case 3:
                v10 = v11 ^ v12;
                f1 = f4 + f7;
                d1 = d4 + d7;
                s4 = s1;
                break;
            case 4:
                v13 = v14 | v15;
                f2 = f5 * f8;
                d2 = d5 * d8;
                break;
            case 5:
                v16 = v17 & v18;
                f3 = f6 / 2.0f;
                d3 = d6 / 2.0;
                break;
            case 6:
                v19 = v20 << 2;
                f4 = f7 * 3.0f;
                d4 = d7 * 3.0;
                break;
            case 7:
                v1 = v4 >> 1;
                f5 = f1 + f2;
                d5 = d1 + d2;
                break;
            case 8:
                v2 = v5 + v6;
                f6 = f3 - f4;
                d6 = d3 - d4;
                break;
            case 9:
                v3 = v7 * v8;
                f7 = f5 * f6;
                d7 = d5 * d6;
                break;
            case 10:
                v4 = v9 - v10;
                f8 = f7 / f1;
                d8 = d7 / d1;
                break;
            case 11:
                v5 = v11 ^ v12;
                f1 = f8 + f2;
                d1 = d8 + d2;
                break;
            case 12:
                v6 = v13 | v14;
                f2 = f3 * f4;
                d2 = d3 * d4;
                break;
            case 13:
                v7 = v15 & v16;
                f3 = f5 / f6;
                d3 = d5 / d6;
                break;
            case 14:
                v8 = v17 << 1;
                f4 = f7 * f8;
                d4 = d7 * d8;
                break;
            case 15:
                v9 = v18 >> 2;
                f5 = f1 - f2;
                d5 = d1 - d2;
                break;
            case 16:
                v10 = v19 + v20;
                f6 = f3 + f4;
                d6 = d3 + d4;
                break;
            case 17:
                v11 = v1 * v2;
                f7 = f5 * f6;
                d7 = d5 * d6;
                break;
            case 18:
                v12 = v3 - v4;
                f8 = f7 / f8;
                d8 = d7 / d8;
                break;
            case 19:
                v13 = v5 ^ v6;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
        }
        
        /* Force variables live across call with inline assembly */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                      "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* Call external function to force register saves */
        if (i % 5 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        }
        
        /* More inline assembly to clobber registers */
        asm volatile ("" : : : "memory", "r8", "r9", "r10", "r11",
                      "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* Update checksum to prevent elimination */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        checksum += (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3;
        checksum += (unsigned long)d1 + (unsigned long)d2;
        checksum += (unsigned long)(long)s1 + (unsigned long)(long)s2;
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    unsigned long result = test_function(iterations, seed);
    printf("Result: %lu\n", result);
    
    return 0;
}
