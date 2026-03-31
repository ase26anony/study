/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
__attribute__((noinline)) 
void use_vars(int* ptrs[], int count) {
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
    int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4, v4 = seed + 5;
    int v5 = seed + 6, v6 = seed + 7, v7 = seed + 8, v8 = seed + 9, v9 = seed + 10;
    int v10 = seed + 11, v11 = seed + 12, v12 = seed + 13, v13 = seed + 14, v14 = seed + 15;
    int v15 = seed + 16, v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    int v20 = seed + 21, v21 = seed + 22, v22 = seed + 23, v23 = seed + 24, v24 = seed + 25;
    int v25 = seed + 26, v26 = seed + 27, v27 = seed + 28, v28 = seed + 29, v29 = seed + 30;
    
    /* Mix in some float/double variables */
    float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    
    /* Pointer variables for additional complexity */
    char* p0 = (char*)&v0;
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    
    /* Array of pointers to force live ranges across calls */
    int* ptr_array[30];
    for (int i = 0; i < 30; i++) {
        ptr_array[i] = &v0 + i;
    }
    
    /* Complex loop with nested control flow */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that clobbers many registers */
        asm volatile (
            "# Force register clobbering\n"
            : : : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi"
        );
        
        /* Large switch statement creating many basic blocks */
        switch ((i + seed) % 12) {
            case 0:
                v0 = v1 + v2;
                f0 = f1 + f2;
                d0 = d1 + d2;
                break;
            case 1:
                v1 = v3 * v4;
                f1 = f3 * f0;
                d1 = d3 * d0;
                break;
            case 2:
                v2 = v5 - v6;
                f2 = f0 - f1;
                d2 = d0 - d1;
                break;
            case 3:
                v3 = v7 / (v8 ? v8 : 1);
                f3 = f1 / (f2 ? f2 : 1.0f);
                d3 = d1 / (d2 ? d2 : 1.0);
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
                v16 = v0 ^ v1;
                v17 = v2 << 3;
                break;
            case 11:
                v18 = v3 >> 2;
                v19 = ~v4;
                break;
        }
        
        /* Nested if-else chain */
        if (i % 3 == 0) {
            if (v0 > v1) {
                v20 = v0 - v1;
            } else if (v0 < v1) {
                v21 = v1 - v0;
            } else {
                v22 = v0 + v1;
            }
        } else if (i % 3 == 1) {
            for (int j = 0; j < 3; j++) {
                v23 += j;
                v24 -= j;
            }
        } else {
            int temp = 0;
            while (temp < 2) {
                v25 += temp;
                v26 -= temp;
                temp++;
            }
        }
        
        /* Force variables live across function call */
        if (i % 5 == 0) {
            use_vars(ptr_array, 30);
            
            /* Another inline assembly with different clobbers */
            asm volatile (
                "# More register pressure\n"
                : : : "memory", "eax", "ebx", "ecx"
            );
        }
        
        /* Mix float/double operations */
        f0 = f1 + f2 * f3;
        f1 = f0 - f3 / (f2 ? f2 : 1.0f);
        d0 = d1 + d2 * d3;
        d1 = d0 - d3 / (d2 ? d2 : 1.0);
        
        /* Pointer arithmetic */
        p0 += (i % 4);
        p1 -= (i % 3);
        p2 = p0 + (p1 - p0);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                   v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                   v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                   (int)f0 + (int)f1 + (int)f2 + (int)f3 +
                   (int)d0 + (int)d1 + (int)d2 + (int)d3 +
                   (int)(p0 - (char*)&v0) + (int)(p1 - (char*)&v1);
    
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
