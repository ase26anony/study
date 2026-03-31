/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate complex flow graph with register pressure */
__attribute__((noinline))
static unsigned long long test_mcf_graph(int seed) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    volatile int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    volatile int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    volatile int v12 = seed + 13, v13 = seed + 14, v14 = seed + 15, v15 = seed + 16;
    volatile int v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    volatile int v20 = seed + 21, v21 = seed + 22, v22 = seed + 23, v23 = seed + 24;
    volatile int v24 = seed + 25, v25 = seed + 26, v26 = seed + 27, v27 = seed + 28;
    volatile int v28 = seed + 29, v29 = seed + 30;
    
    /* Mix different types to pressure different register classes */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2;
    
    unsigned long long result = 0;
    int i, j;
    
    /* Complex loop with many basic blocks */
    for (i = 0; i < 100; i++) {
        /* Create data dependencies across variables */
        v0 = v1 + v2;
        v1 = v3 ^ v4;
        v2 = v5 * v6;
        v3 = v7 | v8;
        v4 = v9 & v10;
        
        /* Inline assembly with register clobbers to force graph transformations */
        asm volatile (
            "# Force register pressure\n\t"
            : "=r"(v5), "=r"(v6), "=r"(v7)
            : "0"(v0), "1"(v1), "2"(v2)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Complex conditional chain creating many basic blocks */
        if (i % 3 == 0) {
            v8 = v11 + v12;
            v9 = v13 - v14;
            
            /* Nested conditionals */
            if (v8 > v9) {
                v10 = v15 * v16;
                asm volatile ("# Conditional block 1" : : : "esi", "edi");
            } else {
                v10 = v17 / (v18 ? v18 : 1);
                goto merge_point1;
            }
            
            v11 = v19 % (v20 ? v20 : 1);
            merge_point1:
            v12 = v21 << 2;
        } else if (i % 3 == 1) {
            v13 = v22 >> 1;
            v14 = ~v23;
            
            /* Another inline asm with different clobbers */
            asm volatile (
                "# More register pressure\n\t"
                : "+r"(v15), "+r"(v16)
                :
                : "r8", "r9", "r10", "r11", "cc"
            );
        } else {
            v17 = v24 | v25;
            v18 = v26 & v27;
            
            /* Deeply nested conditional */
            if (v17 & 1) {
                if (v18 & 2) {
                    v19 = v28 ^ v29;
                    asm volatile ("# Deep nested" : : : "xmm0", "xmm1");
                } else {
                    v19 = v0 | v1;
                }
            }
        }
        
        /* Switch statement with many cases - creates multiple exit points */
        switch (i % 10) {
            case 0: v20 = v2 + v3; f0 += 0.1f; break;
            case 1: v21 = v4 - v5; f1 *= 1.1f; break;
            case 2: v22 = v6 * v7; d0 += 0.01; break;
            case 3: v23 = v8 ^ v9; d1 *= 1.01; break;
            case 4: v24 = v10 | v11; p0 = &v20; break;
            case 5: v25 = v12 & v13; p1 = &v21; break;
            case 6: v26 = v14 << 1; p2 = &v22; break;
            case 7: v27 = v15 >> 2; f2 -= 0.1f; break;
            case 8: v28 = v16 + v17; 
                    /* Artificial goto to create irregular control flow */
                    if (v28 > 1000) goto early_exit;
                    break;
            case 9: v29 = v18 * v19; 
                    /* Another asm with many clobbers */
                    asm volatile (
                        "# Case 9 clobber\n\t"
                        : "+r"(v29)
                        : 
                        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
                    );
                    break;
        }
        
        /* Loop with break/continue creating more control flow */
        for (j = 0; j < 5; j++) {
            if (j == 2 && (i % 7 == 0)) {
                v0 = v1 + v2;
                continue;
            }
            if (j == 3 && (i % 11 == 0)) {
                v3 = v4 * v5;
                break;
            }
            v6 = v7 + v8 + j;
        }
        
        /* Mix integer and float operations */
        f0 = (v0 % 100) * 0.01f;
        f1 = (v1 % 200) * 0.005f;
        d0 = (v2 % 300) * 0.00333;
        
        /* Pointer arithmetic creating aliasing concerns */
        *(int*)p0 = v3;
        *(int*)p1 = v4;
        *(int*)p2 = v5;
        
        /* Accumulate result with complex expression */
        result += (v0 ^ v1) | (v2 & v3) | ((unsigned long long)v4 << 16);
        result += (unsigned long long)(f0 * 1000) + (unsigned long long)(d0 * 1000);
        
        early_exit:
        /* Empty label for goto target */
        if (i == 50 && result > 1000000) {
            /* Early return path */
            return result ^ 0xABCDEF;
        }
    }
    
    /* Final computation mixing all variables */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    result += (unsigned long long)(f0 * 100) + (unsigned long long)(f1 * 100);
    result += (unsigned long long)(d0 * 100) + (unsigned long long)(d1 * 100);
    result += (unsigned long long)p0 + (unsigned long long)p1 + (unsigned long long)p2;
    
    return result;
}

/* Second complex function to increase overall compilation complexity */
__attribute__((noinline))
static unsigned long long another_complex_function(int base) {
    volatile int a = base, b = base + 1, c = base + 2;
    volatile int d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u;
    unsigned long long sum = 0;
    
    for (int x = 0; x < 50; x++) {
        /* Chain of dependent operations */
        d = a + b;
        e = c * d;
        f = e ^ a;
        g = f | b;
        h = g & c;
        
        /* More inline asm */
        asm volatile (
            "# Second function asm\n\t"
            : "+r"(i), "+r"(j), "+r"(k)
            : "r"(d), "r"(e), "r"(f)
            : "r12", "r13", "r14", "r15", "xmm6", "xmm7", "xmm8", "xmm9"
        );
        
        /* Complex conditional */
        if (x % 4 == 0) {
            l = h << (x % 8);
            m = i >> (x % 4);
            if (l > m) goto label_a;
        } else if (x % 4 == 1) {
            n = j + k;
            o = n - i;
            goto label_b;
        } else {
            p = k * j;
            q = p / (i ? i : 1);
        }
        
        label_a:
        r = l + m;
        label_b:
        s = n + o;
        t = p + q;
        u = r + s + t;
        
        sum += u + a + b + c;
        
        /* Rotate values */
        a = b;
        b = c;
        c = u & 0xFF;
    }
    
    return sum;
}

int main(void) {
    unsigned long long total = 0;
    int iterations = 10;
    
    printf("Starting MCF coverage test...\n");
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < iterations; i++) {
        total += test_mcf_graph(i * 7);
        total += another_complex_function(i * 13);
        
        /* Prevent loop unrolling from simplifying too much */
        asm volatile ("# Main loop barrier" : : : "memory");
    }
    
    printf("Result: %llu\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
        return 1;
    }
    
    return 0;
}
