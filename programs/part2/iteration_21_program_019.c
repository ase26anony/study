/* test_mcf_coverage.c - Trigger MCF special node printing in GCC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    /* Prevent optimization */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
}

/* Complex function with high register pressure and control flow */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed - 5;
    int v4 = seed ^ 0x1234;
    int v5 = seed << 3;
    int v6 = seed >> 2;
    int v7 = seed | 0xFF;
    int v8 = seed & 0x7F;
    int v9 = seed + 100;
    int v10 = seed - 50;
    
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
    
    char* s1 = (char*)malloc(32);
    char* s2 = (char*)malloc(32);
    char* s3 = (char*)malloc(32);
    
    if (s1) sprintf(s1, "str%d", seed);
    if (s2) sprintf(s2, "val%d", seed * 2);
    if (s3) sprintf(s3, "ptr%d", seed * 3);
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers registers */
        asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
        
        int mod = (i + seed) % 12;
        
        /* Large switch statement creating complex CFG */
        switch (mod) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 1:
                v2 = v3 * v4;
                f2 = f3 * f4;
                d2 = d3 * d4;
                break;
            case 2:
                v3 = v4 ^ v5;
                f3 = f4 - f5;
                d3 = d4 - d5;
                break;
            case 3:
                v4 = v5 | v6;
                f4 = f5 / 2.0f;
                d4 = d5 / 2.0;
                break;
            case 4:
                v5 = v6 & v7;
                f5 = f1 * 3.0f;
                d5 = d1 * 3.0;
                break;
            case 6:
                v7 = v8 << 2;
                f2 = f3 + f4;
                d2 = d3 + d4;
                break;
            case 7:
                v8 = v9 >> 1;
                f3 = f4 - f1;
                d3 = d4 - d1;
                break;
            case 8:
                v9 = v10 * 3;
                f4 = f5 * 0.5f;
                d4 = d5 * 0.5;
                break;
            case 9:
                v10 = v1 ^ v2;
                f5 = f1 + f2;
                d5 = d1 + d2;
                break;
            case 10:
                v1 = v3 + v4;
                f1 = f3 + f4;
                d1 = d3 + d4;
                break;
            case 11:
                v2 = v5 ^ v6;
                f2 = f4 * f5;
                d2 = d4 * d5;
                break;
            default:
                v6 = v7 + v8;
                f3 = f1 - f2;
                d3 = d1 - d2;
                break;
        }
        
        /* Force variables live across function call */
        if (i % 4 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        } else if (i % 4 == 1) {
            use_vars(&v2, &f2, &d2, &s2);
        } else if (i % 4 == 2) {
            use_vars(&v3, &f3, &d3, &s3);
        } else {
            use_vars(&v4, &f4, &d4, &s1);
        }
        
        /* More register clobbering */
        asm volatile("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)(f1 + f2 + f3 + f4 + f5);
    checksum += (int)(d1 + d2 + d3 + d4 + d5);
    
    if (s1) checksum += strlen(s1);
    if (s2) checksum += strlen(s2);
    if (s3) checksum += strlen(s3);
    
    /* Cleanup */
    free(s1);
    free(s2);
    free(s3);
    
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
