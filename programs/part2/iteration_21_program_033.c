/* test_mcf_coverage.c - Test program to cover special node printing in GCC's MCF solver */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to ensure variables stay live across calls */
void __attribute__((noinline)) use_vars(int* ptr, float* fptr, double* dptr) {
    volatile int sink = *ptr;
    sink += (int)(*fptr);
    sink += (int)(*dptr);
    (void)sink;
}

/* Another external function to prevent optimization */
void __attribute__((noinline)) clobber_memory(void) {
    asm volatile ("" : : : "memory");
}

/* Main test function with high register pressure and complex CFG */
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
    int v9 = ~seed;
    int v10 = seed + 100;
    
    float f1 = seed * 0.1f;
    float f2 = seed * 0.2f;
    float f3 = seed * 0.3f;
    float f4 = seed * 0.4f;
    float f5 = seed * 0.5f;
    
    double d1 = seed * 0.01;
    double d2 = seed * 0.02;
    double d3 = seed * 0.03;
    double d4 = seed * 0.04;
    double d5 = seed * 0.05;
    
    int v11 = v1 + v2;
    int v12 = v3 * v4;
    int v13 = v5 ^ v6;
    int v14 = v7 | v8;
    int v15 = v9 & v10;
    
    float f6 = f1 + f2;
    float f7 = f3 * f4;
    float f8 = f5 / 2.0f;
    
    double d6 = d1 + d2;
    double d7 = d3 * d4;
    double d8 = d5 / 2.0;
    
    int v16 = (int)f1;
    int v17 = (int)f2;
    int v18 = (int)d1;
    int v19 = (int)d2;
    
    int v20 = v11 + v12;
    int v21 = v13 - v14;
    int v22 = v15 * v16;
    int v23 = v17 ^ v18;
    int v24 = v19 | v20;
    int v25 = v21 & v22;
    int v26 = v23 + v24;
    int v27 = v25 * v26;
    int v28 = v27 ^ seed;
    int v29 = v28 << 1;
    int v30 = v29 >> 2;
    
    /* Complex control flow with nested loops */
    int result = 0;
    for (int i = 0; i < iterations; i++) {
        /* Inner loop with variable bound */
        for (int j = 0; j < (i % 5) + 1; j++) {
            /* Large switch statement creating many basic blocks */
            switch ((i + j + seed) % 12) {
                case 0:
                    v1 += v2;
                    f1 += f2;
                    d1 += d2;
                    asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "memory");
                    break;
                case 1:
                    v3 *= v4;
                    f3 *= f4;
                    d3 *= d4;
                    asm volatile ("" : : : "esi", "edi", "ebp", "memory");
                    break;
                case 2:
                    v5 ^= v6;
                    f5 = f6 - f7;
                    d5 = d6 - d7;
                    break;
                case 3:
                    v7 |= v8;
                    f1 = f2 * f3;
                    d1 = d2 * d3;
                    /* Force spill around call */
                    use_vars(&v7, &f1, &d1);
                    break;
                case 4:
                    v9 &= v10;
                    f4 = f5 / f6;
                    d4 = d5 / d6;
                    break;
                case 5:
                    v11 = v12 + v13;
                    f7 = f8 + f1;
                    d7 = d8 + d1;
                    asm volatile ("" : : : "r8", "r9", "r10", "r11", "memory");
                    break;
                case 6:
                    v14 = v15 - v16;
                    f2 = f3 - f4;
                    d2 = d3 - d4;
                    break;
                case 7:
                    v17 = v18 * v19;
                    f5 = f6 * f7;
                    d5 = d6 * d7;
                    clobber_memory();
                    break;
                case 8:
                    v20 = v21 ^ v22;
                    f8 = f1 / f2;
                    d8 = d1 / d2;
                    break;
                case 9:
                    v23 = v24 | v25;
                    f3 = f4 + f5;
                    d3 = d4 + d5;
                    /* Another call forcing register saves */
                    use_vars(&v23, &f3, &d3);
                    break;
                case 10:
                    v26 = v27 & v28;
                    f6 = f7 * f8;
                    d6 = d7 * d8;
                    break;
                case 11:
                    v29 = v30 + v1;
                    f1 = f2 - f3;
                    d1 = d2 - d3;
                    asm volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "memory");
                    break;
            }
            
            /* More computations mixing all variables */
            v1 = (v1 + v3) ^ v5;
            v2 = (v2 * v4) | v6;
            v3 = (v3 - v7) & v8;
            v4 = (v4 ^ v9) + v10;
            
            f1 = f1 + f3 - f5;
            f2 = f2 * f4 / f6;
            f3 = f3 - f7 + f8;
            f4 = f4 / f1 * f2;
            
            d1 = d1 + d3 - d5;
            d2 = d2 * d4 / d6;
            d3 = d3 - d7 + d8;
            d4 = d4 / d1 * d2;
            
            /* Conditional that creates control flow edges */
            if ((j % 3) == 0) {
                v5 = v6 + v7;
                f5 = f6 * f7;
                d5 = d6 * d7;
            } else if ((j % 3) == 1) {
                v6 = v7 - v8;
                f6 = f7 / f8;
                d6 = d7 / d8;
            } else {
                v7 = v8 ^ v9;
                f7 = f8 + f1;
                d7 = d8 + d1;
            }
        }
        
        /* Update result with all variables to prevent elimination */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
        result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        /* Another switch for more CFG complexity */
        switch (i % 7) {
            case 0: result ^= 0x1111; break;
            case 1: result |= 0x2222; break;
            case 2: result &= 0x3333; break;
            case 3: result += 0x4444; break;
            case 4: result -= 0x5555; break;
            case 5: result *= 2; break;
            case 6: result /= 2; break;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 12345;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    int result = test_function(iterations, seed);
    
    printf("Result: %d\n", result);
    
    /* Use result to affect return code (prevents optimization) */
    return (result % 256) == 0 ? 0 : 1;
}
