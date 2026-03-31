/* mcf_coverage.c - Program to trigger GCC's MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
#define NO_OPTIMIZE(x) asm volatile("" : "+r" (x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) 
test_mcf_pass(int seed, int iterations) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mix different types to increase pressure */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* p0 = &v0, *p1 = &v1;
    
    int result = 0;
    
    /* Complex loop with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Create data dependencies across variables */
        v0 = v1 + v2;
        v1 = v3 - v4;
        v2 = v5 * v6;
        v3 = v7 / (v8 + 1);
        v4 = v9 ^ v10;
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = v15 << 2;
        v8 = v16 >> 1;
        v9 = v17 + v18;
        v10 = v19 - v20;
        
        /* Complex conditional chain creating many basic blocks */
        if (v0 & 1) {
            v11 = v21 * v22;
            if (v1 > v2) {
                v12 = v23 + v24;
                /* Inline assembly with register clobbering */
                asm volatile(
                    "# Force register pressure\n"
                    "mov %0, %0\n"
                    :
                    : "r" (v12)
                    : "eax", "ebx", "ecx", "edx"
                );
            } else {
                v12 = v25 - v26;
                /* Another clobber to force graph modifications */
                asm volatile(
                    "# More register pressure\n"
                    :
                    :
                    : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
                );
            }
        } else if (v3 < v4) {
            v11 = v27 | v28;
            /* Force floating point operations */
            f0 = f1 * 2.0f;
            d0 = d1 / 3.0;
        } else {
            v11 = v29 ^ v0;
            /* Pointer arithmetic */
            p0 = (char*)p1 + v11;
        }
        
        /* Switch statement with many cases creating control flow */
        switch (i & 0xF) {
            case 0: v13 = v0 + v1; break;
            case 1: v13 = v2 - v3; break;
            case 2: v13 = v4 * v5; break;
            case 3: v13 = v6 / (v7 + 1); break;
            case 4: v13 = v8 ^ v9; break;
            case 5: v13 = v10 | v11; break;
            case 6: v13 = v12 & v13; break;
            case 7: v13 = v14 << (i & 3); break;
            case 8: v13 = v15 >> (i & 3); break;
            case 9: v13 = v16 + v17; break;
            case 10: v13 = v18 - v19; break;
            case 11: v13 = v20 * v21; break;
            case 12: v13 = v22 / (v23 + 1); break;
            case 13: v13 = v24 ^ v25; break;
            case 14: v13 = v26 | v27; break;
            case 15: v13 = v28 & v29; break;
        }
        
        /* Nested loops with breaks/continues */
        for (int j = 0; j < 3; j++) {
            if (v13 & (1 << j)) {
                v14 += v15;
                if (j == 1) continue;
                v15 -= v16;
            } else {
                v16 *= v17;
                if (j == 2) break;
                v17 /= (v18 + 1);
            }
        }
        
        /* Goto labels creating additional control flow edges */
        if (v14 > 1000) goto reduce_values;
        if (v14 < -1000) goto increase_values;
        
        /* Normal flow continues */
        v18 = v19 ^ v20;
        goto next_iteration;
        
    reduce_values:
        v14 = v14 / 2;
        v15 = v15 / 2;
        goto next_iteration;
        
    increase_values:
        v14 = v14 * 2;
        v15 = v15 * 2;
        
    next_iteration:
        /* Complex computation mixing all variables */
        v19 = (v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18) & 0xFFF;
        
        v20 = (v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29) & 0xFFF;
        
        /* Prevent dead code elimination */
        NO_OPTIMIZE(v0); NO_OPTIMIZE(v1); NO_OPTIMIZE(v2);
        NO_OPTIMIZE(v3); NO_OPTIMIZE(v4); NO_OPTIMIZE(v5);
        NO_OPTIMIZE(v6); NO_OPTIMIZE(v7); NO_OPTIMIZE(v8);
        NO_OPTIMIZE(v9); NO_OPTIMIZE(v10); NO_OPTIMIZE(v11);
        NO_OPTIMIZE(v12); NO_OPTIMIZE(v13); NO_OPTIMIZE(v14);
        NO_OPTIMIZE(v15); NO_OPTIMIZE(v16); NO_OPTIMIZE(v17);
        NO_OPTIMIZE(v18); NO_OPTIMIZE(v19); NO_OPTIMIZE(v20);
        NO_OPTIMIZE(v21); NO_OPTIMIZE(v22); NO_OPTIMIZE(v23);
        NO_OPTIMIZE(v24); NO_OPTIMIZE(v25); NO_OPTIMIZE(v26);
        NO_OPTIMIZE(v27); NO_OPTIMIZE(v28); NO_OPTIMIZE(v29);
        NO_OPTIMIZE(f0); NO_OPTIMIZE(f1);
        NO_OPTIMIZE(d0); NO_OPTIMIZE(d1);
        NO_OPTIMIZE(p0); NO_OPTIMIZE(p1);
        
        result += v19 + v20;
    }
    
    return result;
}

/* Another complex function to ensure MCF runs multiple times */
static int __attribute__((noinline))
another_complex_function(int x) {
    volatile int a = x, b = x * 2, c = x * 3, d = x * 4;
    volatile int e = x * 5, f = x * 6, g = x * 7, h = x * 8;
    
    /* Complex conditional structure */
    if (x > 0) {
        for (int i = 0; i < 10; i++) {
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            
            /* Inline assembly with clobbers */
            asm volatile(
                "# Force graph transformations\n"
                :
                :
                : "memory", "cc", "rax", "rbx", "rcx", "rdx", 
                  "rsi", "rdi", "r8", "r9", "r10", "r11", "r12"
            );
            
            if (i & 1) {
                e = f + g;
                f = g + h;
            } else {
                g = h + a;
                h = a + b;
            }
        }
    } else {
        /* Different path with switch */
        switch (x & 7) {
            case 0: a = b * c; break;
            case 1: b = c * d; break;
            case 2: c = d * e; break;
            case 3: d = e * f; break;
            case 4: e = f * g; break;
            case 5: f = g * h; break;
            case 6: g = h * a; break;
            case 7: h = a * b; break;
        }
    }
    
    return a + b + c + d + e + f + g + h;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_mcf_pass(i, 5);
        total += another_complex_function(i - 5);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
