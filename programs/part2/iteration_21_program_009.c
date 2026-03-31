/* test_mcf_coverage.c - Trigger MCF solver special node printing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to force variables live across calls */
void __attribute__((noinline)) use_vars(int* a, float* b, double* c, char** d) {
    volatile int sink = *a + (int)*b + (int)*c + (int)(long)*d;
    (void)sink;
}

/* Complex function with high register pressure and control flow */
__attribute__((noinline, optimize("O2")))
unsigned long test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed ^ 0x1234;
    int v4 = seed - 100;
    int v5 = seed + 200;
    int v6 = seed * 3;
    int v7 = seed / 2;
    int v8 = seed % 17;
    int v9 = seed | 0xFF00;
    int v10 = seed & 0x00FF;
    
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
    
    char* s1 = (char*)(long)(seed + 1000);
    char* s2 = (char*)(long)(seed + 2000);
    char* s3 = (char*)(long)(seed + 3000);
    char* s4 = (char*)(long)(seed + 4000);
    
    /* Additional variables for more pressure */
    int v11 = v1 ^ v2;
    int v12 = v3 + v4;
    int v13 = v5 * v6;
    int v14 = v7 - v8;
    int v15 = v9 & v10;
    int v16 = v1 | v3;
    int v17 = v2 ^ v4;
    int v18 = v5 + v7;
    int v19 = v6 * v8;
    int v20 = v9 - v10;
    
    float f6 = f1 + f2;
    float f7 = f3 - f4;
    float f8 = f5 * f1;
    float f9 = f2 / f3;
    float f10 = f4 + f5;
    
    double d6 = d1 + d2;
    double d7 = d3 - d4;
    double d8 = d5 * d1;
    double d9 = d2 / d3;
    double d10 = d4 + d5;
    
    unsigned long checksum = 0;
    
    /* Complex loop with switch to create control flow */
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 20;
        
        /* Inline assembly to clobber registers */
        __asm__ volatile (
            "# Force register clobbering\n"
            : : : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi"
        );
        
        /* Large switch statement for complex CFG */
        switch (mod) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * 1.5f;
                d1 = d2 / 2.0;
                break;
            case 1:
                v2 = v3 ^ v4;
                f2 = f3 - 1.0f;
                d2 = d3 * 1.1;
                break;
            case 2:
                v3 = v4 | v5;
                f3 = f4 + 2.0f;
                d3 = d4 - 0.5;
                break;
            case 3:
                v4 = v5 & v6;
                f4 = f5 / 3.0f;
                d4 = d5 * 2.0;
                break;
            case 4:
                v5 = v6 + v7;
                f5 = f1 * 0.5f;
                d5 = d1 / 1.5;
                break;
            case 5:
                v6 = v7 ^ v8;
                f6 = f2 - f1;
                d6 = d2 + d1;
                break;
            case 6:
                v7 = v8 | v9;
                f7 = f3 * f2;
                d7 = d3 - d2;
                break;
            case 7:
                v8 = v9 & v10;
                f8 = f4 / f3;
                d8 = d4 * d3;
                break;
            case 8:
                v9 = v10 + v1;
                f9 = f5 + f4;
                d9 = d5 / d4;
                break;
            case 9:
                v10 = v1 ^ v2;
                f10 = f1 - f5;
                d10 = d1 * d5;
                break;
            case 10:
                v11 = v12 * v13;
                f6 = f7 + 1.1f;
                d6 = d7 * 0.9;
                break;
            case 11:
                v12 = v13 - v14;
                f7 = f8 / 2.2f;
                d7 = d8 + 1.2;
                break;
            case 12:
                v13 = v14 | v15;
                f8 = f9 * 3.3f;
                d8 = d9 - 0.3;
                break;
            case 13:
                v14 = v15 & v16;
                f9 = f10 + 4.4f;
                d9 = d10 * 1.4;
                break;
            case 14:
                v15 = v16 ^ v17;
                f10 = f6 / 5.5f;
                d10 = d6 + 2.5;
                break;
            case 15:
                v16 = v17 + v18;
                f1 = f6 * f7;
                d1 = d6 - d7;
                break;
            case 16:
                v17 = v18 | v19;
                f2 = f7 / f8;
                d2 = d7 * d8;
                break;
            case 17:
                v18 = v19 & v20;
                f3 = f8 + f9;
                d3 = d8 / d9;
                break;
            case 18:
                v19 = v20 ^ v11;
                f4 = f9 - f10;
                d4 = d9 + d10;
                break;
            case 19:
                v20 = v11 + v12;
                f5 = f10 * f6;
                d5 = d10 - d6;
                break;
        }
        
        /* Force variables live across call */
        if (i % 7 == 0) {
            use_vars(&v1, &f1, &d1, &s1);
        }
        if (i % 11 == 0) {
            use_vars(&v10, &f5, &d5, &s4);
        }
        
        /* More inline assembly */
        __asm__ volatile (
            "# More register pressure\n"
            : : : "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        /* Update checksum to prevent elimination */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        checksum += (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3;
        checksum += (unsigned long)d1 + (unsigned long)d2;
        checksum += (unsigned long)s1 + (unsigned long)s2;
    }
    
    /* Final mixing */
    checksum ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    checksum += (unsigned long)f6 + (unsigned long)f7 + (unsigned long)f8;
    checksum += (unsigned long)f9 + (unsigned long)f10;
    checksum += (unsigned long)d6 + (unsigned long)d7 + (unsigned long)d8;
    checksum += (unsigned long)d9 + (unsigned long)d10;
    checksum += (unsigned long)s3 + (unsigned long)s4;
    
    return checksum;
}

int main(int argc, char** argv) {
    int iterations = 100;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    unsigned long result = test_function(iterations, seed);
    printf("Result: %lu\n", result);
    
    return 0;
}
