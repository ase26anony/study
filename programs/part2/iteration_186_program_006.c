/* mcf_test.c - Test program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with high register pressure and control flow */
static int complex_mcf_function(int seed) {
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
        f1 = f2 + 0.5f;
        f2 = f3 - 0.3f;
        f3 = f4 * 2.0f;
        f4 = f0 / 1.5f;
        
        /* Pointer operations */
        *p0 = *p1 + *p2;
        *p1 = *p2 * *p3;
        p0 = (i % 2) ? &v16 : &v17;
        p1 = (i % 3) ? &v18 : &v19;
        
        /* Inline assembly with register clobbers to force graph transformations */
        /* For x86 architecture */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (v16)
            : "r" (v17)
            : "%eax", "%ebx", "%ecx", "%edx", "memory"
        );
        
        /* Complex conditional branching creating many basic blocks */
        if (i % 7 == 0) {
            v17 = v18 + v19;
            goto label_a;
        } else if (i % 7 == 1) {
            v18 = v19 * v20;
            goto label_b;
        } else if (i % 7 == 2) {
            v19 = v20 - v21;
            goto label_c;
        } else if (i % 7 == 3) {
            v20 = v21 ^ v22;
            goto label_d;
        } else if (i % 7 == 4) {
            v21 = v22 | v23;
            goto label_e;
        } else if (i % 7 == 5) {
            v22 = v23 & v24;
            goto label_f;
        } else {
            v23 = v24 << 1;
            goto label_g;
        }
        
    label_a:
        v24 = v25 + 1;
        goto merge_point;
    label_b:
        v25 = v26 * 2;
        goto merge_point;
    label_c:
        v26 = v27 - 3;
        goto merge_point;
    label_d:
        v27 = v28 ^ 0xFF;
        goto merge_point;
    label_e:
        v28 = v29 | 0x0F;
        goto merge_point;
    label_f:
        v29 = v0 & 0xF0;
        goto merge_point;
    label_g:
        v0 = v1 << 2;
        /* fall through to merge_point */
        
    merge_point:
        /* Another inline assembly with different clobbers */
        asm volatile (
            "movl %0, %%esi\n\t"
            "imull %1, %%esi\n\t"
            "movl %%esi, %0\n\t"
            : "+r" (v1)
            : "r" (v2)
            : "%esi", "%edi", "%ebp", "memory"
        );
        
        /* Switch statement with many cases creating control flow */
        switch (i % 13) {
            case 0: v2 = v3 + v4; break;
            case 1: v3 = v4 * v5; break;
            case 2: v4 = v5 - v6; break;
            case 3: v5 = v6 ^ v7; break;
            case 4: v6 = v7 | v8; break;
            case 5: v7 = v8 & v9; break;
            case 6: v8 = v9 << 1; break;
            case 7: v9 = v10 >> 2; break;
            case 8: v10 = v11 + v12; break;
            case 9: v11 = v12 * v13; break;
            case 10: v12 = v13 - v14; break;
            case 11: v13 = v14 ^ v15; break;
            case 12: v14 = v15 | v16; break;
        }
        
        /* Nested loops for additional complexity */
        for (j = 0; j < 5; j++) {
            v15 = v16 + j;
            v16 = v17 * (j + 1);
            if (j % 2 == 0) {
                v17 = v18 - j;
                continue;
            } else {
                v18 = v19 ^ j;
                break;
            }
        }
        
        /* Accumulate result with all variables */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                 (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    }
    
    return result;
}

/* Another complex function to prevent interprocedural optimization */
static int another_complex_function(int base) {
    volatile int a = base, b = base + 100, c = base + 200;
    volatile int d, e, f, g, h;
    
    /* Complex control flow with gotos */
    if (base % 2 == 0) {
        d = a * b;
        goto compute_e;
    } else {
        d = b / a;
        goto compute_f;
    }
    
compute_e:
    e = d + c;
    if (e > 1000) {
        f = e - 500;
        goto final_compute;
    } else {
        f = e + 500;
        goto final_compute;
    }
    
compute_f:
    e = d - c;
    if (e < 0) {
        f = -e;
        goto final_compute;
    } else {
        f = e * 2;
        goto final_compute;
    }
    
final_compute:
    g = f * 3;
    h = g / 2;
    
    /* More inline assembly */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %1\n\t"
        : "=r" (g)
        : "0" (g)
        : "%eax", "memory"
    );
    
    return a + b + c + d + e + f + g + h;
}

int main(void) {
    int total = 0;
    int i;
    
    printf("Starting MCF test...\n");
    
    /* Call complex functions multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        total += complex_mcf_function(i * 100);
        total += another_complex_function(i * 50);
        
        /* Prevent loop unrolling */
        asm volatile ("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    printf("Test completed.\n");
    
    return 0;
}
