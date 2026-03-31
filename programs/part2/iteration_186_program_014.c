/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) test_mcf(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed % 7;
    int v4 = seed ^ 0x1234;
    int v5 = seed | 0x5678;
    int v6 = seed & 0x9ABC;
    int v7 = ~seed;
    int v8 = seed << 2;
    int v9 = seed >> 1;
    int v10 = v0 + v1;
    int v11 = v2 - v3;
    int v12 = v4 * v5;
    int v13 = v6 / (v7 ? v7 : 1);
    int v14 = v8 ^ v9;
    int v15 = v10 | v11;
    int v16 = v12 & v13;
    int v17 = v14 + v15;
    int v18 = v16 - v17;
    int v19 = v18 * v0;
    int v20 = v19 / (v1 ? v1 : 1);
    int v21 = v20 ^ v2;
    int v22 = v21 | v3;
    int v23 = v22 & v4;
    int v24 = v23 + v5;
    int v25 = v24 - v6;
    int v26 = v25 * v7;
    int v27 = v26 / (v8 ? v8 : 1);
    int v28 = v27 ^ v9;
    int v29 = v28 | v10;
    
    /* Complex control flow with many basic blocks */
    int result = 0;
    
    /* First level of conditionals */
    if (v0 > 100) {
        result += v1;
        if (v2 < 50) {
            result += v3;
            goto label_a;
        } else {
            result -= v4;
            if (v5 == 0) {
                result *= 2;
            }
        }
    } else {
        result = v6;
    }
    
    /* Use inline assembly to clobber registers and force graph transformations */
    asm volatile (
        "# Force register clobbering\n"
        "mov %0, %0\n"
        :
        : "r" (result)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Switch statement creates many basic blocks */
    switch (seed % 13) {
        case 0: result += v7; break;
        case 1: result += v8; break;
        case 2: result += v9; break;
        case 3: result += v10; break;
        case 4: result += v11; break;
        case 5: result += v12; break;
        case 6: result += v13; break;
        case 7: result += v14; break;
        case 8: result += v15; break;
        case 9: result += v16; break;
        case 10: result += v17; break;
        case 11: result += v18; break;
        case 12: result += v19; break;
        default: result = 0;
    }
    
    /* Loop with complex exit conditions */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            result += v20;
            if (result > 1000) break;
        } else if (i % 3 == 1) {
            result -= v21;
            if (result < 0) continue;
        } else {
            result ^= v22;
        }
        
        /* Nested loop */
        for (int j = 0; j < 5; j++) {
            result += v23 + j;
            if (j == 3) goto inner_break;
        }
        inner_break:
        
        /* More register pressure in loop */
        v24 = v24 * 3 + 1;
        v25 = v25 / 2 + v24;
    }
    
label_a:
    /* Another level of complex conditionals */
    if (v26 != 0) {
        if (v27 > v28) {
            result += v29;
        } else {
            result -= v0;
        }
    }
    
    /* More inline assembly with different clobbers */
    asm volatile (
        "# More register pressure\n"
        "add %0, %1\n"
        : "+r" (result)
        : "r" (v1)
        : "cc", "memory"
    );
    
    /* Deep if-else chain */
    if (v2 < 10) result += 1;
    else if (v2 < 20) result += 2;
    else if (v2 < 30) result += 3;
    else if (v2 < 40) result += 4;
    else if (v2 < 50) result += 5;
    else if (v2 < 60) result += 6;
    else if (v2 < 70) result += 7;
    else if (v2 < 80) result += 8;
    else if (v2 < 90) result += 9;
    else result += 10;
    
    /* Final computation using all variables */
    result = result + v0 - v1 + v2 - v3 + v4 - v5 + v6 - v7 + v8 - v9 +
             v10 - v11 + v12 - v13 + v14 - v15 + v16 - v17 + v18 - v19 +
             v20 - v21 + v22 - v23 + v24 - v25 + v26 - v27 + v28 - v29;
    
    /* Prevent dead code elimination */
    USE(v0); USE(v1); USE(v2); USE(v3); USE(v4); USE(v5);
    USE(v6); USE(v7); USE(v8); USE(v9); USE(v10); USE(v11);
    USE(v12); USE(v13); USE(v14); USE(v15); USE(v16); USE(v17);
    USE(v18); USE(v19); USE(v20); USE(v21); USE(v22); USE(v23);
    USE(v24); USE(v25); USE(v26); USE(v27); USE(v28); USE(v29);
    
    return result;
}

/* Second complex function to create interprocedural effects */
static int __attribute__((noinline)) helper_func(int x, int y) {
    volatile int a = x;
    volatile int b = y;
    int c = 0;
    
    /* Complex control flow */
    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            c += a;
            a = a * 2 + 1;
        } else {
            c -= b;
            b = b / 2 + 1;
        }
        
        /* Early exit with goto */
        if (c > 10000) goto early_exit;
        if (c < -10000) goto early_exit;
    }
    
early_exit:
    /* Mix float and int operations for register class pressure */
    float f = (float)c;
    f = f * 1.5f;
    c = (int)f;
    
    /* Another inline assembly */
    asm volatile (
        "# Helper function clobber\n"
        : 
        : "r" (c)
        : "xmm0", "xmm1", "xmm2", "memory"
    );
    
    return c;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        int r1 = test_mcf(i);
        int r2 = helper_func(i, i * 2);
        total += r1 + r2;
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
