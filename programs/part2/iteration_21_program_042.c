/* test-mcf-coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer -o test-mcf test-mcf-coverage.c
 * Run with: ./test-mcf 100
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
__attribute__((noinline)) void use_vars(int* a, float* b, double* c, char** d) {
    /* Prevent optimization */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
}

/* Main test function with high register pressure and complex CFG */
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
    int v8 = seed & 0xAA;
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
    
    char* s1 = (char*)malloc(16);
    char* s2 = (char*)malloc(16);
    char* s3 = (char*)malloc(16);
    
    if (s1 && s2 && s3) {
        sprintf(s1, "str%d", seed);
        sprintf(s2, "val%d", seed * 2);
        sprintf(s3, "ptr%d", seed * 3);
    }
    
    /* More variables */
    int v11 = v1 + v2;
    int v12 = v3 * v4;
    int v13 = v5 ^ v6;
    int v14 = v7 & v8;
    int v15 = v9 | v10;
    int v16 = v1 - v10;
    int v17 = v2 * v9;
    int v18 = v3 + v8;
    int v19 = v4 - v7;
    int v20 = v5 ^ v6;
    
    float f6 = f1 + f2;
    float f7 = f3 * f4;
    float f8 = f5 - f1;
    float f9 = f2 * f3;
    float f10 = f4 / 2.0f;
    
    double d5 = d1 + d2;
    double d6 = d3 * d4;
    double d7 = d1 - d4;
    double d8 = d2 / 1.5;
    
    /* Complex loop with switch to create CFG edges */
    int result = 0;
    for (int i = 0; i < iterations; i++) {
        /* Clobber many registers to force spills */
        asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
        
        /* Large switch statement for complex CFG */
        switch (i % 13) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * 3.14f;
                d1 = d2 + 1.0;
                result += v1;
                break;
            case 1:
                v4 = v5 - v6;
                f3 = f4 / 2.0f;
                d3 = d4 * 0.5;
                result += v4;
                break;
            case 2:
                v7 = v8 ^ v9;
                f5 = f1 + f2;
                d5 = d6 - d7;
                result += v7;
                break;
            case 3:
                v10 = v11 * v12;
                f6 = f7 - f8;
                d8 = d1 * d2;
                result += v10;
                break;
            case 4:
                v13 = v14 | v15;
                f9 = f10 * 2.0f;
                result += v13;
                break;
            case 5:
                v16 = v17 & v18;
                f1 = f3 / f4;
                result += v16;
                break;
            case 6:
                v19 = v20 ^ v1;
                f2 = f5 + f6;
                result += v19;
                break;
            case 7:
                v2 = v3 << 2;
                f7 = f8 * f9;
                result += v2;
                break;
            case 8:
                v4 = v5 >> 1;
                f10 = f1 - f2;
                result += v4;
                break;
            case 9:
                v6 = v7 + v8;
                f3 = f4 * 3.0f;
                result += v6;
                break;
            case 10:
                v9 = v10 - v11;
                f5 = f6 / 4.0f;
                result += v9;
                break;
            case 11:
                v12 = v13 * v14;
                f7 = f8 + f9;
                result += v12;
                break;
            case 12:
                v15 = v16 ^ v17;
                f10 = f1 * f2;
                result += v15;
                break;
        }
        
        /* Force variables live across call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        }
        
        /* More register clobbering */
        asm volatile("" : : : "eax", "ebx", "ecx", "edx", "memory");
        
        /* Nested loop for additional CFG complexity */
        for (int j = 0; j < 3; j++) {
            v1 += j;
            v2 -= j;
            f1 += j * 0.1f;
            d1 += j * 0.01;
        }
    }
    
    /* Compute final checksum from all variables */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                   (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10 +
                   (int)d1 + (int)d2 + (int)d3 + (int)d4 +
                   (int)d5 + (int)d6 + (int)d7 + (int)d8;
    
    if (s1) free(s1);
    if (s2) free(s2);
    if (s3) free(s3);
    
    return result + checksum;
}

int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int seed = 42;
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    /* Call test function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        total += test_function(iterations, seed + i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
