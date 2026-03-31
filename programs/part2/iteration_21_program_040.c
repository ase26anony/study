/* test_mcf_coverage.c - Trigger MCF solver special node printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variable liveness across calls */
void __attribute__((noinline)) use_vars(int* ptr, float* fptr, double* dptr) {
    volatile int sink = *ptr;
    *fptr += (float)sink;
    *dptr += (double)sink;
    (void)sink;
}

/* Complex function with high register pressure and control flow */
__attribute__((noinline))
__attribute__((optimize("O2")))
int test_mcf_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed * 1, v2 = seed * 2, v3 = seed * 3, v4 = seed * 4;
    int v5 = seed * 5, v6 = seed * 6, v7 = seed * 7, v8 = seed * 8;
    int v9 = seed * 9, v10 = seed * 10, v11 = seed * 11, v12 = seed * 12;
    int v13 = seed * 13, v14 = seed * 14, v15 = seed * 15, v16 = seed * 16;
    int v17 = seed * 17, v18 = seed * 18, v19 = seed * 19, v20 = seed * 20;
    
    float f1 = v1 * 0.1f, f2 = v2 * 0.2f, f3 = v3 * 0.3f, f4 = v4 * 0.4f;
    float f5 = v5 * 0.5f, f6 = v6 * 0.6f, f7 = v7 * 0.7f, f8 = v8 * 0.8f;
    
    double d1 = v1 * 0.01, d2 = v2 * 0.02, d3 = v3 * 0.03, d4 = v4 * 0.04;
    double d5 = v5 * 0.05, d6 = v6 * 0.06, d7 = v7 * 0.07, d8 = v8 * 0.08;
    
    char* p1 = (char*)&v1, *p2 = (char*)&v2, *p3 = (char*)&v3, *p4 = (char*)&v4;
    
    /* Additional variables for more pressure */
    int v21 = v1 ^ v2, v22 = v3 ^ v4, v23 = v5 ^ v6, v24 = v7 ^ v8;
    int v25 = v9 ^ v10, v26 = v11 ^ v12, v27 = v13 ^ v14, v28 = v15 ^ v16;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int selector = (v1 + i) % 15;
        
        /* Inline assembly to clobber registers - forces spills */
        asm volatile (
            "# Clobber many registers"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* Large switch statement creates complex CFG */
        switch (selector) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 1:
                v2 = v3 + v4;
                f2 = f3 + f4;
                d2 = d3 + d4;
                break;
            case 2:
                v3 = v4 + v5;
                f3 = f4 + f5;
                d3 = d4 + d5;
                break;
            case 3:
                v4 = v5 + v6;
                f4 = f5 + f6;
                d4 = d5 + d6;
                break;
            case 4:
                v5 = v6 + v7;
                f5 = f6 + f7;
                d5 = d6 + d7;
                break;
            case 5:
                v6 = v7 + v8;
                f6 = f7 + f8;
                d6 = d7 + d8;
                break;
            case 6:
                v7 = v8 + v9;
                f7 = f8 + f1;
                d7 = d8 + d1;
                break;
            case 7:
                v8 = v9 + v10;
                f8 = f1 + f2;
                d8 = d1 + d2;
                break;
            case 8:
                v9 = v10 + v11;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 9:
                v10 = v11 + v12;
                f2 = f3 + f4;
                d2 = d3 + d4;
                break;
            case 10:
                v11 = v12 + v13;
                f3 = f4 + f5;
                d3 = d4 + d5;
                break;
            case 11:
                v12 = v13 + v14;
                f4 = f5 + f6;
                d4 = d5 + d6;
                break;
            case 12:
                v13 = v14 + v15;
                f5 = f6 + f7;
                d5 = d6 + d7;
                break;
            case 13:
                v14 = v15 + v16;
                f6 = f7 + f8;
                d6 = d7 + d8;
                break;
            case 14:
                v15 = v16 + v17;
                f7 = f8 + f1;
                d7 = d8 + d1;
                break;
        }
        
        /* Force variables to be live across function call */
        if (i % 3 == 0) {
            use_vars(&v1, &f1, &d1);
        } else if (i % 3 == 1) {
            use_vars(&v2, &f2, &d2);
        } else {
            use_vars(&v3, &f3, &d3);
        }
        
        /* More register clobbering */
        asm volatile (
            "# Clobber more registers"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
        );
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 2; j++) {
            v21 += v22;
            v22 += v23;
            v23 += v24;
            f1 += 0.1f;
            f2 += 0.2f;
        }
    }
    
    /* Compute checksum from all variables to prevent elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                   (int)f5 + (int)f6 + (int)f7 + (int)f8 +
                   (int)d1 + (int)d2 + (int)d3 + (int)d4 +
                   (int)d5 + (int)d6 + (int)d7 + (int)d8;
    
    return checksum;
}

int main(int argc, char** argv) {
    int iterations = 10;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    int result = test_mcf_function(iterations, seed);
    printf("Result: %d\n", result);
    
    return 0;
}
