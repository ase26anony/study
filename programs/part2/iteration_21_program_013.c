/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables stay live across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
    }
    (void)sum;
}

/* Main test function with high register pressure and complex CFG */
void __attribute__((noinline, optimize("O2"))) 
test_function(int iterations, int seed) {
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
    
    /* Pointer variables that might need registers */
    char* p0 = (char*)&v0;
    char* p1 = (char*)&v1;
    
    /* Array of pointers to ensure variables are address-taken */
    int* ptr_array[30];
    ptr_array[0] = &v0; ptr_array[1] = &v1; ptr_array[2] = &v2;
    ptr_array[3] = &v3; ptr_array[4] = &v4; ptr_array[5] = &v5;
    ptr_array[6] = &v6; ptr_array[7] = &v7; ptr_array[8] = &v8;
    ptr_array[9] = &v9; ptr_array[10] = &v10; ptr_array[11] = &v11;
    ptr_array[12] = &v12; ptr_array[13] = &v13; ptr_array[14] = &v14;
    ptr_array[15] = &v15; ptr_array[16] = &v16; ptr_array[17] = &v17;
    ptr_array[18] = &v18; ptr_array[19] = &v19; ptr_array[20] = &v20;
    ptr_array[21] = &v21; ptr_array[22] = &v22; ptr_array[23] = &v23;
    ptr_array[24] = &v24; ptr_array[25] = &v25; ptr_array[26] = &v26;
    ptr_array[27] = &v27; ptr_array[28] = &v28; ptr_array[29] = &v29;
    
    /* Complex loop with nested control flow */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile (
            "# Clobber many registers\n\t"
            : : : "memory", "eax", "ebx", "ecx", "edx", 
                  "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Large switch statement creating complex CFG */
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
                d1 = d0 * 3.0;
                break;
            case 4:
                v15 = v16 << 2;
                v17 = v18 >> 1;
                break;
            case 5:
                v19 = v20 & v21;
                v22 = v23 % 7;
                break;
            case 6:
                f2 = f1 - f0;
                d2 = d1 - d0;
                break;
            case 7:
                v24 = v25 + v26;
                v27 = v28 - v29;
                break;
            case 8:
                /* Call external function with address-taken variables */
                use_vars(ptr_array, 10);
                break;
            case 9:
                v0 = v29 - v28;
                v1 = v27 - v26;
                break;
            case 10:
                f0 = f2 * f1;
                d0 = d2 * d1;
                break;
            case 11:
                v2 = v3 ^ v4;
                v5 = v6 | v7;
                break;
            case 12:
                /* Another inline assembly with different clobbers */
                asm volatile (
                    "# Clobber more registers\n\t"
                    : : : "memory", "ebp", "esp", "xmm4", "xmm5", "xmm6", "xmm7"
                );
                v8 = v9 + v10;
                v11 = v12 - v13;
                break;
        }
        
        /* Additional control flow with if-else chains */
        if (i % 3 == 0) {
            v14 = v15 + 1;
            if (i % 7 == 0) {
                v16 = v17 * 2;
            } else if (i % 5 == 0) {
                v18 = v19 / 3;
            } else {
                v20 = v21 % 11;
            }
        } else if (i % 4 == 0) {
            v22 = v23 << 1;
        }
        
        /* Mix float and int operations */
        f0 += 0.5f;
        d0 += 0.25;
        v0 += (int)f0;
        v1 += (int)d0;
    }
    
    /* Final use of all variables to prevent dead code elimination */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                 (int)f0 + (int)f1 + (int)f2 + (int)d0 + (int)d1 + (int)d2;
    
    /* Store result in a global to ensure it's used */
    extern volatile int g_result;
    g_result = result;
}

volatile int g_result;

int main(int argc, char** argv) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    /* Call test function multiple times with different seeds */
    for (int i = 0; i < 3; i++) {
        test_function(iterations, seed + i * 100);
    }
    
    printf("Result: %d\n", g_result);
    return g_result != 0 ? 0 : 1;
}
