/* test-mcf-coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test-mcf-coverage.c -o test-mcf-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
__attribute__((noinline)) void use_vars(int* a, float* b, double* c, char** d) {
    volatile int sink = *a + (int)*b + (int)*c + (int)(long)*d;
    (void)sink;
}

/* Complex test function with high register pressure */
__attribute__((noinline, optimize("O2"))) 
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed - 5;
    int v4 = seed ^ 0x1234;
    int v5 = seed << 3;
    int v6 = seed >> 2;
    int v7 = seed | 0xFF;
    int v8 = seed & 0x7F;
    int v9 = seed + 100;
    int v10 = seed - 50;
    
    float f1 = (float)seed * 1.1f;
    float f2 = (float)seed * 2.2f;
    float f3 = (float)seed * 3.3f;
    float f4 = (float)seed * 4.4f;
    float f5 = (float)seed * 5.5f;
    
    double d1 = (double)seed * 1.11;
    double d2 = (double)seed * 2.22;
    double d3 = (double)seed * 3.33;
    double d4 = (double)seed * 4.44;
    double d5 = (double)seed * 5.55;
    
    char* s1 = (char*)(long)(seed + 1000);
    char* s2 = (char*)(long)(seed + 2000);
    char* s3 = (char*)(long)(seed + 3000);
    char* s4 = (char*)(long)(seed + 4000);
    char* s5 = (char*)(long)(seed + 5000);
    
    /* Additional variables for more pressure */
    int v11 = v1 + v2;
    int v12 = v3 * v4;
    int v13 = v5 ^ v6;
    int v14 = v7 & v8;
    int v15 = v9 - v10;
    int v16 = v1 * v3;
    int v17 = v2 + v4;
    int v18 = v5 - v6;
    int v19 = v7 ^ v8;
    int v20 = v9 & v10;
    
    float f6 = f1 + f2;
    float f7 = f3 * f4;
    float f8 = f5 - f1;
    float f9 = f2 * f3;
    float f10 = f4 / 2.0f;
    
    double d6 = d1 + d2;
    double d7 = d3 * d4;
    double d8 = d5 - d1;
    double d9 = d2 * d3;
    double d10 = d4 / 2.0;
    
    int result = 0;
    
    /* Complex loop with switch to create control flow */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile (
            ""
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Large switch statement */
        switch (i % 13) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * 1.5f;
                d1 = d2 + 1.0;
                result += v1;
                break;
            case 1:
                v2 = v3 * v4;
                f2 = f3 / 2.0f;
                d2 = d3 * 2.0;
                result += v2;
                break;
            case 2:
                v3 = v4 ^ v5;
                f3 = f4 + 3.0f;
                d3 = d4 - 1.0;
                result += v3;
                break;
            case 3:
                v4 = v5 & v6;
                f4 = f5 * 0.5f;
                d4 = d5 / 2.0;
                result += v4;
                break;
            case 4:
                v5 = v6 << 1;
                f5 = f1 - f2;
                d5 = d1 + d2;
                result += v5;
                break;
            case 5:
                v6 = v7 >> 2;
                f6 = f3 * f4;
                d6 = d3 - d4;
                result += v6;
                break;
            case 6:
                v7 = v8 | v9;
                f7 = f5 / f1;
                d7 = d5 * d1;
                result += v7;
                break;
            case 7:
                v8 = v9 ^ v10;
                f8 = f2 + f3;
                d8 = d2 + d3;
                result += v8;
                break;
            case 8:
                v9 = v10 + v1;
                f9 = f4 - f5;
                d9 = d4 / d5;
                result += v9;
                break;
            case 9:
                v10 = v1 * v2;
                f10 = f1 * 3.0f;
                d10 = d1 * 4.0;
                result += v10;
                break;
            case 10:
                v11 = v12 + v13;
                f6 = f7 * 0.8f;
                d6 = d7 + 0.5;
                result += v11;
                break;
            case 11:
                v12 = v13 - v14;
                f7 = f8 / 1.2f;
                d7 = d8 - 0.3;
                result += v12;
                break;
            case 12:
                v13 = v14 ^ v15;
                f8 = f9 + f10;
                d8 = d9 * d10;
                result += v13;
                break;
        }
        
        /* Force variables live across function call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        } else if (i % 7 == 1) {
            use_vars(&v2, &f2, &d2, &s2);
        } else if (i % 7 == 2) {
            use_vars(&v3, &f3, &d3, &s3);
        } else if (i % 7 == 3) {
            use_vars(&v4, &f4, &d4, &s4);
        } else if (i % 7 == 4) {
            use_vars(&v5, &f5, &d5, &s5);
        } else if (i % 7 == 5) {
            use_vars(&v6, &f6, &d6, &s1);
        } else {
            use_vars(&v7, &f7, &d7, &s2);
        }
        
        /* More computations to keep variables alive */
        v14 = v15 + v16;
        v15 = v16 * v17;
        v16 = v17 ^ v18;
        v17 = v18 & v19;
        v18 = v19 | v20;
        v19 = v20 + v11;
        v20 = v11 - v12;
        
        f9 = f10 * 1.1f;
        f10 = f6 + f7;
        
        d9 = d10 / 1.5;
        d10 = d6 + d7;
    }
    
    /* Combine all variables into final result */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    result += (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
    result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    result += (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10;
    result += (int)(long)s1 + (int)(long)s2 + (int)(long)s3 + 
              (int)(long)s4 + (int)(long)s5;
    
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
    
    /* Use result to prevent optimization */
    if (result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
