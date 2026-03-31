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
    /* Prevent optimization */
    asm volatile("" : : "r"(sum) : "memory");
}

/* Complex function with high register pressure */
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
    float f3 = seed * 0.4f, f4 = seed * 0.5f;
    double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    
    /* Pointer variables */
    char* p0 = (char*)&v0;
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    
    /* Array to pass pointers to external function */
    int* ptr_array[30];
    for (int i = 0; i < 30; i++) {
        ptr_array[i] = &v0 + i;
    }
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers many registers */
        asm volatile("" : : : 
            "eax", "ebx", "ecx", "edx", "esi", "edi", 
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
            "memory");
        
        /* Large switch statement creating many basic blocks */
        switch ((i + seed) % 15) {
            case 0:
                v0 = v1 + v2;
                f0 = f1 + f2;
                d0 = d1 + d2;
                break;
            case 1:
                v1 = v3 * v4;
                f1 = f3 * f4;
                d1 = d0 * 2.0;
                break;
            case 2:
                v2 = v5 - v6;
                f2 = f0 - f1;
                d2 = d1 - d0;
                break;
            case 3:
                v3 = v7 / (v8 ? v8 : 1);
                f3 = f2 / (f1 ? f1 : 1.0f);
                break;
            case 4:
                v4 = v9 | v10;
                v5 = v11 & v12;
                break;
            case 5:
                v6 = v13 ^ v14;
                v7 = v15 << 2;
                break;
            case 6:
                v8 = v16 >> 1;
                v9 = ~v17;
                break;
            case 7:
                v10 = v18 + v19;
                v11 = v20 - v21;
                break;
            case 8:
                v12 = v22 * v23;
                v13 = v24 / (v25 ? v25 : 1);
                break;
            case 9:
                v14 = v26 | v27;
                v15 = v28 & v29;
                break;
            case 10:
                f0 = f1 * f2;
                f1 = f3 + f4;
                break;
            case 11:
                f2 = f0 - f1;
                f3 = f2 * f4;
                break;
            case 12:
                d0 = d1 + d2;
                d1 = d0 * 1.5;
                break;
            case 13:
                d2 = d1 - d0;
                *p0 = (char)v0;
                *p1 = (char)v1;
                break;
            case 14:
                /* Call external function with many live variables */
                use_vars(ptr_array, 30);
                break;
        }
        
        /* Another inline assembly clobber */
        asm volatile("" : : : 
            "eax", "ebx", "ecx", "edx",
            "xmm6", "xmm7", "xmm8", "xmm9",
            "memory");
        
        /* Nested loop with conditional */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 5 == 0) {
                v0 += v1;
                f0 += f1;
                d0 += d1;
            } else if ((i + j) % 5 == 1) {
                v1 -= v2;
                f1 -= f2;
                d1 -= d2;
            } else if ((i + j) % 5 == 2) {
                v2 *= v3;
                f2 *= f3;
                d2 *= d0;
            } else if ((i + j) % 5 == 3) {
                /* Another call with live variables */
                int* temp_ptrs[5] = {&v0, &v1, &v2, &v3, &v4};
                use_vars(temp_ptrs, 5);
            } else {
                v3 = v4 ^ v5;
                f3 = f4 * 0.5f;
                d0 = d1 / 2.0;
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                           v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                           v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                           (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                           (int)d0 + (int)d1 + (int)d2;
    
    /* Force output */
    printf("Checksum: %d\n", checksum);
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
    
    return 0;
}
