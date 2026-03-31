/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables to be live across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
    }
    (void)sum;
}

/* Main test function with high register pressure and complex CFG */
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
    
    /* Mix in some float/double variables */
    float f1 = seed * 0.1f, f2 = seed * 0.2f, f3 = seed * 0.3f;
    double d1 = seed * 0.01, d2 = seed * 0.02, d3 = seed * 0.03;
    
    /* Pointer variables to add aliasing complexity */
    int* ptrs[10];
    ptrs[0] = &v1; ptrs[1] = &v2; ptrs[2] = &v3; ptrs[3] = &v4; ptrs[4] = &v5;
    ptrs[5] = &v6; ptrs[6] = &v7; ptrs[7] = &v8; ptrs[8] = &v9; ptrs[9] = &v10;
    
    /* Complex loop with nested control flow */
    for (int i = 0; i < iterations; i++) {
        /* Large switch statement to create many basic blocks */
        switch (i % 13) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                /* Inline asm that clobbers registers */
                asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "memory");
                break;
            case 1:
                v4 = v5 * v6;
                d1 = d2 * d3;
                asm volatile ("" : : : "esi", "edi", "memory");
                break;
            case 2:
                v7 = v8 ^ v9;
                v10 = v11 | v12;
                break;
            case 3:
                v13 = v14 - v15;
                f2 = f3 - f1;
                asm volatile ("" : : : "eax", "ebx", "memory");
                break;
            case 4:
                v16 = v17 & v18;
                d2 = d3 / 2.0;
                break;
            case 5:
                v19 = v20 << 2;
                v21 = v22 >> 1;
                asm volatile ("" : : : "ecx", "edx", "memory");
                break;
            case 6:
                v23 = v24 + v25;
                f3 = f1 * f2;
                break;
            case 7:
                v26 = v27 - v28;
                d3 = d1 + d2;
                asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
                break;
            case 8:
                v29 = v30 * 3;
                v1 = v2 / 2;
                break;
            case 9:
                v3 = v4 + v5;
                f1 = f2 - f3;
                asm volatile ("" : : : "memory");
                break;
            case 10:
                v6 = v7 ^ v8;
                d1 = d2 * 1.5;
                break;
            case 11:
                v9 = v10 | v11;
                v12 = v13 & v14;
                asm volatile ("" : : : "eax", "ebx", "memory");
                break;
            case 12:
                v15 = v16 << 1;
                v17 = v18 >> 2;
                f2 = f3 / 2.0f;
                break;
        }
        
        /* Nested loop to add more CFG complexity */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 5 == 0) {
                v19 += v20;
                asm volatile ("" : : : "ecx", "memory");
            } else if ((i + j) % 5 == 1) {
                v21 -= v22;
            } else if ((i + j) % 5 == 2) {
                v23 *= v24;
                asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "memory");
            } else if ((i + j) % 5 == 3) {
                v25 ^= v26;
            } else {
                v27 |= v28;
            }
        }
        
        /* Call external function to force variables live across call */
        if (i % 7 == 0) {
            use_vars(ptrs, 10);
            asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
        }
        
        /* More computations to keep variables alive */
        v29 = (v29 * 1103515245 + 12345) & 0x7fffffff;
        v30 = (v30 * 1664525 + 1013904223) & 0x7fffffff;
        f1 = f1 * 1.01f;
        f2 = f2 * 0.99f;
        d1 = d1 * 1.001;
        d2 = d2 * 0.999;
    }
    
    /* Compute checksum from all variables to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                   (int)f1 + (int)f2 + (int)f3 + (int)d1 + (int)d2 + (int)d3;
    
    return checksum;
}

int main(int argc, char* argv[]) {
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
