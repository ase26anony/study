/* mcf_trigger.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with high register pressure and control flow */
static int __attribute__((noinline)) complex_mcf_function(int seed) {
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
    
    volatile int* p0 = &v0, *p1 = &v1, *p2 = &v2;
    
    int result = 0;
    int i, j;
    
    /* Complex loop with data dependencies */
    for (i = 0; i < 100; i++) {
        /* Chain of dependent operations */
        v0 = v1 + v2;
        v1 = v0 * v3;
        v2 = v1 - v4;
        v3 = v2 / (v5 + 1);
        v4 = v3 | v6;
        v5 = v4 ^ v7;
        v6 = v5 & v8;
        v7 = v6 << 2;
        v8 = v7 >> 1;
        v9 = v8 + v10;
        v10 = v9 * v11;
        
        /* Floating point operations to pressure FP registers */
        f0 = f1 * 1.1f;
        f1 = f0 + f2;
        f2 = f1 - f3;
        f3 = f2 * f4;
        f4 = f3 / 2.0f;
        
        /* Pointer arithmetic */
        *p0 = *p1 + *p2;
        p0 = (i % 2) ? &v12 : &v13;
        p1 = (i % 3) ? &v14 : &v15;
        p2 = (i % 4) ? &v16 : &v17;
        
        /* Inline assembly with register clobbering */
        /* For x86 */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (v18)
            : "r" (v19)
            : "%eax", "%ebx", "%ecx", "%edx", "memory"
        );
        
        /* Complex conditional structure creating many basic blocks */
        if (i % 3 == 0) {
            v20 = v21 + v22;
            if (v20 > 1000) {
                v21 = v22 * 2;
                goto label_a;
            } else {
                v22 = v21 / 2;
                goto label_b;
            }
        } else if (i % 3 == 1) {
            v23 = v24 ^ v25;
            if (v23 & 1) {
                v24 = v25 | 0xFF;
            } else {
                v25 = v24 & 0x7F;
            }
        } else {
            v26 = v27 - v28;
            if (v26 < 0) {
                v27 = -v28;
            }
        }
        
        /* Switch statement with many cases */
        switch (i % 10) {
            case 0: v12 = v13 + 1; break;
            case 1: v13 = v14 * 2; break;
            case 2: v14 = v15 - 3; break;
            case 3: v15 = v16 / 4; break;
            case 4: v16 = v17 | 0x0F; break;
            case 5: v17 = v18 ^ 0xAA; break;
            case 6: v18 = v19 & 0x55; break;
            case 7: v19 = v20 << 1; break;
            case 8: v20 = v21 >> 2; break;
            case 9: v21 = v22 + v23; break;
            default: v22 = 0; break;
        }
        
        /* Nested loops with breaks/continues */
        for (j = 0; j < 5; j++) {
            if (j == 2 && (i % 7 == 0)) {
                v28 = v29 * j;
                continue;
            }
            if (j == 3 && (i % 11 == 0)) {
                v29 = v0 + j;
                break;
            }
            v0 += j;
        }
        
        /* More arithmetic to ensure all variables are used */
        v1 += v2 * v3;
        v4 -= v5 / (v6 + 1);
        v7 |= v8 & v9;
        v10 ^= v11 | v12;
        
        /* Compute intermediate result */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        continue; /* Skip the labels for normal flow */
        
    label_a:
        v29 = v0 * 3;
        continue;
        
    label_b:
        v28 = v1 * 4;
        continue;
    }
    
    /* Final computation mixing all variables */
    result = (v0 ^ v1) | (v2 & v3) + (v4 * v5) - (v6 / (v7 + 1)) +
             (v8 << 2) + (v9 >> 1) + (v10 | v11) ^ (v12 & v13) +
             (v14 * v15) - (v16 / (v17 + 1)) + (v18 << 3) + (v19 >> 2) +
             (v20 | v21) ^ (v22 & v23) + (v24 * v25) - (v26 / (v27 + 1)) +
             (v28 << 4) + (v29 >> 3);
    
    return result;
}

/* Another complex function to ensure MCF runs multiple times */
static int __attribute__((noinline)) another_mcf_function(int base) {
    volatile int a = base, b = base + 100, c = base + 200;
    volatile int d, e, f, g, h, i, j, k, l, m;
    volatile int n, o, p, q, r, s, t, u, v, w;
    
    int result = 0;
    
    /* Complex control flow with gotos */
    if (base % 2 == 0) {
        d = a + b;
        goto compute1;
    } else {
        e = b - a;
        goto compute2;
    }
    
compute1:
    f = d * 2;
    g = f + c;
    if (g > 1000) {
        h = g / 2;
        goto compute3;
    } else {
        i = g * 3;
        goto compute4;
    }
    
compute2:
    j = e / 2;
    k = j + c;
    if (k < 500) {
        l = k * 4;
        goto compute5;
    } else {
        m = k / 4;
        goto compute6;
    }
    
compute3:
    n = h + 100;
    result = n;
    goto finish;
    
compute4:
    o = i - 50;
    result = o;
    goto finish;
    
compute5:
    p = l | 0xFF;
    result = p;
    goto finish;
    
compute6:
    q = m & 0x7F;
    result = q;
    goto finish;
    
finish:
    /* Use inline assembly with many clobbered registers */
    asm volatile (
        "movl %0, %%eax\n\t"
        "movl %1, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "+r" (result)
        : "r" (base)
        : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "memory"
    );
    
    return result;
}

int main(void) {
    int total = 0;
    int i;
    
    printf("Starting MCF trigger program...\n");
    
    /* Call complex functions multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        total += complex_mcf_function(i * 100);
        total += another_mcf_function(i * 50);
        
        /* Prevent loop unrolling */
        volatile int anti_opt = i;
        asm volatile ("" : "+r" (anti_opt));
    }
    
    printf("Final result: %d\n", total);
    
    /* Ensure result is used */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
