/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables to be live across calls */
void __attribute__((noinline)) use_vars(int* ptr, float* fptr, double* dptr) {
    volatile int sink = *ptr;
    sink += (int)(*fptr);
    sink += (int)(*dptr);
    (void)sink;
}

/* Main test function with high register pressure and complex CFG */
void __attribute__((noinline, optimize("O2"))) 
test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4, v5 = seed + 5;
    int v6 = seed + 6, v7 = seed + 7, v8 = seed + 8, v9 = seed + 9, v10 = seed + 10;
    int v11 = seed + 11, v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    
    float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f, f4 = seed * 1.4f;
    float f5 = seed * 1.5f, f6 = seed * 1.6f, f7 = seed * 1.7f, f8 = seed * 1.8f;
    
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3, d4 = seed * 2.4;
    double d5 = seed * 2.5, d6 = seed * 2.6, d7 = seed * 2.7, d8 = seed * 2.8;
    
    char* s1 = "string1";
    char* s2 = "string2";
    char* s3 = "string3";
    char* s4 = "string4";
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 15;
        
        /* Large switch statement creating complex CFG */
        switch (mod) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                /* Clobber registers to force spills */
                asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
                break;
            case 1:
                v4 = v5 * v6;
                f4 = f5 * f6;
                d4 = d5 * d6;
                asm volatile ("" : : : "eax", "ebx", "ecx", "edx");
                break;
            case 2:
                v7 = v8 - v9;
                f7 = f8 - f9;
                d7 = d8 - d9;
                break;
            case 3:
                v10 = v11 ^ v12;
                f1 = f2 - f3;
                d1 = d2 * d3;
                asm volatile ("" : : : "eax", "ebx");
                break;
            case 4:
                v13 = v14 | v15;
                f4 = f5 / 2.0f;
                d4 = d5 / 2.0;
                break;
            case 5:
                v16 = v17 & v18;
                f7 = f8 * 3.0f;
                d7 = d8 * 3.0;
                asm volatile ("" : : : "ecx", "edx");
                break;
            case 6:
                v19 = v20 << 2;
                f1 = f2 + f4;
                d1 = d2 + d4;
                break;
            case 7:
                v2 = v3 >> 1;
                f5 = f6 - f7;
                d5 = d6 - d7;
                asm volatile ("" : : : "esi", "edi");
                break;
            case 8:
                v5 = v6 + v7;
                f8 = f1 * f2;
                d8 = d1 * d2;
                break;
            case 9:
                v8 = v9 - v10;
                f3 = f4 / f5;
                d3 = d4 / d5;
                asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
                break;
            case 10:
                v11 = v12 * v13;
                f6 = f7 + f8;
                d6 = d7 + d8;
                break;
            case 11:
                v14 = v15 ^ v16;
                f2 = f3 * 1.5f;
                d2 = d3 * 1.5;
                break;
            case 12:
                v17 = v18 | v19;
                f5 = f6 / 3.0f;
                d5 = d6 / 3.0;
                asm volatile ("" : : : "eax", "ebx", "ecx");
                break;
            case 13:
                v20 = v1 & v2;
                f8 = f1 - f2;
                d8 = d1 - d2;
                break;
            case 14:
                v3 = v4 << 3;
                f4 = f5 * f6;
                d4 = d5 * d6;
                asm volatile ("" : : : "edx", "esi", "edi");
                break;
        }
        
        /* Force variables to be live across function call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1);
            asm volatile ("" : : : "memory");
        }
        if (i % 5 == 0) {
            use_vars(&v10, &f5, &d5);
        }
        
        /* More computations to keep variables alive */
        v1 = v1 + 1;
        v2 = v2 - 1;
        f1 = f1 * 1.01f;
        f2 = f2 / 1.01f;
        d1 = d1 + 0.1;
        d2 = d2 - 0.1;
        
        /* String operations to involve pointer registers */
        if (s1[0]) s1 = s2;
        if (s3[0]) s3 = s4;
    }
    
    /* Final computation to prevent dead code elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7 + (int)f8 +
                 (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 + (int)d6 + (int)d7 + (int)d8 +
                 (int)(long)s1 + (int)(long)s2 + (int)(long)s3 + (int)(long)s4;
    
    printf("Result checksum: %d\n", result);
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
    
    printf("Running MCF test with iterations=%d, seed=%d\n", iterations, seed);
    
    /* Call the complex function multiple times to ensure MCF runs */
    test_function(iterations, seed);
    test_function(iterations / 2, seed + 1);
    test_function(iterations / 4, seed + 2);
    
    return 0;
}
