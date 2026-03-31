/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variable liveness across calls */
void __attribute__((noinline)) use_vars(int* ptrs[], int count) {
    volatile int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += *ptrs[i];
    }
    (void)sum;
}

/* Complex function with high register pressure */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed - 5;
    int v4 = seed ^ 0x1234;
    int v5 = seed | 0xABCD;
    int v6 = seed & 0xF0F0;
    int v7 = seed << 3;
    int v8 = seed >> 2;
    int v9 = seed + 100;
    int v10 = seed - 200;
    
    float f1 = seed * 0.1f;
    float f2 = seed * 0.2f;
    float f3 = seed * 0.3f;
    float f4 = seed * 0.4f;
    float f5 = seed * 0.5f;
    
    double d1 = seed * 0.01;
    double d2 = seed * 0.02;
    double d3 = seed * 0.03;
    double d4 = seed * 0.04;
    
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    char* p4 = (char*)&v4;
    
    int a1 = seed + 10;
    int a2 = seed + 20;
    int a3 = seed + 30;
    int a4 = seed + 40;
    int a5 = seed + 50;
    int a6 = seed + 60;
    int a7 = seed + 70;
    int a8 = seed + 80;
    int a9 = seed + 90;
    int a10 = seed + 100;
    
    int b1 = seed * 3;
    int b2 = seed * 4;
    int b3 = seed * 5;
    int b4 = seed * 6;
    int b5 = seed * 7;
    
    /* Array of pointers to force spilling */
    int* ptrs[20];
    ptrs[0] = &v1; ptrs[1] = &v2; ptrs[2] = &v3; ptrs[3] = &v4; ptrs[4] = &v5;
    ptrs[5] = &v6; ptrs[6] = &v7; ptrs[7] = &v8; ptrs[8] = &v9; ptrs[9] = &v10;
    ptrs[10] = &a1; ptrs[11] = &a2; ptrs[12] = &a3; ptrs[13] = &a4; ptrs[14] = &a5;
    ptrs[15] = &a6; ptrs[16] = &a7; ptrs[17] = &a8; ptrs[18] = &a9; ptrs[19] = &a10;
    
    /* Complex control flow with loops and switch */
    int result = 0;
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile (
            ""
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Large switch statement creating many basic blocks */
        switch (i % 13) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 1:
                v2 = v3 * v4;
                f2 = f3 * f4;
                d2 = d3 * d4;
                break;
            case 2:
                v3 = v4 ^ v5;
                f3 = f4 - f5;
                d3 = d4 - d1;
                break;
            case 3:
                v4 = v5 | v6;
                f4 = f5 / 2.0f;
                d4 = d1 / 2.0;
                break;
            case 4:
                v5 = v6 & v7;
                f5 = f1 * 3.0f;
                d1 = d2 * 3.0;
                break;
            case 5:
                v6 = v7 << 2;
                f1 = f2 - f3;
                d2 = d3 - d4;
                break;
            case 6:
                v7 = v8 >> 1;
                f2 = f3 + f4;
                d3 = d4 + d1;
                break;
            case 7:
                v8 = v9 + v10;
                f3 = f4 * f5;
                d4 = d1 * d2;
                break;
            case 8:
                v9 = v10 * a1;
                f4 = f5 / 3.0f;
                d1 = d2 / 3.0;
                break;
            case 9:
                v10 = a1 ^ a2;
                f5 = f1 + f2;
                d2 = d3 + d4;
                break;
            case 10:
                a1 = a2 + a3;
                f1 = f3 - f4;
                d3 = d1 - d2;
                break;
            case 11:
                a2 = a3 * a4;
                f2 = f4 * f5;
                d4 = d2 * d3;
                break;
            case 12:
                a3 = a4 | a5;
                f3 = f5 / 4.0f;
                d1 = d3 / 4.0;
                break;
        }
        
        /* Force variables to be live across function call */
        if (i % 7 == 0) {
            use_vars(ptrs, 20);
            
            /* More register clobbering */
            asm volatile (
                ""
                : 
                : 
                : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
            );
        }
        
        /* Complex computation mixing all variables */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += (int)(f1 + f2 + f3 + f4 + f5);
        result += (int)(d1 + d2 + d3 + d4);
        result += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
        result += b1 + b2 + b3 + b4 + b5;
        
        /* Nested loop for additional control flow complexity */
        for (int j = 0; j < 3; j++) {
            int temp = result;
            switch (j) {
                case 0: temp ^= v1; break;
                case 1: temp |= v2; break;
                case 2: temp &= v3; break;
            }
            result = temp;
        }
    }
    
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
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
