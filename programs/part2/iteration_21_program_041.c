/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer -o test_mcf test_mcf_coverage.c
 */

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
    
    /* Pointer variables */
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    
    /* Array to pass pointers to external function */
    int* ptr_array[30];
    for (int i = 0; i < 30; i++) {
        ptr_array[i] = &v1 + i;
    }
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 15;
        
        /* Inline assembly that clobbers registers */
        asm volatile (
            "# Force register clobbering\n"
            : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Large switch statement creating complex CFG */
        switch (mod) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * f3;
                break;
            case 1:
                v4 = v5 - v6;
                d1 = d2 / d3;
                break;
            case 2:
                v7 = v8 * v9;
                p1 = (char*)&v10;
                break;
            case 3:
                v10 = v11 ^ v12;
                f2 = f1 + f3;
                break;
            case 4:
                v13 = v14 | v15;
                d2 = d1 - d3;
                break;
            case 5:
                v16 = v17 & v18;
                p2 = p1 + 4;
                break;
            case 6:
                v19 = v20 << 2;
                f3 = f1 * f2;
                break;
            case 7:
                v21 = v22 >> 1;
                d3 = d1 + d2;
                break;
            case 8:
                v23 = v24 + v25;
                p3 = p2 - 4;
                break;
            case 9:
                v26 = v27 - v28;
                f1 = f2 / f3;
                break;
            case 10:
                v29 = v30 * v1;
                d1 = d2 * d3;
                break;
            case 11:
                v2 = v3 + v4;
                f2 = f3 - f1;
                break;
            case 12:
                v5 = v6 ^ v7;
                d2 = d3 / d1;
                break;
            case 13:
                v8 = v9 | v10;
                p1 = (char*)&v11;
                break;
            case 14:
                v11 = v12 & v13;
                f3 = f1 + f2;
                break;
        }
        
        /* Call external function forcing variables to be live */
        if (i % 7 == 0) {
            use_vars(ptr_array, 30);
            
            /* More inline assembly */
            asm volatile (
                "# More register pressure\n"
                : : : "eax", "ebx", "ecx", "edx", "memory"
            );
        }
        
        /* Complex computation mixing all variables */
        v1 = (v1 + v2 + v3 + v4 + v5) % 1000;
        v6 = (v6 + v7 + v8 + v9 + v10) % 1000;
        v11 = (v11 + v12 + v13 + v14 + v15) % 1000;
        v16 = (v16 + v17 + v18 + v19 + v20) % 1000;
        v21 = (v21 + v22 + v23 + v24 + v25) % 1000;
        v26 = (v26 + v27 + v28 + v29 + v30) % 1000;
        
        f1 = f1 * 0.99f + f2 * 0.01f;
        f2 = f2 * 0.98f + f3 * 0.02f;
        f3 = f3 * 0.97f + f1 * 0.03f;
        
        d1 = d1 * 0.999 + d2 * 0.001;
        d2 = d2 * 0.998 + d3 * 0.002;
        d3 = d3 * 0.997 + d1 * 0.003;
    }
    
    /* Final computation to create a checksum */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                 (int)f1 + (int)f2 + (int)f3 + (int)d1 + (int)d2 + (int)d3;
    
    return result % 10000;
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
