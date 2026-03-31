/* test-mcf-coverage.c */
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
    
    float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    float f3 = seed * 0.4f, f4 = seed * 0.5f, f5 = seed * 0.6f;
    
    double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    double d3 = seed * 0.04, d4 = seed * 0.05;
    
    char* p0 = (char*)&v0;
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    
    /* Array of pointers to force spilling */
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
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile ("" : : : "memory");
        
        /* Large switch statement creating many basic blocks */
        switch (i % 13) {
            case 0:
                v0 = v1 + v2;
                f0 = f1 + f2;
                d0 = d1 + d2;
                break;
            case 1:
                v3 = v4 * v5;
                f3 = f4 * f5;
                d3 = d4 * 2.0;
                break;
            case 2:
                v6 = v7 - v8;
                f0 = f3 - f4;
                d0 = d3 - d4;
                break;
            case 3:
                v9 = v10 / (v11 ? v11 : 1);
                f1 = f2 / (f3 ? f3 : 1.0f);
                break;
            case 4:
                v12 = v13 | v14;
                v15 = v16 & v17;
                break;
            case 5:
                v18 = v19 ^ v20;
                v21 = ~v22;
                break;
            case 6:
                v23 = v24 << 2;
                v25 = v26 >> 1;
                break;
            case 7:
                f2 = f0 * f1 + f3;
                d2 = d0 * d1 + d3;
                break;
            case 8:
                v27 = (v28 > v29) ? v28 : v29;
                f4 = (f0 > f1) ? f0 : f1;
                break;
            case 9:
                v0 = v1 = v2 = i;
                f0 = f1 = f2 = i * 0.5f;
                break;
            case 10:
                d0 = d1 = d2 = i * 0.25;
                p0 = p1 = p2 = (char*)&i;
                break;
            case 11:
                /* Force spill by using all variables */
                v0 += v1; v2 += v3; v4 += v5; v6 += v7;
                v8 += v9; v10 += v11; v12 += v13; v14 += v15;
                f0 += f1; f2 += f3; f4 += f5;
                d0 += d1; d2 += d3; d4 += 1.0;
                break;
            case 12:
                /* Call external function with many live variables */
                use_vars(ptr_array, 30);
                break;
        }
        
        /* Another inline assembly to clobber specific registers */
        #ifdef __i386__
        asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
        #elif __x86_64__
        asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        #endif
        
        /* Inner loop for more complexity */
        for (int j = 0; j < 3; j++) {
            v0 = (v0 * 1103515245 + 12345) & 0x7fffffff;
            if (j == 1) {
                f0 = f0 * 1.1f + f1;
                d0 = d0 * 1.01 + d1;
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                   v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                   v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                   (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                   (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4;
    
    return checksum;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) seed = atoi(argv[2]);
    
    int result = test_function(iterations, seed);
    printf("Result: %d\n", result);
    
    return 0;
}
