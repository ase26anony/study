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

/* Another external function to create more call pressure */
void __attribute__((noinline)) clobber_helper(void) {
    /* Force register clobbering */
    asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi");
}

/* The main test function with high register pressure and complex CFG */
void __attribute__((noinline, optimize("O2"))) 
test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v0 = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed - 100;
    int v4 = seed ^ 0x55;
    int v5 = seed | 0xAA;
    int v6 = seed & 0xFF;
    int v7 = seed << 2;
    int v8 = seed >> 1;
    int v9 = ~seed;
    
    float f0 = seed * 1.1f;
    float f1 = seed * 2.2f;
    float f2 = seed * 3.3f;
    float f3 = seed * 4.4f;
    float f4 = seed * 5.5f;
    
    double d0 = seed * 1.11;
    double d1 = seed * 2.22;
    double d2 = seed * 3.33;
    double d3 = seed * 4.44;
    
    char* p0 = (char*)&v0;
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    
    /* More variables for additional pressure */
    int v10 = v0 + v1;
    int v11 = v2 * v3;
    int v12 = v4 ^ v5;
    int v13 = v6 | v7;
    int v14 = v8 & v9;
    int v15 = v0 - v1;
    int v16 = v2 + v3;
    int v17 = v4 * v5;
    int v18 = v6 ^ v7;
    int v19 = v8 | v9;
    
    float f5 = f0 + f1;
    float f6 = f2 * f3;
    float f7 = f4 / 2.0f;
    float f8 = f0 - f1;
    float f9 = f2 + f3;
    
    double d4 = d0 + d1;
    double d5 = d2 * d3;
    double d6 = d0 / 3.0;
    double d7 = d1 - d2;
    
    /* Array of pointers to force some variables to memory */
    int* ptr_array[20];
    ptr_array[0] = &v0; ptr_array[1] = &v1; ptr_array[2] = &v2;
    ptr_array[3] = &v3; ptr_array[4] = &v4; ptr_array[5] = &v5;
    ptr_array[6] = &v6; ptr_array[7] = &v7; ptr_array[8] = &v8;
    ptr_array[9] = &v9; ptr_array[10] = &v10; ptr_array[11] = &v11;
    ptr_array[12] = &v12; ptr_array[13] = &v13; ptr_array[14] = &v14;
    ptr_array[15] = &v15; ptr_array[16] = &v16; ptr_array[17] = &v17;
    ptr_array[18] = &v18; ptr_array[19] = &v19;
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx");
        
        /* Large switch statement to create complex CFG */
        switch (i % 13) {
            case 0:
                v0 = v1 + v2;
                f0 = f1 * 2.0f;
                d0 = d1 + 1.0;
                break;
            case 1:
                v1 = v3 * v4;
                f1 = f2 / 3.0f;
                d1 = d2 * 2.0;
                break;
            case 2:
                v2 = v5 ^ v6;
                f2 = f3 + f4;
                d2 = d3 - 1.0;
                break;
            case 3:
                v3 = v7 | v8;
                f3 = f0 * f1;
                d3 = d0 / 4.0;
                break;
            case 4:
                v4 = v9 & v10;
                f4 = f2 - f3;
                d4 = d1 + d2;
                break;
            case 5:
                v5 = v11 + v12;
                f5 = f4 * 1.5f;
                d5 = d3 * 1.1;
                break;
            case 6:
                v6 = v13 * v14;
                f6 = f0 / 2.0f;
                d6 = d0 - d1;
                break;
            case 7:
                v7 = v15 ^ v16;
                f7 = f1 + f2;
                d7 = d2 * 3.0;
                break;
            case 8:
                v8 = v17 | v18;
                f8 = f3 * f4;
                /* Nested loop for additional complexity */
                for (int j = 0; j < 3; j++) {
                    v8 += j;
                    asm volatile ("" : : : "memory");
                }
                break;
            case 9:
                v9 = v19 & v0;
                f9 = f5 - f6;
                /* Call to external function with live variables */
                use_vars(ptr_array, 10);
                break;
            case 10:
                v10 = v1 + v3;
                f0 = f7 * 2.0f;
                clobber_helper();
                break;
            case 11:
                v11 = v5 * v7;
                f1 = f8 / 3.0f;
                /* Another register clobber */
                asm volatile ("" : : : "memory", "esi", "edi");
                break;
            case 12:
                v12 = v9 ^ v11;
                f2 = f9 + f0;
                /* Force spill by using many variables */
                v13 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
                break;
        }
        
        /* More computations to extend live ranges */
        if (i % 7 == 0) {
            v14 = v0 * v1 + v2 * v3;
            f3 = f4 * f5 + f6 * f7;
            d0 = d1 + d2 + d3 + d4;
        } else if (i % 7 == 1) {
            v15 = v4 | v5 & v6;
            f4 = f8 - f9 * 2.0f;
            d1 = d5 * d6 - d7;
        } else if (i % 7 == 2) {
            v16 = v7 ^ v8 ^ v9;
            f5 = f0 / f1 + f2;
            d2 = d3 + d4 * 1.5;
        } else {
            v17 = v10 + v11 - v12;
            f6 = f3 * 4.0f / f4;
            d3 = d5 - d6 + d7;
        }
        
        /* Periodic call to force register saves/restores */
        if (i % 5 == 0) {
            use_vars(ptr_array + 5, 15);
        }
    }
    
    /* Final computation to produce a result and prevent elimination */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    float fresult = f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9;
    double dresult = d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7;
    
    /* Use results to prevent dead code elimination */
    asm volatile ("" : : "r"(result), "r"(fresult), "r"(dresult) : "memory");
    
    printf("Checksum: %d (float: %f, double: %f)\n", 
           result, fresult, dresult);
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
    
    test_function(iterations, seed);
    
    return 0;
}
