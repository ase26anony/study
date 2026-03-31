/* mcf_coverage.c - Program to trigger GCC's MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex test function with maximum register pressure */
static int __attribute__((noinline)) 
test_mcf_pressure(int seed, int iterations) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed ^ 0x1234;
    volatile int v3 = seed - 456;
    volatile int v4 = seed | 0xABCD;
    volatile int v5 = seed & 0xF0F0;
    volatile int v6 = seed << 3;
    volatile int v7 = seed >> 2;
    volatile int v8 = ~seed;
    volatile int v9 = seed % 17;
    volatile int v10 = seed + v0;
    volatile int v11 = v1 * v2;
    volatile int v12 = v3 ^ v4;
    volatile int v13 = v5 - v6;
    volatile int v14 = v7 | v8;
    volatile int v15 = v9 & v10;
    volatile int v16 = v11 << 1;
    volatile int v17 = v12 >> 2;
    volatile int v18 = ~v13;
    volatile int v19 = v14 % 19;
    volatile int v20 = v15 + v16;
    volatile int v21 = v17 * v18;
    volatile int v22 = v19 ^ v20;
    volatile int v23 = v21 - v22;
    volatile int v24 = v23 | 0xDEAD;
    volatile int v25 = v24 & 0xBEEF;
    volatile int v26 = v25 << 4;
    volatile int v27 = v26 >> 1;
    volatile int v28 = ~v27;
    volatile int v29 = v28 % 23;
    
    /* Additional floating point variables for mixed register class pressure */
    volatile float f0 = seed * 0.1f;
    volatile float f1 = v0 * 0.2f;
    volatile float f2 = v1 * 0.3f;
    volatile float f3 = v2 * 0.4f;
    volatile float f4 = v3 * 0.5f;
    
    /* Pointer variables for additional pressure */
    volatile int* p0 = &v0;
    volatile int* p1 = &v1;
    volatile int* p2 = &v2;
    volatile int* p3 = &v3;
    volatile int* p4 = &v4;
    
    int result = 0;
    int i, j;
    
    /* Complex loop with many basic blocks */
    for (i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers and force graph modifications */
        asm volatile (
            "# MCF pressure point 1\n"
            : 
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4)
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Deep conditional chain creating many basic blocks */
        if (i % 3 == 0) {
            v0 = v1 + v2;
            f0 = f1 + f2;
            *p0 = *p1 + *p2;
            goto label_a;
        } else if (i % 3 == 1) {
            v3 = v4 + v5;
            f1 = f2 + f3;
            *p1 = *p2 + *p3;
            goto label_b;
        } else {
            v6 = v7 + v8;
            f2 = f3 + f4;
            *p2 = *p3 + *p4;
            goto label_c;
        }
        
    label_a:
        /* More computations in each label */
        v10 = v11 * v12;
        f0 = f0 * 1.1f;
        if (v10 > 1000) {
            v11 = v12 / v13;
            goto label_merge;
        } else {
            v11 = v13 / v12;
            goto label_d;
        }
        
    label_b:
        v13 = v14 ^ v15;
        f1 = f1 * 1.2f;
        switch (v13 & 0x7) {
            case 0: v14 = v15 + 1; break;
            case 1: v14 = v15 - 1; break;
            case 2: v14 = v15 * 2; break;
            case 3: v14 = v15 / 2; break;
            case 4: v14 = v15 ^ 0xFF; break;
            case 5: v14 = v15 | 0xAA; break;
            case 6: v14 = v15 & 0x55; break;
            case 7: v14 = ~v15; break;
            default: v14 = 0; break;
        }
        goto label_merge;
        
    label_c:
        v16 = v17 | v18;
        f2 = f2 * 1.3f;
        for (j = 0; j < 3; j++) {
            v17 += j;
            if (j == 1) continue;
            v18 -= j;
            if (j == 2) break;
        }
        goto label_d;
        
    label_d:
        v19 = v20 & v21;
        f3 = f3 * 1.4f;
        if (v19 != 0) {
            v20 = v21 << (v19 & 0x3);
        } else {
            v20 = v21 >> 1;
        }
        /* Fall through to label_merge */
        
    label_merge:
        /* Merge point with more computations */
        v22 = v23 + v24 + v25;
        f4 = f0 + f1 + f2 + f3;
        
        /* Another inline assembly with different clobbers */
        asm volatile (
            "# MCF pressure point 2\n"
            : 
            : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Complex expression spanning multiple operations */
        v26 = ((v22 * v23) + (v24 ^ v25)) | ((v26 << 2) & 0xFFFF);
        v27 = (v27 % 17) + (v28 / 3) - (v29 * 2);
        
        /* Update result with all variables to keep them alive */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                  v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                  v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                  (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                  *p0 + *p1 + *p2 + *p3 + *p4;
    }
    
    /* Final complex computation */
    int final = 0;
    switch (result & 0xF) {
        case 0: final = v0 + v29; break;
        case 1: final = v1 * v28; break;
        case 2: final = v2 ^ v27; break;
        case 3: final = v3 | v26; break;
        case 4: final = v4 & v25; break;
        case 5: final = v5 << (v24 & 3); break;
        case 6: final = v6 >> (v23 & 3); break;
        case 7: final = ~v7 + ~v22; break;
        case 8: final = v8 % (v21 + 1); break;
        case 9: final = v9 - v20; break;
        case 10: final = v10 + v19; break;
        case 11: final = v11 * v18; break;
        case 12: final = v12 ^ v17; break;
        case 13: final = v13 | v16; break;
        case 14: final = v14 & v15; break;
        case 15: final = (v0 + v15 + v29) & 0xFF; break;
    }
    
    USE(final);
    return result + final;
}

/* Another complex function to create interprocedural pressure */
static int __attribute__((noinline))
helper_function(int x, int y) {
    volatile int a = x, b = y;
    volatile int c, d, e, f, g, h;
    
    /* Complex control flow with gotos */
    if (x > y) {
        c = a * b;
        goto compute;
    } else if (x < y) {
        c = a + b;
        goto compute;
    } else {
        c = a ^ b;
    }
    
    d = c << 2;
    e = d >> 1;
    
    if (e & 1) {
        f = e * 3;
        goto finish;
    }
    
compute:
    f = c * 2;
    g = f + 1;
    
    if (g > 100) {
        h = g / 2;
    } else {
        h = g * 2;
    }
    
finish:
    /* Force register pressure with inline asm */
    asm volatile (
        "# Helper function clobber\n"
        : 
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    return a + b + c + d + e + f + g + h;
}

int main(void) {
    int total = 0;
    int i;
    
    printf("Starting MCF coverage test...\n");
    
    /* Call test function multiple times with different seeds */
    for (i = 0; i < 10; i++) {
        int result = test_mcf_pressure(i * 123, 5 + (i % 3));
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
        
        /* Also call helper to create more compilation complexity */
        total += helper_function(i, result & 0xFF);
    }
    
    printf("Total checksum: %d\n", total);
    printf("Test completed.\n");
    
    return total != 0 ? 0 : 1;
}
