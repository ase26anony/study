/* test_mcf_coverage.c - Trigger MCF solver special node printing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
    }
    asm volatile ("" : : "r"(sum) : );
}

/* Complex function with high register pressure and control flow */
__attribute__((noinline, optimize("O2")))
int test_mcf_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed + 5, v6 = seed + 6, v7 = seed + 7, v8 = seed + 8;
    int v9 = seed + 9, v10 = seed + 10, v11 = seed + 11, v12 = seed + 12;
    int v13 = seed + 13, v14 = seed + 14, v15 = seed + 15, v16 = seed + 16;
    int v17 = seed + 17, v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    int v21 = seed + 21, v22 = seed + 22, v23 = seed + 23, v24 = seed + 24;
    int v25 = seed + 25, v26 = seed + 26, v27 = seed + 27, v28 = seed + 28;
    int v29 = seed + 29, v30 = seed + 30;
    
    /* Mix in some floating point variables */
    float f1 = seed * 0.1f, f2 = seed * 0.2f, f3 = seed * 0.3f;
    double d1 = seed * 0.01, d2 = seed * 0.02, d3 = seed * 0.03;
    
    /* Pointer variables */
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    
    /* Array to pass addresses to external function */
    int* ptr_array[30];
    for (int i = 0; i < 30; i++) {
        ptr_array[i] = &v1 + i;
    }
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 15;
        
        /* Large switch statement creates complex CFG */
        switch (mod) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * f3;
                /* Clobber registers around computation */
                asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "memory");
                break;
            case 1:
                v4 = v5 - v6;
                d1 = d2 / d3;
                asm volatile ("" : : : "esi", "edi", "memory");
                break;
            case 2:
                v7 = v8 * v9;
                f2 = f1 + f3;
                break;
            case 3:
                v10 = v11 ^ v12;
                d2 = d1 - d3;
                asm volatile ("" : : : "eax", "ebx", "memory");
                break;
            case 4:
                v13 = v14 | v15;
                f3 = f1 * f2;
                break;
            case 5:
                v16 = v17 & v18;
                d3 = d1 + d2;
                asm volatile ("" : : : "ecx", "edx", "memory");
                break;
            case 6:
                v19 = v20 << 2;
                f1 = f2 - f3;
                break;
            case 7:
                v21 = v22 >> 1;
                d1 = d2 * d3;
                asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
                break;
            case 8:
                v23 = ~v24;
                f2 = f3 / f1;
                break;
            case 9:
                v25 = v26 + v27;
                d2 = d3 - d1;
                asm volatile ("" : : : "memory");
                break;
            case 10:
                v28 = v29 * v30;
                f3 = f1 + f2;
                break;
            case 11:
                v1 = v2 ^ v3;
                d3 = d1 / d2;
                asm volatile ("" : : : "eax", "ebx", "memory");
                break;
            case 12:
                v4 = v5 | v6;
                f1 = f2 * f3;
                break;
            case 13:
                v7 = v8 & v9;
                d1 = d2 + d3;
                asm volatile ("" : : : "ecx", "edx", "memory");
                break;
            case 14:
                v10 = v11 << 3;
                f2 = f3 - f1;
                break;
        }
        
        /* Force variables live across function call */
        if (i % 7 == 0) {
            use_vars(ptr_array, 30);
            asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
        }
        
        /* More computations mixing variables */
        v30 = v1 + v2 + v3 + v4;
        v29 = v5 * v6 * v7 / 8;
        f1 = f2 + f3 * 0.5f;
        d1 = d2 - d3 * 0.25;
        
        /* Another inline asm to force spills */
        asm volatile ("# Force register pressure" : : : "memory");
    }
    
    /* Complex final computation to prevent dead code elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                 (int)f1 + (int)f2 + (int)f3 + (int)d1 + (int)d2 + (int)d3;
    
    /* Use pointers to ensure they're live */
    result += *p1 + *p2 + *p3;
    
    return result;
}

/* Another function to create more CFG complexity */
__attribute__((noinline))
int helper_function(int x, int y) {
    volatile int a = x, b = y;
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a * b + (a ^ b) - (a & b);
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
    
    /* Call complex function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        int result = test_mcf_function(iterations, seed + i);
        total += result;
        
        /* Call helper to create more interprocedural complexity */
        total += helper_function(result, i);
    }
    
    printf("Result checksum: %d\n", total);
    return total != 0 ? 0 : 1;
}
