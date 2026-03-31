/* test_mcf_coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables stay live across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
    }
    /* Prevent optimization */
    asm volatile("" : : "r"(sum) :);
}

/* Complex function with high register pressure and control flow */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed + 5, v6 = seed + 6, v7 = seed + 7, v8 = seed + 8;
    int v9 = seed + 9, v10 = seed + 10, v11 = seed + 11, v12 = seed + 12;
    int v13 = seed + 13, v14 = seed + 14, v15 = seed + 15, v16 = seed + 16;
    int v17 = seed + 17, v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    int v21 = seed + 21, v22 = seed + 22, v23 = seed + 23, v24 = seed + 24;
    int v25 = seed + 25, v26 = seed + 26, v27 = seed + 27, v28 = seed + 28;
    int v29 = seed + 29, v30 = seed + 30;
    
    /* Floating point variables to use different register classes */
    float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f;
    float f4 = seed * 1.4f, f5 = seed * 1.5f, f6 = seed * 1.6f;
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3;
    
    /* Pointer variables */
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    
    /* Array of pointers to ensure they stay live */
    int* ptr_array[30];
    for (int i = 0; i < 30; i++) {
        ptr_array[i] = &v1 + i;
    }
    
    /* Complex loop with nested control flow */
    for (int i = 0; i < iterations; i++) {
        /* Large switch statement creating many basic blocks */
        switch ((seed + i) % 15) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                /* Clobber registers to force spills */
                asm volatile("" : : : "eax", "ebx", "ecx", "edx", 
                                           "esi", "edi", "memory");
                break;
            case 1:
                v4 = v5 * v6;
                f2 = f3 * f4;
                d2 = d3 * 1.5;
                asm volatile("" : : : "eax", "ebx", "ecx", "edx", "memory");
                break;
            case 2:
                v7 = v8 - v9;
                f3 = f4 - f5;
                d3 = d1 - d2;
                break;
            case 3:
                v10 = v11 ^ v12;
                f4 = f5 * 2.0f;
                break;
            case 4:
                v13 = v14 | v15;
                f5 = f6 / 2.0f;
                asm volatile("" : : : "eax", "ebx", "memory");
                break;
            case 5:
                v16 = v17 & v18;
                d1 = d2 / 3.0;
                break;
            case 6:
                v19 = v20 << 2;
                f6 = f1 * f2;
                break;
            case 7:
                v21 = v22 >> 1;
                d2 = d3 * d1;
                asm volatile("" : : : "ecx", "edx", "memory");
                break;
            case 8:
                v23 = v24 + v25;
                f1 = f3 + f5;
                break;
            case 9:
                v26 = v27 - v28;
                d3 = d1 - d2;
                break;
            case 10:
                v29 = v30 * v1;
                f2 = f4 * f6;
                asm volatile("" : : : "eax", "ebx", "ecx", "edx",
                                           "esi", "edi", "memory");
                break;
            case 11:
                v2 = v3 + v4;
                f3 = f5 + f1;
                break;
            case 12:
                v5 = v6 * v7;
                d1 = d2 * d3;
                break;
            case 13:
                v8 = v9 - v10;
                f4 = f6 - f2;
                asm volatile("" : : : "memory");
                break;
            case 14:
                v11 = v12 ^ v13;
                f5 = f1 * f3;
                d2 = d3 * 2.0;
                break;
        }
        
        /* Function call with many live variables */
        if (i % 7 == 0) {
            use_vars(ptr_array, 30);
            
            /* More register clobbering after call */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", 
                                       "esi", "edi", "memory");
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v1 += j;
            v2 -= j;
            if (j % 2 == 0) {
                f1 += j * 0.5f;
                asm volatile("" : : : "eax", "memory");
            }
        }
    }
    
    /* Compute checksum from all variables to prevent elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
                   (int)d1 + (int)d2 + (int)d3 +
                   (int)(long)p1 + (int)(long)p2 + (int)(long)p3;
    
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
