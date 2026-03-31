/* mcf_test.c - Test program to trigger GCC's Minimum Cost Flow pass */
#include <stdio.h>
#include <stdlib.h>

/* Force MCF pass activation with complex control flow and register pressure */
__attribute__((noinline))
static int complex_mcf_function(int seed) {
    /* Declare many variables to create register pressure */
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
    
    /* Complex loop with many iterations to create many basic blocks */
    for (int i = 0; i < 100; i++) {
        /* Chain of dependent arithmetic operations */
        v0 = v1 + v2;
        v1 = v2 * v3;
        v2 = v3 - v4;
        v3 = v4 ^ v5;
        v4 = v5 | v6;
        v5 = v6 & v7;
        v6 = v7 << 2;
        v7 = v8 >> 1;
        v8 = v9 + v10;
        v9 = v10 * v11;
        v10 = v11 - v12;
        v11 = v12 ^ v13;
        v12 = v13 | v14;
        v13 = v14 & v15;
        v14 = v15 << 3;
        v15 = v16 >> 2;
        
        /* Floating point operations to pressure FP registers */
        f0 = f1 * 1.1f;
        f1 = f2 + 2.2f;
        f2 = f3 - 3.3f;
        f3 = f4 * 4.4f;
        f4 = f0 / 5.5f;
        
        /* Pointer operations */
        *p0 = *p1 + *p2;
        *p1 = *p2 - *p3;
        p0 = p1;
        p1 = p2;
        p2 = p3;
        
        /* Complex conditional structure creating many basic blocks */
        if (i % 3 == 0) {
            v16 = v17 + v18;
            v17 = v18 * v19;
            if (i % 6 == 0) {
                v18 = v19 - v20;
                v19 = v20 ^ v21;
                goto label_a;  /* Create control flow edges */
            } else {
                v20 = v21 | v22;
                v21 = v22 & v23;
                goto label_b;
            }
        } else if (i % 5 == 0) {
            v22 = v23 << 1;
            v23 = v24 >> 1;
            if (i % 10 == 0) {
                v24 = v25 + v26;
                goto label_c;
            }
        } else {
            v25 = v26 * v27;
            v26 = v27 - v28;
        }
        
        /* Continue with more operations */
        v27 = v28 ^ v29;
        v28 = v29 | v0;
        v29 = v0 & v1;
        
        /* Inline assembly with register clobbers to force graph transformations */
        asm volatile (
            "# Force register pressure\n"
            : "=r"(v0), "=r"(v1), "=r"(v2)
            : "0"(v0), "1"(v1), "2"(v2)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Switch statement with many cases for complex control flow */
        switch (i % 13) {
            case 0: result += v0; break;
            case 1: result += v1; break;
            case 2: result += v2; break;
            case 3: result += v3; break;
            case 4: result += v4; break;
            case 5: result += v5; break;
            case 6: result += v6; break;
            case 7: result += v7; break;
            case 8: result += v8; break;
            case 9: result += v9; break;
            case 10: result += v10; break;
            case 11: result += v11; break;
            case 12: result += v12; break;
        }
        
        continue;
        
    label_a:
        v0 = v1 * 2;
        continue;
        
    label_b:
        v1 = v2 / 2;
        continue;
        
    label_c:
        v2 = v3 + 100;
        continue;
    }
    
    /* Final computation mixing all variables */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    result += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    result += *p0 + *p1 + *p2 + *p3;
    
    return result;
}

/* Another complex function to ensure MCF runs multiple times */
__attribute__((noinline))
static int another_mcf_function(int base) {
    volatile int a = base, b = base + 1, c = base + 2, d = base + 3;
    volatile int e = base + 4, f = base + 5, g = base + 6, h = base + 7;
    volatile int i = base + 8, j = base + 9, k = base + 10, l = base + 11;
    
    int sum = 0;
    
    /* Loop with early exits creating multiple exit blocks */
    for (int iter = 0; iter < 50; iter++) {
        if (iter % 7 == 0) {
            a = b * c;
            if (a > 1000) break;  /* Early exit */
        }
        
        if (iter % 11 == 0) {
            d = e + f;
            if (d < 0) goto early_exit;  /* Another exit path */
        }
        
        /* Complex expression with many intermediate values */
        g = ((h * i) + (j / k)) | (l ^ a);
        h = ((i * j) - (k / l)) & (a ^ b);
        i = ((j * k) >> (l % 4)) + (b ^ c);
        
        /* More inline assembly with different clobbers */
        asm volatile (
            "# More register pressure\n"
            : "+r"(g), "+r"(h), "+r"(i)
            :
            : "esi", "edi", "ebp", "memory"
        );
        
        sum += g + h + i;
    }
    
    return sum;
    
early_exit:
    return sum + a + b + c;
}

int main(void) {
    int total = 0;
    
    /* Call functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += complex_mcf_function(i * 100);
        total += another_mcf_function(i * 50);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
