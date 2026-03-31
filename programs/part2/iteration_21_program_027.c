/* test_mcf_coverage.c - Triggers MCF special node printing in GCC */

/* External function to force variable liveness across calls */
void __attribute__((noinline)) use_vars(int *a, float *b, double *c) {
    volatile int sink = *a;
    volatile float fsink = *b;
    volatile double dsink = *c;
    (void)sink; (void)fsink; (void)dsink;
}

/* Another external function to prevent inlining */
void __attribute__((noinline)) barrier(void) {
    asm volatile ("" : : : "memory");
}

/* The complex function that creates register pressure and complex CFG */
__attribute__((noinline, optimize("O2")))
int test_function(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed ^ 0x1234;
    int v4 = seed - 456;
    int v5 = seed + 789;
    int v6 = seed * 3;
    int v7 = seed / 2;
    int v8 = seed % 100;
    int v9 = seed | 0xFF00;
    int v10 = seed & 0x00FF;
    
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
    int v12 = v3 - v4;
    int v13 = v5 * v6;
    int v14 = v7 ^ v8;
    int v15 = v9 & v10;
    
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
    
    /* Additional variables to increase pressure */
    int v16 = v11 * v12;
    int v17 = v13 + v14;
    int v18 = v15 ^ seed;
    int v19 = v1 & v3;
    int v20 = v2 | v4;
    int v21 = v5 + v7;
    int v22 = v6 - v8;
    int v23 = v9 * v10;
    int v24 = v11 ^ v12;
    int v25 = v13 & v14;
    
    /* Force register clobbering with inline asm */
    asm volatile ("# Force clobber" 
                  : 
                  : 
                  : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    
    int result = 0;
    
    /* Complex loop with switch to create CFG edges */
    for (int i = 0; i < iterations; i++) {
        int switch_val = (seed + i) % 15;
        
        /* Large switch statement for complex CFG */
        switch (switch_val) {
            case 0:
                v1 = v2 + v3;
                f1 = f2 * f3;
                d1 = d2 - d3;
                break;
            case 1:
                v2 = v3 - v4;
                f2 = f3 / f4;
                d2 = d3 + d4;
                break;
            case 2:
                v3 = v4 * v5;
                f3 = f4 + f5;
                d3 = d4 * d5;
                break;
            case 3:
                v4 = v5 ^ v6;
                f4 = f5 - f1;
                d4 = d5 / d1;
                break;
            case 4:
                v5 = v6 & v7;
                f5 = f1 * f2;
                d5 = d1 + d2;
                break;
            case 5:
                v6 = v7 | v8;
                f6 = f2 / f3;
                d6 = d2 - d3;
                break;
            case 6:
                v7 = v8 + v9;
                f7 = f3 + f4;
                d7 = d3 * d4;
                break;
            case 7:
                v8 = v9 - v10;
                f8 = f4 - f5;
                d8 = d4 / d5;
                break;
            case 8:
                v9 = v10 * v11;
                f9 = f5 * f1;
                d9 = d5 + d1;
                break;
            case 9:
                v10 = v11 ^ v12;
                f10 = f1 / f2;
                d10 = d1 - d2;
                break;
            case 10:
                v11 = v12 & v13;
                f6 = f2 + f3;
                d6 = d2 * d3;
                break;
            case 11:
                v12 = v13 | v14;
                f7 = f3 - f4;
                d7 = d3 / d4;
                break;
            case 12:
                v13 = v14 + v15;
                f8 = f4 * f5;
                d8 = d4 + d5;
                break;
            case 13:
                v14 = v15 - v16;
                f9 = f5 / f1;
                d9 = d5 - d1;
                break;
            case 14:
                v15 = v16 * v17;
                f10 = f1 + f2;
                d10 = d1 * d2;
                break;
        }
        
        /* Force variables to be live across function call */
        if (i % 3 == 0) {
            use_vars(&v1, &f1, &d1);
        } else if (i % 3 == 1) {
            use_vars(&v10, &f5, &d5);
        } else {
            use_vars(&v20, &f10, &d10);
        }
        
        /* More register clobbering */
        asm volatile ("# More clobbering" : : : "eax", "ebx", "ecx", "memory");
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            v16 += v17 * j;
            f6 += f7 * j;
            d6 += d7 * j;
        }
        
        result += v1 + v5 + v10 + v15 + v20;
        result += (int)(f1 + f5 + f10);
        result += (int)(d1 + d5 + d10);
        
        /* Prevent loop unrolling */
        barrier();
    }
    
    /* Final computation using all variables */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += v21 + v22 + v23 + v24 + v25;
    result += (int)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10);
    result += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10);
    
    return result;
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
    
    int result = test_function(iterations, seed);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
