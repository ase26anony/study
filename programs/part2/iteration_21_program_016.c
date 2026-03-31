/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c) {
    volatile int sink = *a + (int)*b + (int)*c;
    (void)sink;
}

/* Complex test function with high register pressure */
void __attribute__((noinline, optimize("O2"))) 
test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed + 5, v6 = seed + 6, v7 = seed + 7, v8 = seed + 8;
    int v9 = seed + 9, v10 = seed + 10, v11 = seed + 11, v12 = seed + 12;
    int v13 = seed + 13, v14 = seed + 14, v15 = seed + 15, v16 = seed + 16;
    int v17 = seed + 17, v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    
    float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f, f4 = seed * 1.4f;
    float f5 = seed * 1.5f, f6 = seed * 1.6f, f7 = seed * 1.7f, f8 = seed * 1.8f;
    
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3, d4 = seed * 2.4;
    double d5 = seed * 2.5, d6 = seed * 2.6, d7 = seed * 2.7, d8 = seed * 2.8;
    
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    char* p4 = (char*)&v4;
    
    /* Complex control flow with loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                                           "esi", "edi");
        
        /* Large switch statement creating complex CFG */
        switch (i % 13) {
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
                v10 = v11 ^ v12;
                f1 = f4 + f7;
                d1 = d4 * d7;
                break;
            case 4:
                v13 = v14 | v15;
                f2 = f5 * f8;
                d2 = d5 + d8;
                break;
            case 5:
                v16 = v17 & v18;
                f3 = f6 - f9;
                d3 = d6 - d9;
                break;
            case 6:
                v19 = v20 << 2;
                f4 = f7 * 2.0f;
                d4 = d7 / 2.0;
                break;
            case 7:
                v1 = v4 >> 1;
                f5 = f8 + 1.0f;
                d5 = d8 * 1.5;
                break;
            case 8:
                v2 = v5 + i;
                f6 = f9 - i;
                d6 = d9 + i;
                break;
            case 9:
                v3 = v6 * i;
                f7 = f1 * i;
                d7 = d1 / i;
                break;
            case 10:
                v8 = v9 ^ i;
                f8 = f2 + i;
                d8 = d2 - i;
                break;
            case 11:
                v11 = v12 | i;
                f9 = f3 * i;
                d9 = d3 + i;
                break;
            case 12:
                v14 = v15 & i;
                f1 = f4 - i;
                d1 = d4 * i;
                break;
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            /* Force variables to be live across call */
            use_vars(&v1, &f1, &d1);
            
            /* More register clobbering */
            asm volatile ("" : : : "memory", "eax", "ebx", "ecx");
            
            /* Conditional that creates different paths */
            if ((i + j) % 5 == 0) {
                v2 = v3 + v4;
                f2 = f3 + f4;
                d2 = d3 + d4;
            } else if ((i + j) % 5 == 1) {
                v5 = v6 * v7;
                f5 = f6 * f7;
                d5 = d6 * d7;
            } else if ((i + j) % 5 == 2) {
                v8 = v9 - v10;
                f8 = f9 - f10;
                d8 = d9 - d10;
            } else if ((i + j) % 5 == 3) {
                v11 = v12 ^ v13;
                f11 = f12 + f13;
                d11 = d12 * d13;
            } else {
                v14 = v15 | v16;
                f14 = f15 * f16;
                d14 = d15 / d16;
            }
        }
        
        /* Another function call with different variables */
        use_vars(&v10, &f5, &d8);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                   (int)f6 + (int)f7 + (int)f8 + (int)d1 + (int)d2 +
                   (int)d3 + (int)d4 + (int)d5 + (int)d6 + (int)d7 +
                   (int)d8;
    
    /* Use checksum to prevent optimization */
    asm volatile ("" : : "r"(checksum) : "memory");
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
    
    /* Call the complex function */
    test_function(iterations, seed);
    
    printf("Test completed (compiled with MCF dumps enabled)\n");
    printf("Check for generated .*.ira and .*.mcf dump files\n");
    
    return 0;
}
