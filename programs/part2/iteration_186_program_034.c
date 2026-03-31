/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) test_mcf(int seed) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mix types to increase pressure on different register classes */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2;
    
    int result = 0;
    int i, j;
    
    /* Complex loop with data dependencies */
    for (i = 0; i < 100; i++) {
        /* Chain of arithmetic operations creating data dependencies */
        v0 = v1 + v2;
        v1 = v3 * v4;
        v2 = v5 - v6;
        v3 = v7 ^ v8;
        v4 = v9 | v10;
        v5 = v11 & v12;
        v6 = v13 << 2;
        v7 = v14 >> 1;
        v8 = v15 + v16;
        v9 = v17 * v18;
        v10 = v19 - v20;
        v11 = v21 ^ v22;
        v12 = v23 | v24;
        v13 = v25 & v26;
        v14 = v27 << 3;
        v15 = v28 >> 2;
        v16 = v29 + v0;
        
        /* Floating point operations */
        f0 = f1 * 1.1f;
        f1 = f2 + 0.5f;
        f2 = f0 - 0.3f;
        d0 = d1 * 1.01;
        d1 = d0 + 0.02;
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + 1;
        p1 = (char*)p2 - 1;
        p2 = p0;
        
        /* Complex conditional structure creating many basic blocks */
        if (i % 3 == 0) {
            v17 = v0 + v1;
            if (i % 5 == 0) {
                v18 = v2 * v3;
                goto label_a;
            } else {
                v19 = v4 - v5;
                goto label_b;
            }
        } else if (i % 3 == 1) {
            v20 = v6 ^ v7;
            if (i % 7 == 0) {
                v21 = v8 | v9;
                goto label_c;
            }
        } else {
            v22 = v10 & v11;
        }
        
        /* Continue normal flow */
        v23 = v12 + v13;
        goto label_merge;
        
    label_a:
        v24 = v14 * v15;
        goto label_merge;
        
    label_b:
        v25 = v16 - v17;
        goto label_merge;
        
    label_c:
        v26 = v18 ^ v19;
        /* Fall through */
        
    label_merge:
        v27 = v20 + v21;
        v28 = v22 * v23;
        v29 = v24 - v25;
        
        /* Inline assembly with register clobbers to force graph transformations */
        asm volatile (
            "# Force register pressure\n"
            : 
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5)
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Switch statement with many cases creating complex control flow */
        switch (i % 13) {
            case 0: v0 += v1; break;
            case 1: v1 += v2; break;
            case 2: v2 += v3; break;
            case 3: v3 += v4; break;
            case 4: v4 += v5; break;
            case 5: v5 += v6; break;
            case 6: v6 += v7; break;
            case 7: v7 += v8; break;
            case 8: v8 += v9; break;
            case 9: v9 += v10; break;
            case 10: v10 += v11; break;
            case 11: v11 += v12; break;
            case 12: v12 += v13; break;
            default: v13 += v14; break;
        }
        
        /* Nested loop with break/continue creating more control flow edges */
        for (j = 0; j < 10; j++) {
            if (j == 5) continue;
            v14 += j;
            if (j == 8) break;
            v15 += j * 2;
        }
    }
    
    /* Final computation using all variables */
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
             v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
             v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
             (int)f0 + (int)f1 + (int)f2 + (int)d0 + (int)d1 +
             (int)(long)p0 + (int)(long)p1 + (int)(long)p2;
    
    /* Prevent dead code elimination */
    USE(v0); USE(v1); USE(v2); USE(v3); USE(v4); USE(v5);
    USE(v6); USE(v7); USE(v8); USE(v9); USE(v10); USE(v11);
    USE(v12); USE(v13); USE(v14); USE(v15); USE(v16); USE(v17);
    USE(v18); USE(v19); USE(v20); USE(v21); USE(v22); USE(v23);
    USE(v24); USE(v25); USE(v26); USE(v27); USE(v28); USE(v29);
    USE(f0); USE(f1); USE(f2); USE(d0); USE(d1);
    USE(p0); USE(p1); USE(p2);
    
    return result;
}

/* Another complex function to increase overall compilation complexity */
static int __attribute__((noinline)) helper_func(int x) {
    volatile int a = x, b = x * 2, c = x * 3;
    volatile float fa = x * 0.5f;
    
    /* More inline assembly with different clobbers */
    asm volatile (
        "# Additional pressure\n"
        : 
        : "r"(a), "r"(b), "r"(c)
        : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
    );
    
    switch (x % 8) {
        case 0: a = b + c; break;
        case 1: b = c - a; break;
        case 2: c = a * b; break;
        case 3: a = b ^ c; break;
        case 4: b = c | a; break;
        case 5: c = a & b; break;
        case 6: a = b << 1; break;
        case 7: b = c >> 1; break;
    }
    
    return a + b + c + (int)fa;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int res1 = test_mcf(i * 100);
        int res2 = helper_func(i * 50);
        total += res1 + res2;
        
        /* Prevent optimization across iterations */
        asm volatile("" : : "r"(res1), "r"(res2));
    }
    
    printf("Result: %d\n", total);
    return 0;
}
