/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
#define NO_OPTIMIZE(x) asm volatile("" : "+r" (x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) 
complex_mcf_test(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    volatile int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    volatile int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    volatile int v12 = seed + 13, v13 = seed + 14, v14 = seed + 15, v15 = seed + 16;
    volatile int v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    volatile int v20 = seed + 21, v21 = seed + 22, v22 = seed + 23, v23 = seed + 24;
    volatile int v24 = seed + 25, v25 = seed + 26, v26 = seed + 27, v27 = seed + 28;
    volatile int v28 = seed + 29, v29 = seed + 30;
    
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
        v3 = v7 / (v8 ? v8 : 1);
        v4 = v9 ^ v10;
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = v15 << 2;
        v8 = v16 >> 1;
        v9 = v17 + v18;
        v10 = v19 * v20;
        
        /* Floating point operations to pressure FP registers */
        f0 = f1 * f2;
        f1 = f3 + f4;
        f2 = f0 - f1;
        f3 = f2 * 1.1f;
        f4 = f3 / 2.0f;
        
        /* Pointer operations */
        *p0 = *p1 + *p2;
        *p1 = *p3 * i;
        
        /* Inline assembly with register clobbering */
        /* For x86 */
        asm volatile (
            "# Force register pressure\n"
            "mov %0, %%eax\n"
            "mov %1, %%ebx\n"
            "add %%ebx, %%eax\n"
            "mov %%eax, %0\n"
            : "+r" (v21)
            : "r" (v22)
            : "%eax", "%ebx", "cc"
        );
        
        /* More clobbering for different register classes */
        asm volatile (
            "# Clobber more registers\n"
            "mov %0, %%ecx\n"
            "mov %1, %%edx\n"
            "imul %%edx, %%ecx\n"
            "mov %%ecx, %0\n"
            : "+r" (v23)
            : "r" (v24)
            : "%ecx", "%edx", "cc"
        );
        
        /* Complex conditional structure creating many basic blocks */
        switch (i % 15) {
            case 0: v11 = v12 + v13; break;
            case 1: v12 = v13 - v14; break;
            case 2: v13 = v14 * v15; break;
            case 3: v14 = v15 ^ v16; break;
            case 4: v15 = v16 | v17; break;
            case 5: v16 = v17 & v18; break;
            case 6: v17 = v18 << 3; break;
            case 7: v18 = v19 >> 2; break;
            case 8: v19 = v20 + v21; break;
            case 9: v20 = v21 * v22; break;
            case 10: v21 = v22 - v23; break;
            case 11: v22 = v23 ^ v24; break;
            case 12: v23 = v24 | v25; break;
            case 13: v24 = v25 & v26; break;
            case 14: v25 = v26 << 1; break;
            default: v26 = v27 + v28; break;
        }
        
        /* Nested conditionals */
        if (i % 3 == 0) {
            v27 = v28 * v29;
            if (i % 7 == 0) {
                v28 = v29 + v0;
                goto label1;  /* Create irregular control flow */
            } else {
                v29 = v0 - v1;
            }
        } else if (i % 5 == 0) {
            v0 = v1 * v2;
            if (i % 11 == 0) {
                v1 = v2 + v3;
                goto label2;
            }
        } else {
            v2 = v3 - v4;
        }
        
        /* More computations */
        v3 = v4 * v5;
        v4 = v5 + v6;
        
        /* Labels for goto statements creating complex CFG */
        label1:
        v5 = v6 - v7;
        
        label2:
        v6 = v7 * v8;
        
        /* Prevent loop unrolling */
        NO_OPTIMIZE(i);
    }
    
    /* Combine all results */
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
             v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
             v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    /* Use floating point results */
    result += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    
    return result;
}

/* Another complex function to ensure MCF runs multiple times */
static int __attribute__((noinline))
another_complex_function(int base) {
    volatile int a = base, b = base + 1, c = base + 2, d = base + 3;
    volatile int e = base + 4, f = base + 5, g = base + 6, h = base + 7;
    volatile int i = base + 8, j = base + 9, k = base + 10, l = base + 11;
    
    /* Deep if-else chain */
    if (a > 0) {
        b = c * d;
        if (b < 100) {
            c = d + e;
            goto mid_block;
        } else {
            d = e - f;
        }
    } else if (a < -10) {
        e = f * g;
    } else {
        f = g + h;
    }
    
    mid_block:
    for (int x = 0; x < 50; x++) {
        /* More register pressure */
        g = h + i;
        h = i * j;
        i = j - k;
        j = k + l;
        k = l * a;
        
        /* Another inline asm with clobber */
        asm volatile (
            "# Additional clobber\n"
            "mov %0, %%esi\n"
            "mov %1, %%edi\n"
            "add %%edi, %%esi\n"
            "mov %%esi, %0\n"
            : "+r" (l)
            : "r" (a)
            : "%esi", "%edi", "cc"
        );
        
        /* Complex switch */
        switch (x % 8) {
            case 0: a = b + 1; break;
            case 1: b = c - 2; break;
            case 2: c = d * 3; break;
            case 3: d = e ^ 4; break;
            case 4: e = f | 5; break;
            case 5: f = g & 6; break;
            case 6: g = h << 1; break;
            case 7: h = i >> 2; break;
        }
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

int main(void) {
    int total = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Call test functions multiple times with different inputs */
    for (int iter = 0; iter < 10; iter++) {
        int r1 = complex_mcf_test(iter * 100);
        int r2 = another_complex_function(iter * 50);
        
        total += r1 + r2;
        
        /* Prevent optimization across iterations */
        NO_OPTIMIZE(total);
    }
    
    printf("Result: %d\n", total);
    printf("Test completed. Check RTL dump files for MCF debug output.\n");
    
    /* Look for files like:
     * - *.mcf (contains the flow graph with special block names)
     * - *.mcf_details (detailed MCF information)
     */
    
    return total != 0 ? 0 : 1;
}
