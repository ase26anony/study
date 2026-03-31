/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
#define NO_OPTIMIZE(x) __asm__ volatile("" : "+r" (x))

/* Complex test function with maximum register pressure */
static int __attribute__((noinline)) 
complex_mcf_test(int seed, int iterations) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mix different types to increase pressure */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* ptr0 = &v0, *ptr1 = &v1, *ptr2 = &v2;
    
    int result = 0;
    
    /* Complex loop with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Chain of dependent operations */
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
        
        /* Inline assembly with register clobbering */
        /* Force compiler to work around clobbered registers */
        __asm__ volatile (
            "# Force register pressure\n"
            : 
            : "r" (v0), "r" (v1), "r" (v2), "r" (v3), "r" (v4)
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Complex conditional structure creating many basic blocks */
        switch (i % 15) {
            case 0: v11 = v21 + v22; break;
            case 1: v11 = v21 - v22; break;
            case 2: v11 = v21 * v22; break;
            case 3: v11 = v21 ^ v22; break;
            case 4: v11 = v21 | v22; break;
            case 5: v11 = v21 & v22; break;
            case 6: v11 = v21 << (i % 4); break;
            case 7: v11 = v21 >> (i % 4); break;
            case 8: v11 = v22 + v23; break;
            case 9: v11 = v22 - v23; break;
            case 10: v11 = v22 * v23; break;
            case 11: v11 = v22 ^ v23; break;
            case 12: v11 = v22 | v23; break;
            case 13: v11 = v22 & v23; break;
            case 14: v11 = v23 << (i % 4); break;
            default: v11 = 0;
        }
        
        /* Nested conditionals */
        if (i % 3 == 0) {
            v12 = v24 + v25;
            if (i % 6 == 0) {
                v13 = v26 * v27;
                goto label1;  /* Create irregular control flow */
            } else {
                v13 = v26 - v27;
            }
            v14 = v28 ^ v29;
        } else if (i % 3 == 1) {
            v12 = v24 - v25;
            v13 = v26 | v27;
            v14 = v28 & v29;
        } else {
            v12 = v24 * v25;
            v13 = v26 ^ v27;
            v14 = v28 | v29;
        }
        
        /* Another inline assembly with different clobbers */
        __asm__ volatile (
            "# More register pressure\n"
            : "+r" (v15), "+r" (v16)
            : 
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Floating point operations to use FP registers */
        f0 = f1 * 1.1f + f2;
        f1 = f0 * 0.9f - f2;
        f2 = f1 + f0 * 0.5f;
        
        d0 = d1 * 1.01;
        d1 = d0 * 0.99;
        
        /* Pointer arithmetic */
        ptr0 = (char*)ptr1 + v0;
        ptr1 = (char*)ptr2 - v1;
        ptr2 = (char*)ptr0 + v2;
        
        /* Loop with early exit creating exit-like blocks */
        if (i % 7 == 0) {
            for (int j = 0; j < 5; j++) {
                v15 += v16 + j;
                if (j == 3 && (i % 14 == 0)) {
                    break;  /* Early exit from inner loop */
                }
                v16 += v15 * j;
            }
        }
        
        label1:
        /* More arithmetic chains */
        v17 = v0 + v1 + v2;
        v18 = v3 * v4 * v5;
        v19 = v6 - v7 - v8;
        v20 = v9 ^ v10 ^ v11;
        v21 = v12 | v13 | v14;
        v22 = v15 & v16 & v17;
        v23 = v18 << (v19 % 4);
        v24 = v20 >> (v21 % 4);
        
        /* Prevent dead code elimination */
        NO_OPTIMIZE(v0); NO_OPTIMIZE(v1); NO_OPTIMIZE(v2);
        NO_OPTIMIZE(v3); NO_OPTIMIZE(v4); NO_OPTIMIZE(v5);
        NO_OPTIMIZE(v6); NO_OPTIMIZE(v7); NO_OPTIMIZE(v8);
        NO_OPTIMIZE(v9); NO_OPTIMIZE(v10); NO_OPTIMIZE(v11);
        NO_OPTIMIZE(v12); NO_OPTIMIZE(v13); NO_OPTIMIZE(v14);
        NO_OPTIMIZE(v15); NO_OPTIMIZE(v16); NO_OPTIMIZE(v17);
        NO_OPTIMIZE(v18); NO_OPTIMIZE(v19); NO_OPTIMIZE(v20);
        NO_OPTIMIZE(v21); NO_OPTIMIZE(v22); NO_OPTIMIZE(v23);
        NO_OPTIMIZE(v24); NO_OPTIMIZE(v25); NO_OPTIMIZE(v26);
        NO_OPTIMIZE(v27); NO_OPTIMIZE(v28); NO_OPTIMIZE(v29);
        NO_OPTIMIZE(f0); NO_OPTIMIZE(f1); NO_OPTIMIZE(f2);
        NO_OPTIMIZE(d0); NO_OPTIMIZE(d1);
        NO_OPTIMIZE(ptr0); NO_OPTIMIZE(ptr1); NO_OPTIMIZE(ptr2);
        
        /* Accumulate result */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                 (int)f0 + (int)f1 + (int)f2 + (int)d0 + (int)d1;
    }
    
    return result;
}

/* Another complex function to ensure MCF runs multiple times */
static int __attribute__((noinline))
another_mcf_test(int base) {
    volatile int a = base, b = base + 100, c = base + 200;
    volatile int d = base + 300, e = base + 400, f = base + 500;
    
    /* Complex control flow with gotos */
    if (base % 2 == 0) {
        goto block_a;
    } else {
        goto block_b;
    }
    
    block_a:
    a = b * c;
    if (a > 1000) {
        goto block_c;
    } else {
        goto block_d;
    }
    
    block_b:
    a = b + c;
    if (a < 500) {
        goto block_e;
    } else {
        goto block_f;
    }
    
    block_c:
    d = e ^ f;
    goto block_g;
    
    block_d:
    d = e | f;
    goto block_g;
    
    block_e:
    d = e & f;
    goto block_g;
    
    block_f:
    d = e - f;
    /* fall through */
    
    block_g:
    /* More operations */
    for (int i = 0; i < 10; i++) {
        a += b * i;
        b += c << (i % 3);
        c += d >> (i % 3);
        
        /* Inline assembly with many clobbers */
        __asm__ volatile (
            "# Complex constraints\n"
            : "+r" (a), "+r" (b), "+r" (c)
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
    }
    
    return a + b + c + d + e + f;
}

int main(void) {
    int total = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int r1 = complex_mcf_test(i * 100, 50);
        int r2 = another_mcf_test(i * 50);
        
        total += r1 + r2;
        
        printf("Iteration %d: r1=%d, r2=%d, total=%d\n", 
               i, r1, r2, total);
    }
    
    printf("Final checksum: %d\n", total);
    printf("Test completed. Check GCC dump files for MCF output.\n");
    
    return 0;
}
