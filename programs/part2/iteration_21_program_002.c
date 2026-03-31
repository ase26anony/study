/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables to be live across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        if (ptrs[i]) sum += *ptrs[i];
    }
    /* Prevent optimization */
    asm volatile("" : : "r"(sum));
}

/* Another external function to create more call pressure */
float __attribute__((noinline)) compute_factor(float a, float b) {
    volatile float result = a * b - a / (b + 1.0f);
    asm volatile("" : : "r"(result));
    return result;
}

/* Main test function with high register pressure and complex CFG */
void __attribute__((noinline, optimize("O2"))) 
test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed * 2, v6 = seed * 3, v7 = seed * 4, v8 = seed * 5;
    int v9 = seed ^ 0x55, v10 = seed ^ 0xAA, v11 = seed ^ 0xFF;
    int v12 = seed << 1, v13 = seed << 2, v14 = seed << 3;
    int v15 = ~seed, v16 = seed | 0xF0, v17 = seed & 0x0F;
    int v18 = seed % 17, v19 = seed % 23, v20 = seed % 31;
    
    /* Floating point variables to use FP registers */
    float f1 = seed * 0.1f, f2 = seed * 0.2f, f3 = seed * 0.3f;
    float f4 = seed * 0.4f, f5 = seed * 0.5f, f6 = seed * 0.6f;
    double d1 = seed * 0.01, d2 = seed * 0.02, d3 = seed * 0.03;
    
    /* Pointer variables */
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    
    /* Array to hold pointers for function call */
    int* ptr_array[20];
    ptr_array[0] = &v1; ptr_array[1] = &v2; ptr_array[2] = &v3;
    ptr_array[3] = &v4; ptr_array[4] = &v5; ptr_array[5] = &v6;
    ptr_array[6] = &v7; ptr_array[7] = &v8; ptr_array[8] = &v9;
    ptr_array[9] = &v10; ptr_array[10] = &v11; ptr_array[11] = &v12;
    ptr_array[12] = &v13; ptr_array[13] = &v14; ptr_array[14] = &v15;
    ptr_array[15] = &v16; ptr_array[16] = &v17; ptr_array[17] = &v18;
    ptr_array[18] = &v19; ptr_array[19] = &v20;
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers many registers */
        asm volatile("" : : : "eax", "ebx", "ecx", "edx", 
                     "esi", "edi", "memory");
        
        /* Large switch statement creating many basic blocks */
        switch (i % 13) {
            case 0:
                v1 = v2 + v3;
                f1 = compute_factor(f1, f2);
                break;
            case 1:
                v4 = v5 - v6;
                f2 = f3 * f4;
                break;
            case 2:
                v7 = v8 * v9;
                d1 = d2 + d3;
                break;
            case 3:
                v10 = v11 / (v12 + 1);
                f3 = f4 - f5;
                break;
            case 4:
                v13 = v14 ^ v15;
                d2 = d1 * 0.5;
                break;
            case 5:
                v16 = v17 | v18;
                f4 = compute_factor(f5, f6);
                break;
            case 6:
                v19 = v20 & v1;
                d3 = d2 / 2.0;
                break;
            case 7:
                v2 = v3 << (i % 4);
                f5 = f6 * 1.1f;
                break;
            case 8:
                v5 = v6 >> (i % 4);
                f6 = f1 + f2;
                break;
            case 9:
                v8 = v9 % (v10 + 1);
                d1 = compute_factor(d1, d2);
                break;
            case 10:
                v11 = ~v12;
                f1 = f3 - f4;
                break;
            case 11:
                v14 = v15 ^ 0xAA;
                d2 = d3 * 1.5;
                break;
            case 12:
                v17 = v18 | 0x55;
                f2 = compute_factor(f3, f4);
                break;
            default:
                v20 = v1 + i;
                d3 = d1 + d2;
                break;
        }
        
        /* Function call that forces many variables to be live */
        use_vars(ptr_array, 20);
        
        /* Another inline assembly clobber */
        asm volatile("" : : : "eax", "ebx", "ecx", "edx", "memory");
        
        /* Inner loop with more operations */
        for (int j = 0; j < 3; j++) {
            v1 += j; v2 -= j; v3 *= (j + 1);
            f1 += j * 0.1f; f2 -= j * 0.1f;
            d1 += j * 0.01; d2 -= j * 0.01;
            
            /* Conditional that creates more CFG edges */
            if (j % 2 == 0) {
                v4 = v5 + v6;
                f3 = compute_factor(f3, f4);
            } else {
                v7 = v8 - v9;
                f4 = f5 * f6;
            }
        }
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                           (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
                           (int)d1 + (int)d2 + (int)d3;
    
    /* Force the checksum to be used */
    asm volatile("" : : "r"(checksum));
    
    printf("Checksum: %d\n", checksum);
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
    
    test_function(iterations, seed);
    
    return 0;
}
