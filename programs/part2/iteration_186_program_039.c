/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations from simplifying our complex control flow */
#define NOOPT __attribute__((optimize("O0")))
#define VOLATILE_VAR volatile

/* Large test function with maximum register pressure */
NOOPT
static unsigned long test_mcf_pressure(int seed) {
    /* Declare 30+ local variables to create register pressure */
    VOLATILE_VAR int v0 = seed;
    VOLATILE_VAR int v1 = seed * 2;
    VOLATILE_VAR int v2 = seed + 1;
    VOLATILE_VAR int v3 = seed - 1;
    VOLATILE_VAR int v4 = seed ^ 0x55;
    VOLATILE_VAR int v5 = seed | 0xAA;
    VOLATILE_VAR int v6 = seed & 0xFF;
    VOLATILE_VAR int v7 = seed << 2;
    VOLATILE_VAR int v8 = seed >> 1;
    VOLATILE_VAR int v9 = ~seed;
    VOLATILE_VAR int v10 = seed * 3;
    VOLATILE_VAR int v11 = seed / 2;
    VOLATILE_VAR int v12 = seed % 7;
    VOLATILE_VAR int v13 = seed + 100;
    VOLATILE_VAR int v14 = seed - 50;
    VOLATILE_VAR int v15 = seed * seed;
    VOLATILE_VAR int v16 = seed | 0xF0F0;
    VOLATILE_VAR int v17 = seed & 0x0F0F;
    VOLATILE_VAR int v18 = seed ^ 0x1234;
    VOLATILE_VAR int v19 = seed + 0xABCD;
    VOLATILE_VAR int v20 = 0;
    VOLATILE_VAR int v21 = 0;
    VOLATILE_VAR int v22 = 0;
    VOLATILE_VAR int v23 = 0;
    VOLATILE_VAR int v24 = 0;
    VOLATILE_VAR int v25 = 0;
    VOLATILE_VAR int v26 = 0;
    VOLATILE_VAR int v27 = 0;
    VOLATILE_VAR int v28 = 0;
    VOLATILE_VAR int v29 = 0;
    
    /* Mix different types to increase pressure on different register classes */
    VOLATILE_VAR float f0 = seed * 0.5f;
    VOLATILE_VAR float f1 = seed * 1.5f;
    VOLATILE_VAR double d0 = seed * 0.25;
    VOLATILE_VAR double d1 = seed * 0.75;
    VOLATILE_VAR void* p0 = &v0;
    VOLATILE_VAR void* p1 = &v1;
    
    /* Complex control flow with many basic blocks */
    unsigned long result = 0;
    int i, j;
    
    /* Outer loop creating many basic blocks */
    for (i = 0; i < 100; i++) {
        /* Deep chain of dependent operations */
        v20 = v0 + v1;
        v21 = v20 * v2;
        v22 = v21 - v3;
        v23 = v22 ^ v4;
        v24 = v23 | v5;
        v25 = v24 & v6;
        v26 = v25 << (v7 & 3);
        v27 = v26 >> (v8 & 3);
        v28 = v27 + v9;
        v29 = v28 * v10;
        
        /* Inline assembly with register clobbers to force graph transformations */
        /* For x86 */
        asm volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (v20)
            : "r" (v21)
            : "%eax", "%ebx", "cc"
        );
        
        /* More assembly with different clobbers */
        asm volatile (
            "movl %0, %%ecx\n\t"
            "movl %1, %%edx\n\t"
            "imull %%edx, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "+r" (v22)
            : "r" (v23)
            : "%ecx", "%edx", "cc"
        );
        
        /* Complex conditional chain creating many basic blocks */
        if (i % 3 == 0) {
            v0 = v29 + v20;
            if (v0 > 1000) {
                v1 = v0 / 2;
                goto label1;
            } else {
                v1 = v0 * 2;
                goto label2;
            }
        } else if (i % 3 == 1) {
            v2 = v29 - v20;
            if (v2 < 0) {
                v3 = -v2;
                /* Another inline asm with clobbers */
                asm volatile (
                    "movl %0, %%esi\n\t"
                    "negl %%esi\n\t"
                    "movl %%esi, %0\n\t"
                    : "+r" (v3)
                    :: "%esi", "cc"
                );
            } else {
                v3 = v2;
            }
        } else {
            v4 = v29 ^ v20;
            /* Force spill/reload by using all variables */
            v5 = v4 + v0 + v1 + v2 + v3 + v6 + v7 + v8 + v9;
        }
        
        /* Switch statement with many cases for more basic blocks */
        switch (i % 13) {
            case 0: v6 = v5 + 1; break;
            case 1: v6 = v5 - 1; break;
            case 2: v6 = v5 * 2; break;
            case 3: v6 = v5 / 2; break;
            case 4: v6 = v5 & 0xFF; break;
            case 5: v6 = v5 | 0xAA; break;
            case 6: v6 = v5 ^ 0x55; break;
            case 7: v6 = v5 << 1; break;
            case 8: v6 = v5 >> 1; break;
            case 9: v6 = ~v5; break;
            case 10: v6 = v5 + v0; break;
            case 11: v6 = v5 - v1; break;
            case 12: v6 = v5 * v2; break;
            default: v6 = 0; break;
        }
        
        /* Nested loops for additional control flow complexity */
        for (j = 0; j < 5; j++) {
            if (j % 2 == 0) {
                v7 += v6;
                if (v7 > 10000) {
                    v7 = 0;
                    break;  /* Early exit from inner loop */
                }
            } else {
                v8 -= v6;
                if (v8 < 0) {
                    v8 = 100;
                    continue;  /* Skip to next iteration */
                }
            }
            v9 = v7 * v8;
        }
        
        /* Use float/double variables to pressure FP registers */
        f0 = v9 * 0.5f;
        f1 = f0 + 1.0f;
        d0 = v9 * 0.25;
        d1 = d0 + 1.0;
        
        /* Pointer arithmetic */
        p0 = (char*)p0 + v9;
        p1 = (char*)p1 - v9;
        
        /* Accumulate result */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        result += (int)f0 + (int)f1 + (int)d0 + (int)d1;
        result += (unsigned long)p0 + (unsigned long)p1;
        
        continue;  /* Explicit continue for control flow edge */
        
    label1:
        v10 = v1 * 3;
        continue;
        
    label2:
        v10 = v1 / 3;
        /* Fall through */
    }
    
    /* Final complex computation using all variables */
    unsigned long final = result;
    final += v0 * 2;
    final += v1 * 3;
    final += v2 * 4;
    final += v3 * 5;
    final += v4 * 6;
    final += v5 * 7;
    final += v6 * 8;
    final += v7 * 9;
    final += v8 * 10;
    final += v9 * 11;
    final += v10 * 12;
    final += v11 * 13;
    final += v12 * 14;
    final += v13 * 15;
    final += v14 * 16;
    final += v15 * 17;
    final += v16 * 18;
    final += v17 * 19;
    final += v18 * 20;
    final += v19 * 21;
    final += v20 * 22;
    final += v21 * 23;
    final += v22 * 24;
    final += v23 * 25;
    final += v24 * 26;
    final += v25 * 27;
    final += v26 * 28;
    final += v27 * 29;
    final += v28 * 30;
    final += v29 * 31;
    
    return final;
}

/* Second test function with different patterns */
NOOPT
static unsigned long test_mcf_pattern2(int seed) {
    VOLATILE_VAR int a = seed, b = seed + 1, c = seed + 2;
    VOLATILE_VAR int d, e, f, g, h, i, j, k, l, m;
    unsigned long result = 0;
    
    /* Different control flow pattern */
    for (int x = 0; x < 50; x++) {
        switch (x % 11) {
            case 0: d = a + b; break;
            case 1: d = a - b; break;
            case 2: d = a * b; break;
            case 3: d = a / (b ? b : 1); break;
            case 4: d = a & b; break;
            case 5: d = a | b; break;
            case 6: d = a ^ b; break;
            case 7: d = a << (b & 3); break;
            case 8: d = a >> (b & 3); break;
            case 9: d = ~a; break;
            case 10: d = a + c; break;
        }
        
        /* More inline asm with clobbers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (e)
            : "r" (d)
            : "%eax", "cc"
        );
        
        /* Complex conditional */
        if (e > 1000) {
            f = e / 2;
            g = f * 3;
        } else if (e > 500) {
            f = e / 3;
            g = f * 4;
        } else if (e > 100) {
            f = e / 4;
            g = f * 5;
        } else {
            f = e * 2;
            g = f + 1;
        }
        
        /* Loop with break/continue */
        for (int y = 0; y < 10; y++) {
            if (y == g % 10) break;
            if (y % 2 == 0) continue;
            h = g + y;
        }
        
        result += a + b + c + d + e + f + g + h;
        
        /* Rotate values */
        i = a; a = b; b = c; c = i;
    }
    
    return result;
}

int main(void) {
    unsigned long total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_mcf_pressure(i * 17);
        total += test_mcf_pattern2(i * 23);
    }
    
    printf("Result: %lu\n", total);
    return 0;
}
