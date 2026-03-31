/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf
 * Or for RISC-V: riscv32-unknown-linux-gnu-gcc -O3 -march=rv32i -fno-tree-vectorize -fno-unroll-loops -fno-web -fno-gcse test_mcf_coverage.c -o test_mcf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* ptr, float* fptr, double* dptr) {
    asm volatile ("" : : "r"(ptr), "r"(fptr), "r"(dptr) : "memory");
}

/* Main test function with high register pressure and complex CFG */
__attribute__((noinline, optimize("O2")))
unsigned long long test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed ^ 0x1234;
    int v4 = seed - 100;
    int v5 = seed + 200;
    int v6 = seed * 3;
    int v7 = seed / 2;
    int v8 = seed % 17;
    int v9 = seed | 0xFF00;
    int v10 = seed & 0x0F0F;
    
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
    
    int v11 = v1 + v2;
    int v12 = v3 - v4;
    int v13 = v5 * v6;
    int v14 = v7 ^ v8;
    int v15 = v9 & v10;
    
    float f6 = f1 + f2;
    float f7 = f3 - f4;
    float f8 = f5 * f1;
    float f9 = f2 / f3;
    float f10 = f4 + f5;
    
    double d6 = d1 + d2;
    double d7 = d3 - d4;
    double d8 = d5 * d1;
    double d9 = d2 / d3;
    double d10 = d4 + d5;
    
    int v16 = v11 + v12;
    int v17 = v13 - v14;
    int v18 = v15 * v11;
    int v19 = v12 ^ v13;
    int v20 = v14 & v15;
    
    char* s1 = (char*)malloc(32);
    char* s2 = (char*)malloc(32);
    if (s1 && s2) {
        sprintf(s1, "seed=%d", seed);
        sprintf(s2, "iter=%d", iterations);
    }
    
    unsigned long long checksum = 0;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers - forces spills */
        asm volatile ("# Force register clobbering" 
                     : : : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                           "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
        
        /* Large switch statement to create complex CFG */
        switch ((i + seed) % 13) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * f3;
                d1 = d2 - d3;
                break;
            case 1:
                v2 = v3 ^ v4;
                f2 = f3 / f4;
                d2 = d3 * d4;
                break;
            case 2:
                v3 = v4 & v5;
                f3 = f4 + f5;
                d3 = d4 - d5;
                break;
            case 3:
                v4 = v5 | v6;
                f4 = f5 * f6;
                d4 = d5 / d6;
                break;
            case 4:
                v5 = v6 + v7;
                f5 = f6 - f7;
                d5 = d6 + d7;
                break;
            case 5:
                v6 = v7 ^ v8;
                f6 = f7 / f8;
                d6 = d7 * d8;
                break;
            case 6:
                v7 = v8 & v9;
                f7 = f8 + f9;
                d7 = d8 - d9;
                break;
            case 7:
                v8 = v9 | v10;
                f8 = f9 * f10;
                d8 = d9 / d10;
                break;
            case 8:
                v9 = v10 + v11;
                f9 = f10 - f1;
                d9 = d10 + d1;
                break;
            case 9:
                v10 = v11 ^ v12;
                f10 = f1 / f2;
                d10 = d1 * d2;
                break;
            case 10:
                v11 = v12 & v13;
                f1 = f2 + f3;
                d1 = d2 - d3;
                break;
            case 11:
                v12 = v13 | v14;
                f2 = f3 * f4;
                d2 = d3 / d4;
                break;
            case 12:
                v13 = v14 + v15;
                f3 = f4 - f5;
                d3 = d4 + d5;
                break;
        }
        
        /* Force variables live across function call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1);
            use_vars(&v2, &f2, &d2);
            use_vars(&v3, &f3, &d3);
        }
        
        /* More inline assembly */
        asm volatile ("# More clobbering" 
                     : : : "eax", "ebx", "ecx", "edx", "memory");
        
        /* Update checksum with all variables */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
        checksum += (long long)d1 + (long long)d2 + (long long)d3;
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v16 += v17 * j;
            v17 -= v18 ^ j;
            v18 |= v19 & j;
            v19 ^= v20 | j;
            
            asm volatile ("# Inner loop clobber" : : : "esi", "edi");
        }
    }
    
    /* Final mixing */
    checksum ^= v16 ^ v17 ^ v18 ^ v19 ^ v20;
    checksum += (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
    checksum += (long long)d6 + (long long)d7 + (long long)d8 + (long long)d9 + (long long)d10;
    
    if (s1) free(s1);
    if (s2) free(s2);
    
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
    
    unsigned long long result = test_function(iterations, seed);
    printf("Result: %llu\n", result);
    
    return 0;
}
