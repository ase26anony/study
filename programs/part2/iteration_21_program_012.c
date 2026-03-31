/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    volatile int sink = *a + (int)*b + (int)*c + (int)(long)*d;
    (void)sink;
}

/* Complex test function with high register pressure */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed + 2;
    int v3 = seed + 3;
    int v4 = seed + 4;
    int v5 = seed + 5;
    int v6 = seed + 6;
    int v7 = seed + 7;
    int v8 = seed + 8;
    int v9 = seed + 9;
    int v10 = seed + 10;
    int v11 = seed + 11;
    int v12 = seed + 12;
    int v13 = seed + 13;
    int v14 = seed + 14;
    int v15 = seed + 15;
    int v16 = seed + 16;
    int v17 = seed + 17;
    int v18 = seed + 18;
    int v19 = seed + 19;
    int v20 = seed + 20;
    
    float f1 = seed * 1.1f;
    float f2 = seed * 1.2f;
    float f3 = seed * 1.3f;
    float f4 = seed * 1.4f;
    float f5 = seed * 1.5f;
    float f6 = seed * 1.6f;
    float f7 = seed * 1.7f;
    float f8 = seed * 1.8f;
    
    double d1 = seed * 2.1;
    double d2 = seed * 2.2;
    double d3 = seed * 2.3;
    double d4 = seed * 2.4;
    double d5 = seed * 2.5;
    
    char* s1 = (char*)(long)(seed + 100);
    char* s2 = (char*)(long)(seed + 200);
    char* s3 = (char*)(long)(seed + 300);
    char* s4 = (char*)(long)(seed + 400);
    
    int result = 0;
    
    /* Complex loop with switch to create control flow */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", 
                                           "esi", "edi", "xmm0", "xmm1", 
                                           "xmm2", "xmm3", "xmm4", "xmm5");
        
        /* Large switch statement */
        switch ((i + seed) % 12) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 + f3;
                d1 = d2 + d3;
                break;
            case 1:
                v4 = v5 * v6;
                f4 = f5 * f6;
                d4 = d5 * 1.5;
                break;
            case 2:
                v7 = v8 - v9;
                f7 = f8 - f9;
                d2 = d3 - d4;
                break;
            case 3:
                v10 = v11 / (v12 ? v12 : 1);
                f3 = f4 / (f5 ? f5 : 1.0f);
                d5 = d1 / (d2 ? d2 : 1.0);
                break;
            case 4:
                v13 = v14 & v15;
                v16 = v17 | v18;
                break;
            case 5:
                v19 = v20 ^ v1;
                f2 = f1 * f3;
                break;
            case 6:
                d3 = d4 + d5;
                f6 = f7 + f8;
                v2 = v3 + v4;
                break;
            case 7:
                v5 = v6 * v7;
                f5 = f6 * f7;
                d1 = d2 * d3;
                break;
            case 8:
                v8 = v9 - v10;
                f8 = f1 - f2;
                d4 = d5 - d1;
                break;
            case 9:
                v11 = v12 / (v13 ? v13 : 1);
                f4 = f3 / (f2 ? f2 : 1.0f);
                break;
            case 10:
                v14 = v15 & v16;
                v17 = v18 | v19;
                f7 = f6 * f5;
                break;
            case 11:
                v20 = v1 ^ v2;
                d5 = d4 + d3;
                f1 = f8 + f7;
                break;
        }
        
        /* Force variables live across function call */
        if (i % 3 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        } else if (i % 3 == 1) {
            use_vars(&v10, &f4, &d3, &s2);
        } else {
            use_vars(&v20, &f8, &d5, &s3);
        }
        
        /* More inline assembly */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx");
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 2; j++) {
            int temp = v1 + v2 + v3;
            v1 = v2 + temp;
            v2 = v3 + temp;
            v3 = temp;
            
            asm volatile ("" : : : "memory");
        }
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  (int)f1 + (int)f2 + (int)f3 + (int)f4 + 
                  (int)f5 + (int)f6 + (int)f7 + (int)f8 +
                  (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
                  (int)(long)s1 + (int)(long)s2 + (int)(long)s3 + (int)(long)s4;
    }
    
    return result;
}

int main(int argc, char** argv) {
    int iterations = 10;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    int result = test_function(iterations, seed);
    
    printf("Result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Unexpected zero result\n");
        return 1;
    }
    
    return 0;
}
