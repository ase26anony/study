/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate complex control flow and register pressure */
#define USE_VOLATILE(var) do { asm volatile("" : "+r" (var)); } while(0)

/* Large test function with high register pressure and complex CFG */
static int __attribute__((noinline)) test_mcf_pressure(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed ^ 0x1234;
    volatile int v4 = seed | 0xABCD;
    volatile int v5 = seed & 0xF0F0;
    volatile int v6 = seed << 2;
    volatile int v7 = seed >> 1;
    volatile int v8 = ~seed;
    volatile int v9 = seed + 100;
    volatile int v10 = seed - 50;
    volatile int v11 = seed * 3;
    volatile int v12 = seed / 2;
    volatile int v13 = seed ^ 0x5678;
    volatile int v14 = seed | 0xDCBA;
    volatile int v15 = seed & 0x0F0F;
    volatile int v16 = seed << 3;
    volatile int v17 = seed >> 2;
    volatile int v18 = seed + 200;
    volatile int v19 = seed - 100;
    volatile int v20 = seed * 4;
    volatile int v21 = seed / 4;
    volatile int v22 = seed ^ 0x9ABC;
    volatile int v23 = seed | 0x4321;
    volatile int v24 = seed & 0x3333;
    volatile int v25 = seed << 1;
    volatile int v26 = seed >> 3;
    volatile int v27 = seed + 300;
    volatile int v28 = seed - 150;
    volatile int v29 = seed * 5;
    
    /* Mix different types to increase pressure */
    volatile float f0 = seed * 1.5f;
    volatile float f1 = seed / 2.0f;
    volatile void* p0 = &v0;
    volatile void* p1 = &v1;
    
    int result = 0;
    int i, j;
    
    /* Complex loop with many basic blocks */
    for (i = 0; i < 100; i++) {
        /* Create data dependencies between variables */
        v0 = v1 + v2;
        v1 = v3 - v4;
        v2 = v5 * v6;
        v3 = v7 / (v8 + 1);
        v4 = v9 ^ v10;
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = v15 << (v16 & 3);
        v8 = v17 >> (v18 & 3);
        
        /* Force register clobbering with inline asm */
        asm volatile(
            "# Force register pressure\n\t"
            "mov %0, %0\n\t"
            :
            : "r" (v0), "r" (v1), "r" (v2), "r" (v3), 
              "r" (v4), "r" (v5), "r" (v6), "r" (v7)
            : "memory"
        );
        
        /* Complex conditional chain creating many basic blocks */
        if (i % 3 == 0) {
            v9 = v19 + v20;
            v10 = v21 - v22;
            /* Another asm with different clobbers */
            asm volatile(
                "addl $1, %0\n\t"
                : "+r" (v9), "+r" (v10)
                :
                : "cc"
            );
        } else if (i % 3 == 1) {
            v11 = v23 * v24;
            v12 = v25 / (v26 + 1);
            goto label1;  /* Create irregular CFG */
        } else {
            v13 = v27 ^ v28;
            v14 = v29 | v0;
        }
        
        /* Comeback from goto */
        label1:
        
        /* Switch statement with many cases */
        switch (i % 10) {
            case 0: v15 = v1 + v2; break;
            case 1: v16 = v3 - v4; break;
            case 2: v17 = v5 * v6; break;
            case 3: v18 = v7 / (v8 + 1); break;
            case 4: v19 = v9 ^ v10; break;
            case 5: v20 = v11 | v12; break;
            case 6: v21 = v13 & v14; break;
            case 7: v22 = v15 << (v16 & 3); break;
            case 8: v23 = v17 >> (v18 & 3); break;
            case 9: v24 = v19 + v20; break;
            default: v25 = v21 - v22; break;
        }
        
        /* Nested loop with break/continue */
        for (j = 0; j < 10; j++) {
            if (j == 5) {
                v26 = v23 * v24;
                continue;
            }
            if (j == 8) {
                v27 = v25 / (v26 + 1);
                break;
            }
            v28 = v27 ^ v28;
            v29 = v29 | v0;
        }
        
        /* Use float variables to pressure FP registers */
        f0 = f0 * 1.1f + v0;
        f1 = f1 / 1.2f - v1;
        
        /* Pointer arithmetic */
        p0 = (char*)p0 + v2;
        p1 = (char*)p1 - v3;
        
        /* Accumulate result */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                 (int)f0 + (int)f1 + ((int)p0 & 0xFF) + ((int)p1 & 0xFF);
        
        /* Another goto to create more CFG complexity */
        if (i % 7 == 0) {
            goto label2;
        }
        
        v0 = v1;
        v1 = v2;
        
        label2:
        v2 = v3;
    }
    
    /* Final computation with all variables */
    result = result ^ v0 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^
             v10 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^
             v20 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29;
    
    return result;
}

/* Another complex function to ensure MCF runs multiple times */
static int __attribute__((noinline)) test_mcf_alternative(int seed) {
    volatile int a = seed, b = seed * 2, c = seed * 3;
    volatile int d, e, f, g, h, i, j, k, l, m;
    int result = 0;
    
    /* Different pattern of control flow */
    switch (seed % 8) {
        case 0: d = a + b; e = c - a; break;
        case 1: d = b * c; e = a / (b + 1); break;
        case 2: d = a ^ b; e = c | a; break;
        case 3: d = b << 2; e = c >> 1; break;
        case 4: d = ~a; e = ~b; break;
        case 5: d = a + c; e = b - c; break;
        case 6: d = a * b; e = c / (a + 1); break;
        case 7: d = b ^ c; e = a | b; break;
    }
    
    /* Loop with irregular exits */
    for (f = 0; f < 50; f++) {
        if (f % 11 == 0) {
            g = d + e;
            asm volatile("# Clobber more registers" ::: "eax", "ebx", "ecx", "edx");
        } else if (f % 13 == 0) {
            h = d - e;
            goto alt_label;
        } else {
            i = d * e;
        }
        
        alt_label:
        j = g + h + i;
        k = j ^ d;
        l = k | e;
        m = l & a;
        
        result += m;
        
        /* Force spill/reload */
        asm volatile("" : "+r" (d), "+r" (e), "+r" (g), "+r" (h), 
                          "+r" (i), "+r" (j), "+r" (k), "+r" (l), "+r" (m));
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int iter = 0; iter < 10; iter++) {
        total += test_mcf_pressure(iter * 100);
        total += test_mcf_alternative(iter * 50 + 123);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
