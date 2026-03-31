/* test-mcf-coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables to be live across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
    }
    asm volatile ("" : : "r"(sum) : "memory");
}

/* Complex test function with high register pressure */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many variables to create register pressure */
    int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    int v12 = seed + 13, v13 = seed + 14, v14 = seed + 15, v15 = seed + 16;
    int v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    int v20 = seed + 21, v21 = seed + 22, v22 = seed + 23, v23 = seed + 24;
    int v24 = seed + 25, v25 = seed + 26, v26 = seed + 27, v27 = seed + 28;
    int v28 = seed + 29, v29 = seed + 30, v30 = seed + 31, v31 = seed + 32;
    
    /* Floating point variables to use FP registers */
    float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    float f4 = seed * 0.5f, f5 = seed * 0.6f, f6 = seed * 0.7f, f7 = seed * 0.8f;
    
    /* Double variables for more FP pressure */
    double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    
    /* Pointer variables */
    char* p0 = (char*)&v0;
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    
    /* Array of pointers to force spilling */
    int* ptr_array[32];
    for (int i = 0; i < 32; i++) {
        ptr_array[i] = &v0 + i;
    }
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers registers */
        asm volatile ("# Force register clobbering" 
                     : : : "eax", "ebx", "ecx", "edx", 
                           "esi", "edi", "memory");
        
        /* Large switch statement to create complex CFG */
        switch (i % 13) {
            case 0:
                v0 = v1 + v2;
                f0 = f1 + f2;
                d0 = d1 + d2;
                break;
            case 1:
                v3 = v4 * v5;
                f3 = f4 * f5;
                d3 = d1 * d2;
                break;
            case 2:
                v6 = v7 - v8;
                f6 = f7 - f8;
                d0 = d1 - d2;
                break;
            case 3:
                v9 = v10 / (v11 ? v11 : 1);
                f9 = f10 / (f11 ? f11 : 1.0f);
                break;
            case 4:
                v12 = v13 ^ v14;
                v15 = v16 | v17;
                break;
            case 5:
                v18 = v19 & v20;
                v21 = v22 << 2;
                break;
            case 6:
                v23 = v24 >> 1;
                f0 = f1 * f2 + f3;
                break;
            case 7:
                d0 = d1 * d2 - d3;
                v25 = v26 + v27;
                break;
            case 8:
                v28 = v29 * v30;
                f4 = f5 / f6;
                break;
            case 9:
                v31 = v0 - v1;
                d1 = d2 + d3;
                break;
            case 10:
                f7 = f0 * f1;
                v2 = v3 + v4;
                break;
            case 11:
                v5 = v6 * v7;
                f2 = f3 - f4;
                break;
            case 12:
                v8 = v9 ^ v10;
                d2 = d0 * d1;
                break;
        }
        
        /* Call external function to force variables live across call */
        if (i % 7 == 0) {
            use_vars(ptr_array, 16);
            
            /* More inline assembly after call */
            asm volatile ("# More clobbering after call" 
                         : : : "eax", "ebx", "ecx", "edx", 
                               "xmm0", "xmm1", "xmm2", "xmm3",
                               "memory");
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v0 += j;
            v1 -= j;
            f0 += j * 0.1f;
            
            /* Conditional with multiple branches */
            if (j == 0) {
                v2 = v3 * 2;
            } else if (j == 1) {
                v4 = v5 / 2;
            } else {
                v6 = v7 + v8;
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                   v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                   v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                   v30 + v31;
    
    checksum += (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
    checksum += (int)(d0 + d1 + d2 + d3);
    
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
