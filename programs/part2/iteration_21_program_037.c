/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables are live across calls */
__attribute__((noinline)) void use_vars(int *ptr, float *fptr, double *dptr) {
    asm volatile ("" : : "r"(ptr), "r"(fptr), "r"(dptr) : "memory");
}

/* Main test function with high register pressure and complex CFG */
__attribute__((noinline, optimize("O2"))) 
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed - 4;
    int v5 = seed + 5;
    int v6 = seed * 6;
    int v7 = seed / 7;
    int v8 = seed - 8;
    int v9 = seed + 9;
    int v10 = seed * 10;
    int v11 = seed / 11;
    int v12 = seed - 12;
    int v13 = seed + 13;
    int v14 = seed * 14;
    int v15 = seed / 15;
    int v16 = seed - 16;
    int v17 = seed + 17;
    int v18 = seed * 18;
    int v19 = seed / 19;
    int v20 = seed - 20;
    
    /* Float variables to use FP registers */
    float f1 = seed * 1.1f;
    float f2 = seed * 2.2f;
    float f3 = seed * 3.3f;
    float f4 = seed * 4.4f;
    float f5 = seed * 5.5f;
    float f6 = seed * 6.6f;
    float f7 = seed * 7.7f;
    float f8 = seed * 8.8f;
    
    /* Double variables */
    double d1 = seed * 1.11;
    double d2 = seed * 2.22;
    double d3 = seed * 3.33;
    double d4 = seed * 4.44;
    double d5 = seed * 5.55;
    
    /* Pointer variables */
    char *p1 = (char*)&v1;
    char *p2 = (char*)&v2;
    char *p3 = (char*)&v3;
    
    int result = 0;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers many registers */
        asm volatile (
            "# Clobber many registers\n"
            : : : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                  "memory", "st", "st(1)", "st(2)", "st(3)", "st(4)"
        );
        
        /* Large switch statement creating complex CFG */
        switch ((i + seed) % 12) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 1:
                v4 = v5 * v6;
                f4 = f5 * f6;
                d4 = d5 * 2.0;
                break;
            case 2:
                v7 = v8 - v9;
                f7 = f8 - f1;
                d2 = d3 - d1;
                break;
            case 3:
                v10 = v11 / (v12 ? v12 : 1);
                f2 = f3 / (f4 ? f4 : 1.0f);
                d5 = d1 / (d2 ? d2 : 1.0);
                break;
            case 4:
                v13 = v14 & v15;
                v16 = v17 | v18;
                break;
            case 5:
                v19 = v20 ^ v1;
                f5 = f6 + f7;
                d3 = d4 * d5;
                break;
            case 6:
                *p1 = (char)v2;
                *p2 = (char)v3;
                *p3 = (char)v4;
                break;
            case 7:
                v5 = v6 << 2;
                v7 = v8 >> 1;
                break;
            case 8:
                f8 = f1 * f2 - f3;
                d1 = d2 + d3 - d4;
                break;
            case 9:
                v9 = v10 + v11 - v12;
                f3 = f4 * f5 / f6;
                break;
            case 10:
                v13 = ~v14;
                v15 = -v16;
                break;
            case 11:
                /* Force variables to be live across function call */
                use_vars(&v17, &f7, &d5);
                break;
        }
        
        /* More computations mixing variables */
        v1 = v1 + i;
        v2 = v2 - i;
        v3 = v3 * (i % 5 + 1);
        f1 = f1 + i * 0.1f;
        f2 = f2 - i * 0.2f;
        d1 = d1 + i * 0.01;
        
        /* Another inline assembly with different clobbers */
        asm volatile (
            "# Clobber more registers\n"
            : : : "eax", "ebx", "ecx", "edx", "esi", "edi",
                  "st(5)", "st(6)", "st(7)", "mm0", "mm1"
        );
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v18 = v19 + v20 + j;
            f4 = f5 * f6 + j;
            d2 = d3 - d4 * j;
        }
    }
    
    /* Combine all variables into a result */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
             v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
             (int)f1 + (int)f2 + (int)f3 + (int)f4 + 
             (int)f5 + (int)f6 + (int)f7 + (int)f8 +
             (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    
    return result;
}

int main(int argc, char *argv[]) {
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
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
