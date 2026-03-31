/* mcf_coverage.c - Program to trigger MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Complex test function with high register pressure */
static int __attribute__((noinline)) test_mcf(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f, f4 = seed * 0.5f;
    
    volatile int* p0 = &v0, *p1 = &v1, *p2 = &v2, *p3 = &v3;
    
    int result = 0;
    int i, j;
    
    /* Complex loop with data dependencies */
    for (i = 0; i < 100; i++) {
        /* Chain of dependent operations */
        v0 = v1 + v2;
        v1 = v3 * v4;
        v2 = v5 - v6;
        v3 = v7 / (v8 + 1);
        v4 = v9 ^ v10;
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = v15 << 2;
        v8 = v16 >> 1;
        v9 = v17 + v18;
        v10 = v19 * v20;
        
        /* Mix integer and float operations */
        f0 = f1 * 1.1f + (float)v0;
        f1 = f2 / 2.0f - (float)v1;
        f2 = f3 + 3.14f * (float)v2;
        f3 = f4 - 0.5f * (float)v3;
        
        /* Pointer arithmetic */
        *p0 = *p1 + *p2;
        *p1 = *p3 * 2;
        p2 = &v21 + (i % 10);
        p3 = &v22 - (i % 5);
        
        /* Complex conditional structure creating many basic blocks */
        if (i % 3 == 0) {
            v11 = v12 + v13;
            if (i % 7 == 0) {
                v12 = v14 * v15;
                goto label_a;
            } else {
                v13 = v16 - v17;
                goto label_b;
            }
        } else if (i % 5 == 0) {
            v14 = v18 ^ v19;
            if (i % 11 == 0) {
                v15 = v20 | v21;
            }
        } else {
            v16 = v22 & v23;
        }
        
        /* More operations after labels */
        v17 = v24 + v25;
        
    label_a:
        v18 = v26 * v27;
        
    label_b:
        v19 = v28 - v29;
        
        /* Large switch statement creating many basic blocks */
        switch (i % 13) {
            case 0: v20 = v0 + 1; break;
            case 1: v21 = v1 * 2; break;
            case 2: v22 = v2 - 3; break;
            case 3: v23 = v3 / 4; break;
            case 4: v24 = v4 ^ 5; break;
            case 5: v25 = v5 | 6; break;
            case 6: v26 = v6 & 7; break;
            case 7: v27 = v7 << 1; break;
            case 8: v28 = v8 >> 2; break;
            case 9: v29 = v9 + v10; break;
            case 10: v0 = v11 - v12; break;
            case 11: v1 = v13 * v14; break;
            case 12: v2 = v15 / (v16 + 1); break;
        }
        
        /* Inline assembly with register clobbers to force graph transformations */
        /* For x86 */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+m" (v0)
            : "r" (v1)
            : "%eax", "%ebx", "%ecx", "%edx", "memory"
        );
        
        /* Another asm with different clobbers */
        asm volatile (
            "movl %0, %%ebx\n\t"
            "imull %1, %%ebx\n\t"
            "movl %%ebx, %0\n\t"
            : "+m" (v2)
            : "r" (v3)
            : "%eax", "%ebx", "%ecx", "%edx", "memory"
        );
        
        /* Nested loop for additional complexity */
        for (j = 0; j < 5; j++) {
            v21 = v22 + v23 + j;
            v22 = v24 * v25 - j;
            if (j % 2 == 0) {
                v23 = v26 ^ v27;
                continue;
            } else {
                v24 = v28 | v29;
                if (j == 3) break;
            }
        }
        
        /* Accumulate result */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        result += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    }
    
    return result;
}

/* Another complex function to prevent interprocedural optimization */
static int __attribute__((noinline)) test_mcf2(int seed) {
    volatile int a = seed, b = seed * 2, c = seed * 3;
    volatile int d = seed * 4, e = seed * 5, f = seed * 6;
    volatile int g = seed * 7, h = seed * 8, i = seed * 9, j = seed * 10;
    
    /* Complex control flow with gotos */
    if (seed % 2 == 0) {
        a = b + c;
        goto compute1;
    } else {
        d = e - f;
        goto compute2;
    }
    
compute1:
    g = h * i;
    if (g > 1000) {
        j = a + d;
        goto finish;
    } else {
        j = g - h;
        goto compute2;
    }
    
compute2:
    a = d * e;
    b = f + g;
    
finish:
    /* Force spill/reload with inline asm */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl %%eax, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+m" (a)
        :: "%eax", "%ebx", "%ecx", "%edx", "memory"
    );
    
    return a + b + c + d + e + f + g + h + i + j;
}

int main(void) {
    int total = 0;
    int i;
    
    /* Call test functions multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        total += test_mcf(i);
        total += test_mcf2(i * 7);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
