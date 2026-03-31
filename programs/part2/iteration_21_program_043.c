/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variable liveness across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    volatile int sink = *a + (int)*b + (int)*c + (int)(long)*d;
    (void)sink;
}

/* Function with high register pressure and complex control flow */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed + 5, v6 = seed + 6, v7 = seed + 7, v8 = seed + 8;
    int v9 = seed + 9, v10 = seed + 10, v11 = seed + 11, v12 = seed + 12;
    int v13 = seed + 13, v14 = seed + 14, v15 = seed + 15, v16 = seed + 16;
    int v17 = seed + 17, v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    
    float f1 = seed * 1.1f, f2 = seed * 1.2f, f3 = seed * 1.3f, f4 = seed * 1.4f;
    float f5 = seed * 1.5f, f6 = seed * 1.6f, f7 = seed * 1.7f, f8 = seed * 1.8f;
    
    double d1 = seed * 2.1, d2 = seed * 2.2, d3 = seed * 2.3, d4 = seed * 2.4;
    double d5 = seed * 2.5, d6 = seed * 2.6, d7 = seed * 2.7, d8 = seed * 2.8;
    
    char* s1 = (char*)(long)(seed + 100);
    char* s2 = (char*)(long)(seed + 200);
    char* s3 = (char*)(long)(seed + 300);
    char* s4 = (char*)(long)(seed + 400);
    
    int result = 0;
    
    /* Complex loop with switch inside */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 15;
        
        /* Inline assembly to clobber registers - forces spills */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                                       "esi", "edi", "xmm0", "xmm1", 
                                       "xmm2", "xmm3", "xmm4", "xmm5");
        
        /* Large switch statement creating complex CFG */
        switch (mod) {
            case 0:
                v1 = v2 + v3; f1 = f2 - f3; d1 = d2 * d3; s1 = s2;
                break;
            case 1:
                v4 = v5 ^ v6; f4 = f5 / f6; d4 = d5 + d6; s4 = s3;
                break;
            case 2:
                v7 = v8 << 2; f7 = f8 * 2.0f; d7 = d8 / 2.0; result += v7;
                break;
            case 3:
                v9 = v10 | v11; f1 = f4 + f7; d1 = d4 - d7; s2 = s1;
                break;
            case 4:
                v12 = v13 & v14; f2 = f5 * f8; d2 = d5 * d8; result ^= v12;
                break;
            case 5:
                v15 = v16 >> 1; f3 = f6 / f1; d3 = d6 + d1; s3 = s4;
                break;
            case 6:
                v17 = v18 * v19; f4 = f7 - f2; d4 = d7 / d2; result |= v17;
                break;
            case 7:
                v20 = v1 + v4; f5 = f8 + f3; d5 = d8 - d3; s4 = s2;
                break;
            case 8:
                v2 = v3 - v5; f6 = f1 * f4; d6 = d1 * d4; result &= v2;
                break;
            case 9:
                v6 = v7 ^ v8; f7 = f2 / f5; d7 = d2 + d5; s1 = s3;
                break;
            case 10:
                v9 = v10 << 3; f8 = f3 - f6; d8 = d3 / d6; result += v9;
                break;
            case 11:
                v11 = v12 | v13; f1 = f4 * f7; d1 = d4 - d7; result ^= v11;
                break;
            case 12:
                v14 = v15 & v16; f2 = f5 / f8; d2 = d5 * d8; s2 = s4;
                break;
            case 13:
                v17 = v18 >> 2; f3 = f6 + f1; d3 = d6 / d1; result |= v17;
                break;
            case 14:
                v19 = v20 * v1; f4 = f7 - f2; d4 = d7 + d2; s3 = s1;
                break;
        }
        
        /* Force variables to be live across this call */
        use_vars(&v1, &f1, &d1, &s1);
        use_vars(&v10, &f5, &d5, &s2);
        use_vars(&v15, &f8, &d8, &s3);
        
        /* More register clobbering */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx");
    }
    
    /* Combine all variables into final result */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    result += (int)f5 + (int)f6 + (int)f7 + (int)f8;
    result += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    result += (int)d5 + (int)d6 + (int)d7 + (int)d8;
    result += (int)(long)s1 + (int)(long)s2 + (int)(long)s3 + (int)(long)s4;
    
    return result;
}

int main(int argc, char** argv) {
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
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
