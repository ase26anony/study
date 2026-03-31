/* mcf_trigger.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to preserve all computations */
static volatile int sink;

/* Complex test function with high register pressure */
static int __attribute__((noinline)) 
complex_mcf_test(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    float f0, f1, f2, f3, f4;
    void *p0, *p1, *p2;
    int result = seed;
    
    /* Initialize variables with complex dependencies */
    v0 = seed * 1;
    v1 = seed * 2;
    v2 = seed * 3;
    v3 = seed * 4;
    v4 = seed * 5;
    v5 = seed * 6;
    v6 = seed * 7;
    v7 = seed * 8;
    v8 = seed * 9;
    v9 = seed * 10;
    v10 = seed * 11;
    v11 = seed * 12;
    v12 = seed * 13;
    v13 = seed * 14;
    v14 = seed * 15;
    v15 = seed * 16;
    v16 = seed * 17;
    v17 = seed * 18;
    v18 = seed * 19;
    v19 = seed * 20;
    v20 = seed * 21;
    v21 = seed * 22;
    v22 = seed * 23;
    v23 = seed * 24;
    v24 = seed * 25;
    v25 = seed * 26;
    v26 = seed * 27;
    v27 = seed * 28;
    v28 = seed * 29;
    v29 = seed * 30;
    
    f0 = seed * 1.1f;
    f1 = seed * 2.2f;
    f2 = seed * 3.3f;
    f3 = seed * 4.4f;
    f4 = seed * 5.5f;
    
    p0 = &v0;
    p1 = &v10;
    p2 = &v20;
    
    /* Complex loop with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Create data dependencies across variables */
        v0 = v1 + v2;
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        
        /* Inline assembly to force register constraints */
        asm volatile (
            "# Force register pressure\n\t"
            : 
            : "r" (v0), "r" (v1), "r" (v2), "r" (v3), "r" (v4)
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Complex conditional chain creating many basic blocks */
        if (i % 3 == 0) {
            v5 = v6 * v7;
            v6 = v7 / (v8 + 1);
            f0 = f1 * 1.5f;
            
            /* Nested conditionals */
            if (v5 > 100) {
                v7 = v8 << 2;
                v8 = v9 >> 1;
                
                /* More inline assembly */
                asm volatile (
                    "# More register pressure\n\t"
                    : 
                    : "r" (v5), "r" (v6), "r" (v7), "r" (v8)
                    : "eax", "ebx", "ecx", "memory"
                );
            } else {
                v7 = v8 * 3;
                v8 = v9 + 7;
                f1 = f2 + 2.0f;
            }
        } else if (i % 3 == 1) {
            v9 = v10 ^ v11;
            v10 = v11 | v12;
            v11 = v12 & v13;
            f2 = f3 - 1.0f;
            
            /* Another level of nesting */
            if (v9 < 50) {
                v12 = v13 + v14;
                v13 = v14 - v15;
                
                /* Force spill/reload */
                asm volatile (
                    "# Force spills\n\t"
                    : "+r" (v12), "+r" (v13)
                    : 
                    : "memory"
                );
            }
        } else {
            v14 = v15 * v16;
            v15 = v16 + v17;
            v16 = v17 - v18;
            f3 = f4 * 0.5f;
        }
        
        /* Switch statement creating multiple control flow paths */
        switch (i % 7) {
            case 0:
                v17 = v18 + 1;
                v18 = v19 - 1;
                break;
            case 1:
                v19 = v20 * 2;
                v20 = v21 / 2;
                break;
            case 2:
                v21 = v22 << 1;
                v22 = v23 >> 1;
                break;
            case 3:
                v23 = v24 ^ 0xFF;
                v24 = v25 | 0x0F;
                break;
            case 4:
                v25 = v26 + v27;
                v26 = v27 - v28;
                break;
            case 5:
                v27 = v28 * v29;
                v28 = v29 + i;
                break;
            case 6:
                v29 = v0 + i;
                v0 = v1 - i;
                break;
        }
        
        /* Floating point operations mixing register classes */
        f4 = f0 + f1 + f2 + f3;
        
        /* Pointer arithmetic creating address computations */
        if (p0) {
            *(int*)p0 = v0;
            p0 = (char*)p0 + 1;
        }
        
        if (p1) {
            *(int*)p1 = v10;
            p1 = (char*)p1 - 1;
        }
        
        /* Loop with early exit creating exit blocks */
        if (i > iterations / 2) {
            if (result > 1000000) {
                /* Early return path */
                result = result / 2;
                goto early_exit;
            }
        }
        
        /* Cross basic block value usage */
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        
        /* Another inline assembly block */
        asm volatile (
            "# Final register pressure\n\t"
            : "+r" (v1), "+r" (v2), "+r" (v3)
            : 
            : "eax", "ebx", "ecx", "edx", "memory"
        );
    }
    
early_exit:
    
    /* Complex computation using all variables */
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
             v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
             v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
             (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    
    /* Use sink to prevent dead code elimination */
    sink = result;
    
    return result;
}

/* Another complex function to ensure MCF runs multiple times */
static int __attribute__((noinline))
another_complex_function(int x, int y) {
    int a = x, b = y, c, d, e, f, g, h, i, j;
    int k, l, m, n, o, p, q, r, s, t;
    
    /* Deep conditional nesting */
    if (x > 0) {
        a = x * 2;
        b = y * 3;
        
        if (y > 0) {
            c = a + b;
            d = a - b;
            
            /* Label creating additional basic block */
            if (c > d) {
                e = c * d;
                f = c / (d + 1);
                
                /* Goto creating control flow edge */
                if (e > 1000) goto large_value;
            } else {
                e = d * c;
                f = d / (c + 1);
            }
        } else {
            c = a - b;
            d = a + b;
        }
    } else {
        a = x - 1;
        b = y - 1;
    }
    
large_value:
    
    /* Loop with break/continue creating multiple exits */
    for (int z = 0; z < 10; z++) {
        g = a + z;
        h = b - z;
        
        if (z % 2 == 0) {
            i = g * h;
            continue;
        } else {
            j = g / (h + 1);
            if (j > 50) break;
        }
        
        k = i + j;
        l = i - j;
    }
    
    /* Final computation */
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

int main(int argc, char **argv) {
    int total = 0;
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    printf("Running MCF trigger test with %d iterations\n", iterations);
    
    /* Call complex functions multiple times to ensure MCF runs */
    for (int i = 0; i < iterations; i++) {
        int seed = i * 12345;
        
        /* Call first complex function */
        int result1 = complex_mcf_test(50 + (i % 20), seed);
        
        /* Call second complex function */
        int result2 = another_complex_function(seed, seed + 1);
        
        total += result1 + result2;
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            asm volatile ("# Loop barrier\n\t" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", total);
    printf("Sink value: %d\n", sink);
    
    return total != 0 ? 0 : 1;
}
