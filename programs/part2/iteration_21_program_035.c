/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
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
    
    /* Array to pass pointers to external function */
    int* ptr_array[30];
    for (int i = 0; i < 30; i++) {
        ptr_array[i] = &v1 + i;
    }
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers many registers */
        asm volatile (
            "# Clobber many registers\n"
            : : : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi"
        );
        
        /* Large switch statement creating many basic blocks */
        switch ((seed + i) % 12) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 1:
                v2 = v3 * v4;
                v5 = v6 - v7;
                break;
            case 2:
                v3 = v4 / (v5 ? v5 : 1);
                f2 = f1 * 2.0f;
                break;
            case 3:
                v4 = v5 ^ v6;
                v7 = v8 | v9;
                break;
            case 4:
                v6 = v7 & v8;
                d2 = d1 * 3.14;
                break;
            case 5:
                v8 = v9 << 2;
                v10 = v11 >> 1;
                break;
            case 6:
                v12 = v13 + v14 * v15;
                f3 = f1 + f2;
                break;
            case 7:
                v16 = v17 - v18 + v19;
                d3 = d1 + d2;
                break;
            case 8:
                v20 = v21 * v22 - v23;
                p1 = (char*)&v24;
                break;
            case 9:
                v25 = v26 + v27 * v28;
                p2 = (char*)&v29;
                break;
            case 10:
                v30 = v1 + v2 + v3 + v4;
                f1 = f2 = f3 = v30 * 0.5f;
                break;
            case 11:
                v1 = v30;
                v2 = v29;
                v3 = v28;
                d1 = d2 = d3 = v1 * 0.01;
                break;
        }
        
        /* Call external function making variables live across call */
        use_vars(ptr_array, 30);
        
        /* Another inline assembly clobber */
        asm volatile (
            "# Clobber more registers\n"
            : : : "memory", "eax", "ebx", "ecx", "edx"
        );
        
        /* Nested loop for additional CFG complexity */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 5 == 0) {
                v1 += v2;
                v3 -= v4;
            } else if ((i + j) % 5 == 1) {
                v5 *= v6;
                v7 /= (v8 ? v8 : 1);
            }
            
            /* Small switch inside nested loop */
            switch ((i * j) % 4) {
                case 0: v9++; break;
                case 1: v10--; break;
                case 2: v11 ^= v12; break;
                case 3: v13 |= v14; break;
            }
        }
    }
    
    /* Compute checksum from all variables to prevent elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                   (int)f1 + (int)f2 + (int)f3 + (int)d1 + (int)d2 + (int)d3 +
                   (int)(long)p1 + (int)(long)p2;
    
    return checksum;
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
