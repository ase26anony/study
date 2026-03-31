/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer -o test_mcf test_mcf_coverage.c
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
    asm volatile("" : : "r"(sum));
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
    
    /* Mixed types to use different register classes */
    float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f;
    float f4 = seed * 1.4f, f5 = seed * 1.5f;
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3;
    char* s1 = "str1", *s2 = "str2", *s3 = "str3";
    
    /* Array of pointers to ensure some variables have their address taken */
    int* ptrs[30];
    ptrs[0] = &v1; ptrs[1] = &v2; ptrs[2] = &v3; ptrs[3] = &v4; ptrs[4] = &v5;
    ptrs[5] = &v6; ptrs[6] = &v7; ptrs[7] = &v8; ptrs[8] = &v9; ptrs[9] = &v10;
    ptrs[10] = &v11; ptrs[11] = &v12; ptrs[12] = &v13; ptrs[13] = &v14;
    ptrs[14] = &v15; ptrs[15] = &v16; ptrs[16] = &v17; ptrs[17] = &v18;
    ptrs[18] = &v19; ptrs[19] = &v20; ptrs[20] = &v21; ptrs[21] = &v22;
    ptrs[22] = &v23; ptrs[23] = &v24; ptrs[24] = &v25; ptrs[25] = &v26;
    ptrs[26] = &v27; ptrs[27] = &v28; ptrs[28] = &v29; ptrs[29] = &v30;
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers many registers */
        asm volatile("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                                           "esi", "edi", "xmm0", "xmm1", "xmm2");
        
        /* Large switch statement creating many basic blocks */
        switch (i % 13) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * 1.5f;
                d1 = d2 + d3;
                break;
            case 1:
                v4 = v5 * v6;
                f2 = f3 / 2.0f;
                d2 = d1 - d3;
                break;
            case 2:
                v7 = v8 ^ v9;
                f3 = f1 + f4;
                d3 = d1 * d2;
                break;
            case 3:
                v10 = v11 | v12;
                f4 = f5 - f2;
                /* Force spill by using many variables */
                v13 = v14 + v15 + v16 + v17;
                break;
            case 4:
                v18 = v19 & v20;
                f5 = f1 * f3;
                v21 = v22 - v23;
                break;
            case 5:
                v24 = v25 << 2;
                v26 = v27 >> 1;
                f1 = f2 + f3 + f4 + f5;
                break;
            case 6:
                v28 = v29 % (v30 + 1);
                d1 = d2 / (d3 + 1.0);
                v1 = v2 * v3 * v4;
                break;
            case 7:
                v5 = v6 + v7 + v8;
                f2 = f3 * f4 * f5;
                d2 = d1 + d3;
                break;
            case 8:
                v9 = v10 - v11 - v12;
                f3 = f4 / f1;
                v13 = v14 | v15 | v16;
                break;
            case 9:
                v17 = v18 ^ v19 ^ v20;
                f4 = f5 + f1 + f2;
                d3 = d1 - d2;
                break;
            case 10:
                v21 = v22 & v23 & v24;
                f5 = f1 - f2 - f3;
                v25 = v26 + v27 + v28;
                break;
            case 11:
                v29 = v30 << (i % 4);
                v1 = v2 >> 1;
                f1 = f2 * f3 * f4 * f5;
                break;
            case 12:
                v3 = v4 + v5 + v6 + v7 + v8;
                d1 = d2 * d3 * 0.5;
                f2 = f3 + f4 + f5;
                break;
        }
        
        /* Call external function with addresses of variables */
        if (i % 7 == 0) {
            use_vars(ptrs, 30);
            
            /* More register clobbering */
            asm volatile("" : : : "memory", "eax", "ebx", "ecx", "edx",
                                               "xmm3", "xmm4", "xmm5", "xmm6");
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < (i % 5); j++) {
            v1 += j;
            v2 -= j;
            f1 += j * 0.1f;
            d1 += j * 0.01;
            
            /* Small switch inside nested loop */
            switch (j % 3) {
                case 0: v3 *= 2; break;
                case 1: v4 /= 2; break;
                case 2: v5 = v5 ^ j; break;
            }
        }
    }
    
    /* Compute checksum from all variables to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                   (int)d1 + (int)d2 + (int)d3 +
                   (int)(long)s1 + (int)(long)s2 + (int)(long)s3;
    
    return checksum;
}

int main(int argc, char* argv[]) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    int result = test_function(iterations, seed);
    printf("Result: %d\n", result);
    
    return 0;
}
