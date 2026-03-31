/* test_mcf_coverage.c - Trigger MCF special node printing in GCC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c) {
    volatile int sink = *a + (int)*b + (int)*c;
    (void)sink;
}

/* Complex function with high register pressure and control flow */
__attribute__((noinline, optimize("O2")))
unsigned long test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed - 4;
    int v5 = seed ^ 0x55;
    int v6 = seed | 0xAA;
    int v7 = seed & 0xF0;
    int v8 = seed << 2;
    int v9 = seed >> 1;
    int v10 = ~seed;
    
    float f1 = seed * 1.1f;
    float f2 = seed * 2.2f;
    float f3 = seed * 3.3f;
    float f4 = seed * 4.4f;
    float f5 = seed * 5.5f;
    
    double d1 = seed * 1.11;
    double d2 = seed * 2.22;
    double d3 = seed * 3.33;
    double d4 = seed * 4.44;
    double d5 = seed * 5.55;
    
    char* p1 = (char*)&v1;
    char* p2 = (char*)&v2;
    char* p3 = (char*)&v3;
    char* p4 = (char*)&v4;
    char* p5 = (char*)&v5;
    
    /* Additional variables for more pressure */
    int v11 = v1 + v2;
    int v12 = v3 + v4;
    int v13 = v5 + v6;
    int v14 = v7 + v8;
    int v15 = v9 + v10;
    int v16 = v1 * v3;
    int v17 = v2 * v4;
    int v18 = v5 * v7;
    int v19 = v6 * v8;
    int v20 = v9 * v10;
    
    float f6 = f1 + f2;
    float f7 = f3 + f4;
    float f8 = f5 * 2.0f;
    float f9 = f1 * f2;
    float f10 = f3 * f4;
    
    double d6 = d1 + d2;
    double d7 = d3 + d4;
    double d8 = d5 * 2.0;
    double d9 = d1 * d2;
    double d10 = d3 * d4;
    
    unsigned long checksum = 0;
    
    /* Complex loop with switch statement */
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers */
        __asm__ volatile (
            ""
            : 
            : 
            : "memory", "eax", "ebx", "ecx", "edx", 
              "esi", "edi", "xmm0", "xmm1", "xmm2", "xmm3"
        );
        
        /* Large switch to create complex CFG */
        switch ((i + seed) % 12) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * 3.14f;
                d1 = d2 / 2.71828;
                break;
            case 1:
                v2 = v3 ^ v4;
                f2 = f3 + 1.0f;
                d2 = d3 - 1.0;
                break;
            case 2:
                v3 = v4 | v5;
                f3 = f4 * f5;
                d3 = d4 * d5;
                break;
            case 3:
                v4 = v5 & v6;
                f4 = f5 / 2.0f;
                d4 = d5 + 3.14159;
                break;
            case 4:
                v5 = v6 << 1;
                f5 = f1 - f2;
                d5 = d1 - d2;
                break;
            case 5:
                v6 = v7 >> 2;
                f1 = f3 * f4;
                d1 = d3 / d4;
                break;
            case 6:
                v7 = v8 * v9;
                f2 = f4 + f5;
                d2 = d4 * 2.0;
                break;
            case 7:
                v8 = v9 / (v10 + 1);
                f3 = f5 - f1;
                d3 = d5 + d1;
                break;
            case 8:
                v9 = v10 ^ v1;
                f4 = f1 * 3.0f;
                d4 = d2 / 1.5;
                break;
            case 9:
                v10 = v1 | v2;
                f5 = f2 / 4.0f;
                d5 = d3 * 1.1;
                break;
            case 10:
                v11 = v12 + v13;
                f6 = f7 * f8;
                d6 = d7 + d8;
                break;
            case 11:
                v12 = v13 ^ v14;
                f7 = f8 + f9;
                d7 = d8 * d9;
                break;
        }
        
        /* Force variables live across function call */
        use_vars(&v1, &f1, &d1);
        use_vars(&v2, &f2, &d2);
        
        /* More computations to keep variables alive */
        v13 = v1 + v2 + v3;
        v14 = v4 * v5 * v6;
        v15 = v7 ^ v8 ^ v9;
        f8 = f1 + f3 + f5;
        f9 = f2 * f4 * f6;
        d8 = d1 + d3 + d5;
        d9 = d2 * d4 * d6;
        
        /* Update checksum to prevent elimination */
        checksum += v1 + v2 + v3 + v4 + v5;
        checksum += (unsigned long)(f1 + f2 + f3);
        checksum += (unsigned long)(d1 + d2 + d3);
    }
    
    /* Final mixing of all variables */
    checksum ^= v6 ^ v7 ^ v8 ^ v9 ^ v10;
    checksum ^= v11 ^ v12 ^ v13 ^ v14 ^ v15;
    checksum ^= (unsigned long)(f4 + f5 + f6 + f7 + f8 + f9 + f10);
    checksum ^= (unsigned long)(d4 + d5 + d6 + d7 + d8 + d9 + d10);
    
    return checksum;
}

int main(int argc, char** argv) {
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
    
    unsigned long result = test_function(iterations, seed);
    printf("Result: %lu\n", result);
    
    return 0;
}
