/* test_mcf_coverage.c */
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

/* Complex function with high register pressure and control flow */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int mode) {
    /* Declare many local variables to create register pressure */
    int v1 = mode + 1, v2 = mode + 2, v3 = mode + 3, v4 = mode + 4;
    int v5 = mode + 5, v6 = mode + 6, v7 = mode + 7, v8 = mode + 8;
    int v9 = mode + 9, v10 = mode + 10, v11 = mode + 11, v12 = mode + 12;
    int v13 = mode + 13, v14 = mode + 14, v15 = mode + 15, v16 = mode + 16;
    int v17 = mode + 17, v18 = mode + 18, v19 = mode + 19, v20 = mode + 20;
    int v21 = mode + 21, v22 = mode + 22, v23 = mode + 23, v24 = mode + 24;
    int v25 = mode + 25, v26 = mode + 26, v27 = mode + 27, v28 = mode + 28;
    int v29 = mode + 29, v30 = mode + 30;
    
    /* Floating point variables to use different register sets */
    float f1 = mode * 1.1f, f2 = mode * 2.2f, f3 = mode * 3.3f;
    float f4 = mode * 4.4f, f5 = mode * 5.5f;
    double d1 = mode * 1.11, d2 = mode * 2.22, d3 = mode * 3.33;
    
    /* Pointer variables */
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    
    /* Array of pointers to force some variables to memory */
    int* ptr_array[10];
    ptr_array[0] = &v1; ptr_array[1] = &v2; ptr_array[2] = &v3;
    ptr_array[3] = &v4; ptr_array[4] = &v5; ptr_array[5] = &v6;
    ptr_array[6] = &v7; ptr_array[7] = &v8; ptr_array[8] = &v9;
    ptr_array[9] = &v10;
    
    int result = 0;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers registers */
        asm volatile (
            "# Force register clobbering\n"
            : : : "memory", "eax", "ebx", "ecx", "edx", 
                  "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Large switch statement creating complex CFG */
        switch ((i + mode) % 15) {
            case 0:
                v1 = v2 + v3; v4 = v5 * v6;
                f1 = f2 + f3; d1 = d2 - d3;
                result += v1 + (int)f1;
                break;
            case 1:
                v7 = v8 - v9; v10 = v11 / (v12 ? v12 : 1);
                f2 = f1 * f3; d2 = d1 + d3;
                result += v7 + (int)f2;
                break;
            case 2:
                v13 = v14 ^ v15; v16 = v17 | v18;
                f3 = f1 - f2; d3 = d1 * d2;
                result += v13 + (int)f3;
                break;
            case 3:
                v19 = v20 & v21; v22 = v23 << 2;
                f4 = f5 * 2.0f; 
                result += v19 + (int)f4;
                break;
            case 4:
                v24 = v25 >> 1; v26 = v27 % (v28 ? v28 : 1);
                f5 = f1 / (f2 ? f2 : 1.0f);
                result += v24 + (int)f5;
                break;
            case 5:
                v29 = v30 * 3; v1 = v2 + v4;
                d1 = d2 / (d3 ? d3 : 1.0);
                result += v29 + (int)d1;
                break;
            case 6:
                v3 = v5 - v6; v7 = v8 * v9;
                d2 = d3 + d1;
                result += v3 + (int)d2;
                break;
            case 7:
                v10 = v11 ^ v12; v13 = v14 | v15;
                d3 = d1 - d2;
                result += v10 + (int)d3;
                break;
            case 8:
                v16 = v17 & v18; v19 = v20 << 3;
                result += v16 * 2;
                break;
            case 9:
                v21 = v22 >> 2; v23 = v24 % (v25 ? v25 : 1);
                result += v21 - v23;
                break;
            case 10:
                v26 = v27 * 4; v28 = v29 + v30;
                result += v26 | v28;
                break;
            case 11:
                v1 = v3 ^ v5; v2 = v4 & v6;
                result += v1 ^ v2;
                break;
            case 12:
                v7 = v8 << 1; v9 = v10 >> 1;
                result += v7 + v9;
                break;
            case 13:
                v11 = v12 * v13; v14 = v15 - v16;
                result += v11 % (v14 ? v14 : 1);
                break;
            case 14:
                v17 = v18 + v19; v20 = v21 * v22;
                result += v17 & v20;
                break;
        }
        
        /* Call external function making variables live across call */
        if (i % 7 == 0) {
            use_vars(ptr_array, 10);
            
            /* More register clobbering */
            asm volatile (
                "# More clobbering after call\n"
                : : : "memory", "eax", "ebx", "ecx", "edx",
                      "xmm4", "xmm5", "xmm6", "xmm7"
            );
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v1 += j; v2 -= j; v3 *= (j + 1);
            f1 += j * 0.5f; f2 -= j * 0.25f;
        }
    }
    
    /* Final computation using all variables */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    result += (int)d1 + (int)d2 + (int)d3;
    result += (int)(p1 - p2) + (int)(p2 - p3);
    
    return result;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int mode = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    if (argc > 2) {
        mode = atoi(argv[2]);
    }
    
    int result = test_function(iterations, mode);
    
    printf("Result: %d\n", result);
    
    /* Use result to prevent optimization */
    if (result == 0x12345678) {
        printf("Impossible!\n");
    }
    
    return 0;
}
