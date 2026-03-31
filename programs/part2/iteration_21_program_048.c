/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables are live across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
    }
    (void)sum;
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
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3;
    
    /* Pointer variables */
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    
    /* Array to pass pointers to external function */
    int* ptr_array[30];
    for (int i = 0; i < 30; i++) {
        ptr_array[i] = &v1 + i;
    }
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers many registers */
        asm volatile (
            "# Force register clobbering\n"
            : : : "memory", "eax", "ebx", "ecx", "edx", 
                  "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Large switch statement creating complex CFG */
        switch ((seed + i) % 12) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 1:
                v4 = v5 * v6;
                v7 = v8 - v9;
                break;
            case 2:
                v10 = v11 ^ v12;
                v13 = v14 | v15;
                break;
            case 3:
                v16 = v17 & v18;
                v19 = v20 << 2;
                break;
            case 4:
                v21 = v22 >> 1;
                v23 = v24 % 7;
                break;
            case 5:
                v25 = v26 + v27 * v28;
                f2 = f1 * 3.14f;
                break;
            case 6:
                v29 = v30 - v1;
                d2 = d1 / 2.0;
                break;
            case 7:
                v2 = v3 + v4;
                f3 = f1 + f2;
                break;
            case 8:
                v5 = v6 * v7;
                d3 = d1 * d2;
                break;
            case 9:
                v8 = v9 ^ v10;
                f1 = f2 - f3;
                break;
            case 10:
                v11 = v12 & v13;
                v14 = v15 | v16;
                break;
            case 11:
                v17 = v18 + v19;
                v20 = v21 - v22;
                break;
        }
        
        /* Call external function with many live variables */
        use_vars(ptr_array, 30);
        
        /* More computations to extend live ranges */
        v1 = v1 + v2;
        v3 = v3 + v4;
        v5 = v5 + v6;
        f1 = f1 + f2;
        d1 = d1 + d2;
        
        /* Another inline assembly barrier */
        asm volatile (
            "# Another clobber point\n"
            : : : "memory", "eax", "ebx", "ecx", "edx"
        );
        
        /* Nested loop for additional CFG complexity */
        for (int j = 0; j < 3; j++) {
            v1 += j;
            v2 -= j;
            if (j % 2 == 0) {
                v3 *= (j + 1);
            } else {
                v4 /= (j + 1);
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                 (int)f1 + (int)f2 + (int)f3 + (int)d1 + (int)d2 + (int)d3;
    
    return result;
}

int main(int argc, char* argv[]) {
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
