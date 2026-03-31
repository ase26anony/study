/* test_mcf_coverage.c
 * Compile with: gcc -O2 -m32 -fdump-rtl-ira -fdump-rtl-mcf -fno-omit-frame-pointer test_mcf_coverage.c -o test_mcf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
__attribute__((noinline)) void use_vars(int *a, float *b, double *c, char **d) {
    volatile int sink = *a + (int)*b + (int)*c + (int)(long)*d;
    (void)sink;
}

/* Complex test function with high register pressure */
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
    
    char *p1 = (char*)(long)(seed + 100);
    char *p2 = (char*)(long)(seed + 200);
    char *p3 = (char*)(long)(seed + 300);
    char *p4 = (char*)(long)(seed + 400);
    
    int result = 0;
    
    /* Complex loop with switch to create control flow */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        asm volatile (
            "# Clobber many registers\n"
            : : : "memory", "eax", "ebx", "ecx", "edx", 
                  "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Large switch statement for complex CFG */
        switch (i % 13) {
            case 0:
                v1 = v2 + v3; f1 = f2 + f3; d1 = d2 + d3;
                break;
            case 1:
                v4 = v5 * v6; f4 = f5 * f6; d4 = d5 * d6;
                break;
            case 2:
                v7 = v8 - v9; f7 = f8 - f9; d7 = d8 - d9;
                break;
            case 3:
                v10 = v11 ^ v12; p1 = p2; p2 = p3;
                break;
            case 4:
                v13 = v14 | v15; f3 = f4 * 2.0f; d3 = d4 / 2.0;
                break;
            case 5:
                v16 = v17 & v18; f5 = f6 - 1.0f; d5 = d6 + 1.0;
                break;
            case 6:
                v19 = v20 << 2; f7 = f8 / 3.0f; d7 = d8 * 3.0;
                break;
            case 7:
                v1 = v4 + v7; f2 = f5 * f8; d2 = d5 - d8;
                break;
            case 8:
                v2 = v5 ^ v8; f3 = f6 + f9; d3 = d6 / d9;
                break;
            case 9:
                v3 = v6 & v9; f4 = f7 * 4.0f; d4 = d7 + 4.0;
                break;
            case 10:
                v10 = v13 | v16; f8 = f1 - f4; d8 = d1 * d4;
                break;
            case 11:
                v11 = v14 ^ v17; f9 = f2 / f5; d9 = d2 - d5;
                break;
            case 12:
                v12 = v15 & v18; f1 = f3 + f6; d1 = d3 / d6;
                break;
        }
        
        /* Force variables live across function call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &p1);
            use_vars(&v2, &f2, &d2, &p2);
        }
        
        /* More register clobbering */
        asm volatile (
            "# Clobber more registers\n"
            : : : "memory", "ebp", "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* Complex computation mixing all variables */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
        result += (int)f5 + (int)f6 + (int)f7 + (int)f8;
        result += (int)d1 + (int)d2 + (int)d3 + (int)d4;
        result += (int)d5 + (int)d6 + (int)d7 + (int)d8;
        result += (int)(long)p1 + (int)(long)p2 + (int)(long)p3 + (int)(long)p4;
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            int temp = v1 + j;
            v1 = v2 + temp;
            v2 = v3 - temp;
            asm volatile ("# Inner loop clobber" : : : "memory");
        }
    }
    
    return result;
}

/* Another complex function to create more CFG edges */
__attribute__((noinline))
void helper_function(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        switch (arr[i] % 5) {
            case 0: arr[i] += 1; break;
            case 1: arr[i] *= 2; break;
            case 2: arr[i] -= 3; break;
            case 3: arr[i] ^= 0xFF; break;
            case 4: arr[i] |= 0xAA; break;
        }
    }
}

int main(int argc, char **argv) {
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
    
    /* Create array for additional control flow */
    int arr[50];
    for (int i = 0; i < 50; i++) {
        arr[i] = seed + i;
    }
    
    /* Call helper to build more complex CFG */
    helper_function(arr, 50);
    
    /* Main test function with high register pressure */
    int result = test_function(iterations, seed);
    
    /* Use array values to prevent optimization */
    for (int i = 0; i < 50; i++) {
        result += arr[i];
    }
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}
