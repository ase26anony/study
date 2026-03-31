/* mcf_test.c - Test program to trigger MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Complex test function with high register pressure */
static int __attribute__((noinline)) 
complex_mcf_test(int seed) 
{
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed - 4;
    volatile int v4 = seed % 5;
    volatile int v5 = seed | 0x55;
    volatile int v6 = seed & 0xAA;
    volatile int v7 = seed ^ 0xFF;
    volatile int v8 = seed << 2;
    volatile int v9 = seed >> 1;
    volatile int v10 = v0 + v1;
    volatile int v11 = v2 * v3;
    volatile int v12 = v4 ^ v5;
    volatile int v13 = v6 | v7;
    volatile int v14 = v8 & v9;
    volatile int v15 = v10 - v11;
    volatile int v16 = v12 + v13;
    volatile int v17 = v14 * v15;
    volatile int v18 = v16 / (v17 ? v17 : 1);
    volatile int v19 = v18 << 3;
    volatile int v20 = v19 >> 2;
    volatile int v21 = v20 | 0x1234;
    volatile int v22 = v21 & 0xABCD;
    volatile int v23 = v22 ^ 0xDEAD;
    volatile int v24 = v23 * 0xBEEF;
    volatile int v25 = v24 + 0xCAFE;
    volatile int v26 = v25 - 0xF00D;
    volatile int v27 = v26 % 0x1000;
    volatile int v28 = v27 << 4;
    volatile int v29 = v28 >> 2;
    
    int result = 0;
    
    /* Complex control flow with many basic blocks */
    for (int i = 0; i < 100; i++) {
        /* Nested conditionals creating control flow splits */
        if (i % 3 == 0) {
            v0 = v1 + v2;
            /* Inline assembly with register clobbers */
            asm volatile (
                "# Force register pressure\n\t"
                : 
                : "r"(v0), "r"(v1), "r"(v2)
                : "eax", "ebx", "ecx", "edx", "esi", "edi"
            );
            goto label1;
        } else if (i % 3 == 1) {
            v3 = v4 * v5;
            asm volatile (
                "# More register pressure\n\t"
                : 
                : "r"(v3), "r"(v4), "r"(v5)
                : "eax", "ebx", "ecx"
            );
            goto label2;
        } else {
            v6 = v7 ^ v8;
            asm volatile (
                "# Different clobber set\n\t"
                : 
                : "r"(v6), "r"(v7), "r"(v8)
                : "edx", "esi", "edi"
            );
            goto label3;
        }
        
    label1:
        v9 = v10 - v11;
        if (v9 > 0) {
            v12 = v13 | v14;
            result += v12;
        } else {
            v15 = v16 & v17;
            result -= v15;
        }
        continue;
        
    label2:
        v18 = v19 << 1;
        switch (i % 7) {
            case 0: v20 = v21 + 1; break;
            case 1: v20 = v22 * 2; break;
            case 2: v20 = v23 / 3; break;
            case 3: v20 = v24 - 4; break;
            case 4: v20 = v25 % 5; break;
            case 5: v20 = v26 | 6; break;
            case 6: v20 = v27 & 7; break;
            default: v20 = 0;
        }
        result ^= v20;
        continue;
        
    label3:
        v28 = v29 * i;
        /* More inline assembly with varying clobbers */
        asm volatile (
            "# Mixed register clobbers\n\t"
            : "+r"(v28)
            : 
            : "memory", "eax", "ebx", "ecx", "edx"
        );
        
        /* Deep conditional chain */
        if (v28 < 1000) {
            if (v28 < 500) {
                if (v28 < 250) {
                    result += v28 * 2;
                } else {
                    result += v28 / 2;
                }
            } else {
                if (v28 < 750) {
                    result -= v28 * 3;
                } else {
                    result -= v28 / 3;
                }
            }
        } else {
            if (v28 < 2000) {
                if (v28 < 1500) {
                    result |= v28;
                } else {
                    result &= v28;
                }
            } else {
                result ^= v28;
            }
        }
    }
    
    /* Final computation using all variables */
    result += v0 + v1 + v2 + v3 + v4 + v5;
    result += v6 + v7 + v8 + v9 + v10 + v11;
    result += v12 + v13 + v14 + v15 + v16 + v17;
    result += v18 + v19 + v20 + v21 + v22 + v23;
    result += v24 + v25 + v26 + v27 + v28 + v29;
    
    return result;
}

/* Another complex function to prevent interprocedural optimization */
static int __attribute__((noinline))
another_complex_function(int x, int y) 
{
    volatile int a = x, b = y;
    volatile int c, d, e, f, g, h;
    
    /* Loop with complex exit conditions */
    for (int i = 0; i < 50; i++) {
        c = a * b;
        d = c + i;
        
        /* Force graph restructuring with goto */
        if (d % 2 == 0) {
            e = d << 1;
            goto compute_f;
        } else {
            e = d >> 1;
            goto compute_g;
        }
        
    compute_f:
        f = e * 3;
        /* Inline assembly that forces compensation code */
        asm volatile (
            "# Force graph fixup\n\t"
            : "+r"(f)
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        a = f;
        continue;
        
    compute_g:
        g = e / 3;
        asm volatile (
            "# Different clobber pattern\n\t"
            : "+r"(g)
            : 
            : "eax", "ebx", "memory"
        );
        b = g;
        
        /* Early exit with multiple paths */
        if (g > 1000) {
            h = a + b + c + d + e + f + g;
            return h;
        }
    }
    
    return a + b;
}

int main(void) 
{
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += complex_mcf_test(i * 100);
        total += another_complex_function(i, i * 2);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
