/* test-mcf-coverage.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
    }
    (void)sum;
}

/* Complex test function with high register pressure */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    int v12 = seed + 13, v13 = seed + 14, v14 = seed + 15, v15 = seed + 16;
    int v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    int v20 = seed + 21, v21 = seed + 22, v22 = seed + 23, v23 = seed + 24;
    int v24 = seed + 25, v25 = seed + 26, v26 = seed + 27, v27 = seed + 28;
    int v28 = seed + 29, v29 = seed + 30;
    
    /* Mix in some float/double variables */
    float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    
    /* Pointer variables */
    char* p0 = (char*)&v0;
    char* p1 = (char*)&v1;
    
    /* Array to pass addresses to external function */
    int* ptr_array[30];
    for (int i = 0; i < 30; i++) {
        ptr_array[i] = &v0 + i;
    }
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                      "esi", "edi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15", "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* Large switch statement creating many basic blocks */
        switch (i % 13) {
            case 0:
                v0 = v1 + v2;
                f0 = f1 + f2;
                d0 = d1 + d2;
                break;
            case 1:
                v3 = v4 * v5;
                v6 = v7 - v8;
                break;
            case 2:
                v9 = v10 ^ v11;
                v12 = v13 | v14;
                break;
            case 3:
                f1 = f0 * 2.0f;
                d1 = d0 / 2.0;
                break;
            case 4:
                v15 = v16 << 2;
                v17 = v18 >> 1;
                break;
            case 5:
                v19 = v20 & v21;
                v22 = ~v23;
                break;
            case 6:
                f2 = f1 - f0;
                d2 = d1 * d0;
                break;
            case 7:
                v24 = v25 + v26;
                v27 = v28 - v29;
                break;
            case 8:
                /* Force spill by using all variables */
                v0 = v1 + v2 + v3 + v4 + v5;
                break;
            case 9:
                /* Complex floating point chain */
                f0 = f1 * f2 + f0;
                d0 = d1 - d2 * d0;
                break;
            case 10:
                /* Pointer arithmetic */
                p0 = p1 + (v0 % 16);
                break;
            case 11:
                /* Mix integer and float */
                v0 = (int)(f0 * 100.0f);
                f0 = (float)v0 / 100.0f;
                break;
            case 12:
                /* Another clobbering asm */
                asm volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx");
                break;
        }
        
        /* Call external function with many live variables */
        if (i % 7 == 0) {
            use_vars(ptr_array, 30);
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v0 += j;
            v1 -= j;
            if (j % 2 == 0) {
                f0 += 0.5f;
                d0 += 0.25;
            }
        }
    }
    
    /* Compute checksum to prevent elimination */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                 (int)f0 + (int)f1 + (int)f2 + (int)d0 + (int)d1 + (int)d2;
    
    return result;
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
