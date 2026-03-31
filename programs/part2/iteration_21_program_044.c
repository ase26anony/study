/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* ptr, float* fptr, double* dptr) {
    volatile int sink = *ptr;
    *fptr += sink;
    *dptr -= sink;
}

/* Another noinline function to create more call edges */
void __attribute__((noinline)) clobber_helper(void) {
    asm volatile ("" : : : "memory");
}

/* The main test function with high register pressure and complex CFG */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed + 5, v6 = seed + 6, v7 = seed + 7, v8 = seed + 8;
    int v9 = seed + 9, v10 = seed + 10, v11 = seed + 11, v12 = seed + 12;
    int v13 = seed + 13, v14 = seed + 14, v15 = seed + 15, v16 = seed + 16;
    int v17 = seed + 17, v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    
    float f1 = seed * 0.1f, f2 = seed * 0.2f, f3 = seed * 0.3f, f4 = seed * 0.4f;
    float f5 = seed * 0.5f, f6 = seed * 0.6f, f7 = seed * 0.7f, f8 = seed * 0.8f;
    
    double d1 = seed * 1.1, d2 = seed * 1.2, d3 = seed * 1.3, d4 = seed * 1.4;
    double d5 = seed * 1.5, d6 = seed * 1.6, d7 = seed * 1.7, d8 = seed * 1.8;
    
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    char* p4 = (char*)&v4;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 15;
        
        /* Inline assembly to clobber registers */
        asm volatile ("# Force register clobbering" 
                     : : : "eax", "ebx", "ecx", "edx", 
                           "esi", "edi", "memory");
        
        /* Large switch statement creating many basic blocks */
        switch (mod) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * f3;
                d1 = d2 - d3;
                use_vars(&v1, &f1, &d1);
                break;
            case 1:
                v4 = v5 ^ v6;
                f4 = f5 / f6;
                d4 = d5 + d6;
                p1 = (char*)&v4;
                break;
            case 2:
                v7 = v8 << 2;
                f7 = f8 + 1.0f;
                d7 = d8 * 2.0;
                use_vars(&v7, &f7, &d7);
                break;
            case 3:
                v9 = v10 | v11;
                f1 = f2 - f3;
                d1 = d2 / d3;
                clobber_helper();
                break;
            case 4:
                v12 = v13 & v14;
                f4 = f5 * f6;
                d4 = d5 - d6;
                asm volatile ("# More clobbering" : : : "r8", "r9", "r10", "r11");
                break;
            case 5:
                v15 = v16 + v17;
                f7 = f8 / 2.0f;
                d7 = d8 + 3.0;
                use_vars(&v15, &f7, &d7);
                break;
            case 6:
                v18 = v19 * v20;
                f1 = f2 + f3;
                d1 = d2 * d3;
                p2 = (char*)&v18;
                break;
            case 7:
                v2 = v3 - v4;
                f4 = f5 - f6;
                d4 = d5 / d6;
                clobber_helper();
                break;
            case 8:
                v5 = v6 >> 1;
                f7 = f8 * 3.0f;
                d7 = d8 - 1.0;
                use_vars(&v5, &f7, &d7);
                break;
            case 9:
                v8 = v9 ^ v10;
                f1 = f2 / f3;
                d1 = d2 + d3;
                asm volatile ("# Even more clobbering" : : : "xmm0", "xmm1", "xmm2");
                break;
            case 10:
                v11 = v12 & v13;
                f4 = f5 + f6;
                d4 = d5 * d6;
                p3 = (char*)&v11;
                break;
            case 11:
                v14 = v15 | v16;
                f7 = f8 - 4.0f;
                d7 = d8 / 2.0;
                use_vars(&v14, &f7, &d7);
                break;
            case 12:
                v17 = v18 + v19;
                f1 = f2 * 2.0f;
                d1 = d2 - d3;
                clobber_helper();
                break;
            case 13:
                v20 = v1 * v2;
                f4 = f5 / 3.0f;
                d4 = d5 + 1.0;
                p4 = (char*)&v20;
                break;
            case 14:
                v3 = v4 ^ v5;
                f7 = f8 + f1;
                d7 = d8 * d1;
                use_vars(&v3, &f7, &d7);
                break;
        }
        
        /* More computations mixing variables */
        v1 = (v1 + v2) * (v3 - v4);
        f1 = f1 + f2 - f3 * f4;
        d1 = d1 / d2 + d3 * d4;
        
        /* Another call site */
        if (i % 3 == 0) {
            use_vars(&v5, &f5, &d5);
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 2; j++) {
            v6 += v7 * j;
            f6 += f7 * j;
            d6 += d7 * j;
        }
    }
    
    /* Compute checksum from all variables */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    checksum += (int)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8);
    checksum += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
    checksum += (int)((long)p1 + (long)p2 + (long)p3 + (long)p4);
    
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
    
    int result = test_function(iterations, seed);
    printf("Result: %d\n", result);
    
    return 0;
}
