/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with high register pressure and control flow */
static int __attribute__((noinline)) complex_mcf_function(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed ^ 0x1234;
    volatile int v4 = seed | 0xABCD;
    volatile int v5 = seed & 0xF0F0;
    volatile int v6 = seed << 3;
    volatile int v7 = seed >> 2;
    volatile int v8 = seed + v0;
    volatile int v9 = v1 * v2;
    volatile int v10 = v3 ^ v4;
    volatile int v11 = v5 | v6;
    volatile int v12 = v7 & v8;
    volatile int v13 = v9 + v10;
    volatile int v14 = v11 * v12;
    volatile int v15 = v13 ^ v14;
    volatile int v16 = v0 | v15;
    volatile int v17 = v1 & v16;
    volatile int v18 = v2 + v17;
    volatile int v19 = v3 * v18;
    volatile int v20 = v4 ^ v19;
    volatile int v21 = v5 | v20;
    volatile int v22 = v6 & v21;
    volatile int v23 = v7 + v22;
    volatile int v24 = v8 * v23;
    volatile int v25 = v9 ^ v24;
    volatile int v26 = v10 | v25;
    volatile int v27 = v11 & v26;
    volatile int v28 = v12 + v27;
    volatile int v29 = v13 * v28;
    
    int result = 0;
    
    /* Complex control flow with many basic blocks */
    for (int i = 0; i < 100; i++) {
        /* Inline assembly to clobber registers and force graph transformations */
        asm volatile (
            "# Force register clobbering\n"
            : 
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Nested conditionals creating many basic blocks */
        if (i % 3 == 0) {
            v0 = v1 + v2;
            if (v0 > 1000) {
                v3 = v4 * v5;
                goto label_a;  /* Create control flow edges */
            } else {
                v6 = v7 ^ v8;
                if (v6 < 500) {
                    v9 = v10 | v11;
                }
            }
        } else if (i % 3 == 1) {
            v12 = v13 & v14;
            if (v12 == 0) {
                v15 = v16 + v17;
            }
        } else {
            v18 = v19 * v20;
        }
        
        /* Switch with many cases to create more basic blocks */
        switch (i % 10) {
            case 0: v21 = v22 + 1; break;
            case 1: v21 = v23 * 2; break;
            case 2: v21 = v24 ^ 3; break;
            case 3: v21 = v25 | 4; break;
            case 4: v21 = v26 & 5; break;
            case 5: v21 = v27 + 6; break;
            case 6: v21 = v28 * 7; break;
            case 7: v21 = v29 ^ 8; break;
            case 8: v21 = v0 | 9; break;
            case 9: v21 = v1 & 10; break;
            default: v21 = 0;
        }
        
        /* Another level of nested conditionals */
        if (i % 7 == 0) {
            v2 = v3 + v4;
            if (v2 % 2 == 0) {
                v5 = v6 * v7;
            } else {
                v8 = v9 ^ v10;
                if (v8 > 100) {
                    v11 = v12 | v13;
                }
            }
        }
        
        /* Loop with break/continue creating exit edges */
        for (int j = 0; j < 5; j++) {
            if (j == 3 && (i % 4 == 0)) {
                v14 = v15 + v16;
                break;  /* Creates exit edge */
            }
            v17 = v18 * j;
            if (j == 2) {
                continue;  /* Creates back edge */
            }
            v19 = v20 ^ j;
        }
        
        /* Compute result with data dependencies */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        /* Mix types to increase pressure */
        float f1 = v0 * 1.5f;
        float f2 = v1 * 2.5f;
        double d1 = v2 * 3.14159;
        void* ptr = &v3;
        
        /* Use all variables to prevent elimination */
        asm volatile (
            "# Ensure all variables are used\n"
            : 
            : "r"(f1), "r"(f2), "r"(d1), "r"(ptr)
            : "xmm0", "xmm1", "xmm2", "memory"
        );
        
        label_a:
        /* More computations to extend live ranges */
        v0 = v1 ^ v2;
        v1 = v3 | v4;
        v2 = v5 & v6;
    }
    
    return result;
}

/* Another complex function to prevent interprocedural optimization */
static int __attribute__((noinline)) another_complex_function(int base) {
    volatile int a = base, b = base + 1, c = base + 2;
    volatile int d, e, f, g, h, i, j, k, l, m;
    
    /* Complex expression with many intermediate values */
    for (int x = 0; x < 50; x++) {
        d = a * b + c;
        e = d ^ a;
        f = e | b;
        g = f & c;
        h = g + d;
        i = h * e;
        j = i ^ f;
        k = j | g;
        l = k & h;
        m = l + i;
        
        /* Conditional with goto to create more edges */
        if (x % 11 == 0) {
            a = b + c;
            goto extra_computation;
        }
        
        if (x % 13 == 0) {
            b = c * d;
            goto skip_point;
        }
        
        extra_computation:
        c = d ^ e;
        
        skip_point:
        /* Use inline asm with clobbers */
        asm volatile (
            "# More register pressure\n"
            : "=r"(a), "=r"(b), "=r"(c)
            : "0"(a), "1"(b), "2"(c)
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m;
}

int main(void) {
    int total = 0;
    
    /* Call functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += complex_mcf_function(i * 100);
        total += another_complex_function(i * 50);
        
        /* Prevent loop unrolling */
        asm volatile (
            "# Prevent optimization\n"
            : 
            : "r"(total)
            : "memory"
        );
    }
    
    printf("Result: %d\n", total);
    return 0;
}
