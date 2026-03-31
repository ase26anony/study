/* test-mcf-coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables stay live across calls */
void __attribute__((noinline)) use_vars(int* ptr, float* fptr, double* dptr) {
    volatile int sink = *ptr;
    sink += (int)*fptr;
    sink += (int)*dptr;
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
    
    /* Mix in floating point variables */
    float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f, f4 = seed * 1.4f;
    float f5 = seed * 1.5f, f6 = seed * 1.6f, f7 = seed * 1.7f, f8 = seed * 1.8f;
    
    /* Double precision variables */
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3, d4 = seed * 2.4;
    double d5 = seed * 2.5, d6 = seed * 2.6, d7 = seed * 2.7, d8 = seed * 2.8;
    
    /* Pointer variables */
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    char* p4 = (char*)&v4;
    
    int result = 0;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers registers */
        asm volatile (
            "# Force register clobbering\n"
            : : : "memory", "eax", "ebx", "ecx", "edx", 
                  "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Large switch statement creating complex CFG */
        switch (i % 15) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 1:
                v4 = v5 * v6;
                f4 = f5 * f6;
                d4 = d5 * d6;
                break;
            case 2:
                v7 = v8 - v9;
                f7 = f8 - f9;
                d7 = d8 - d9;
                break;
            case 3:
                v10 = v11 / (v12 ? v12 : 1);
                f10 = f11 / (f12 ? f12 : 1.0f);
                d10 = d11 / (d12 ? d12 : 1.0);
                break;
            case 4:
                v13 = v14 ^ v15;
                result ^= v13;
                break;
            case 5:
                v16 = v17 | v18;
                f5 = f6 + f7;
                break;
            case 6:
                v19 = v20 & v1;
                d5 = d6 * d7;
                break;
            case 7:
                v2 = v3 << 2;
                f2 = f3 * 2.0f;
                break;
            case 8:
                v4 = v5 >> 1;
                d4 = d5 / 2.0;
                break;
            case 9:
                v6 = ~v7;
                f6 = -f7;
                break;
            case 10:
                v8 = v9 % (v10 ? v10 : 1);
                d8 = fmod(d9, d10 ? d10 : 1.0);
                break;
            case 11:
                v11 = v12 + v13 * v14;
                f11 = f12 + f13 * f14;
                break;
            case 12:
                v15 = v16 - v17 / (v18 ? v18 : 1);
                d15 = d16 - d17 / (d18 ? d18 : 1.0);
                break;
            case 13:
                v19 = (v20 << 3) | (v1 >> 2);
                result |= v19;
                break;
            case 14:
                v2 = v3 ^ v4 ^ v5;
                f2 = f3 + f4 - f5;
                d2 = d3 * d4 / d5;
                break;
        }
        
        /* Force variables to be live across function call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1);
        }
        if (i % 5 == 0) {
            use_vars(&v10, &f10, &d10);
        }
        
        /* More inline assembly */
        asm volatile (
            "# More clobbering\n"
            : : : "memory", "r8", "r9", "r10", "r11",
                  "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v1 += j;
            f1 += j * 0.5f;
            d1 += j * 0.25;
            
            /* Conditional with multiple branches */
            if (j == 0) {
                v2 *= 2;
            } else if (j == 1) {
                v3 /= 2;
            } else {
                v4 = v5 + v6;
            }
        }
    }
    
    /* Compute final checksum to prevent optimization */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    result += (int)f5 + (int)f6 + (int)f7 + (int)f8;
    result += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    result += (int)d5 + (int)d6 + (int)d7 + (int)d8;
    
    return result;
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
    
    /* Use result to affect return code */
    return (result % 256);
}
