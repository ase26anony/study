/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) test_mcf(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed ^ 0x1234;
    volatile int v4 = seed | 0xABCD;
    volatile int v5 = seed & 0xF0F0;
    volatile int v6 = seed << 3;
    volatile int v7 = seed >> 2;
    volatile int v8 = seed + v0;
    volatile int v9 = v1 * v2;
    volatile int v10 = v3 ^ v4;
    volatile int v11 = v5 | v6;
    volatile int v12 = v7 & v8;
    volatile int v13 = v9 + v10;
    volatile int v14 = v11 * v12;
    volatile int v15 = v13 ^ v14;
    volatile int v16 = v0 | v15;
    volatile int v17 = v1 & v16;
    volatile int v18 = v2 + v17;
    volatile int v19 = v3 * v18;
    volatile int v20 = v4 ^ v19;
    volatile int v21 = v5 | v20;
    volatile int v22 = v6 & v21;
    volatile int v23 = v7 + v22;
    volatile int v24 = v8 * v23;
    volatile int v25 = v9 ^ v24;
    volatile int v26 = v10 | v25;
    volatile int v27 = v11 & v26;
    volatile int v28 = v12 + v27;
    volatile int v29 = v13 * v28;
    
    int result = 0;
    
    /* Complex control flow with many basic blocks */
    for (int i = 0; i < 100; i++) {
        /* Inline assembly to clobber registers and force graph transformations */
        asm volatile(
            "# Force register clobbering\n"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Chain of dependent operations */
        v0 = v1 + i;
        v1 = v2 * v0;
        v2 = v3 ^ v1;
        v3 = v4 | v2;
        v4 = v5 & v3;
        v5 = v6 + v4;
        v6 = v7 * v5;
        v7 = v8 ^ v6;
        v8 = v9 | v7;
        v9 = v10 & v8;
        
        /* Complex conditional structure creating many basic blocks */
        switch (i % 13) {
            case 0:
                v10 = v11 + v9;
                result += v10;
                break;
            case 1:
                v11 = v12 * v10;
                result ^= v11;
                break;
            case 2:
                v12 = v13 ^ v11;
                result |= v12;
                break;
            case 3:
                v13 = v14 | v12;
                result &= v13;
                break;
            case 4:
                v14 = v15 & v13;
                result += v14;
                break;
            case 5:
                v15 = v16 + v14;
                result ^= v15;
                break;
            case 6:
                v16 = v17 * v15;
                result |= v16;
                break;
            case 7:
                v17 = v18 ^ v16;
                result &= v17;
                break;
            case 8:
                v18 = v19 | v17;
                result += v18;
                break;
            case 9:
                v19 = v20 & v18;
                result ^= v19;
                break;
            case 10:
                v20 = v21 + v19;
                result |= v20;
                break;
            case 11:
                v21 = v22 * v20;
                result &= v21;
                break;
            case 12:
                v22 = v23 ^ v21;
                result += v22;
                /* Fall through to create additional control flow edges */
            default:
                v23 = v24 | v22;
                result ^= v23;
        }
        
        /* Nested conditionals */
        if (i & 1) {
            if (i & 2) {
                v24 = v25 + v23;
                result += v24;
            } else {
                v25 = v26 * v24;
                result ^= v25;
            }
        } else {
            if (i & 4) {
                v26 = v27 ^ v25;
                result |= v26;
            } else {
                v27 = v28 | v26;
                result &= v27;
            }
        }
        
        /* More register pressure with floating point */
        {
            volatile float f0 = v0 * 1.5f;
            volatile float f1 = v1 * 2.5f;
            volatile float f2 = v2 * 3.5f;
            volatile float f3 = v3 * 4.5f;
            
            /* Force use of floating point registers */
            asm volatile(
                "# Clobber floating point registers\n"
                : 
                : 
                : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
            );
            
            /* Mix integer and float operations */
            v28 = (int)(f0 + f1 + f2 + f3);
            result += v28;
        }
        
        /* Loop with break/continue creating exit-like regions */
        for (int j = 0; j < 5; j++) {
            if (j == 3 && (i % 7 == 0)) {
                v29 = v0 * v28;
                result ^= v29;
                break;  /* Creates early exit */
            }
            if (j == 2 && (i % 5 == 0)) {
                continue;  /* Creates continue edge */
            }
            v29 = v29 + j;
            result += v29;
        }
        
        /* Use all variables to prevent elimination */
        USE(v0); USE(v1); USE(v2); USE(v3); USE(v4); USE(v5);
        USE(v6); USE(v7); USE(v8); USE(v9); USE(v10); USE(v11);
        USE(v12); USE(v13); USE(v14); USE(v15); USE(v16); USE(v17);
        USE(v18); USE(v19); USE(v20); USE(v21); USE(v22); USE(v23);
        USE(v24); USE(v25); USE(v26); USE(v27); USE(v28); USE(v29);
    }
    
    return result;
}

/* Another complex function to increase interprocedural pressure */
static int __attribute__((noinline)) helper_func(int x, int y) {
    volatile int a = x, b = y;
    volatile int c, d, e, f, g, h;
    
    /* Create data dependencies */
    for (int i = 0; i < 50; i++) {
        c = a + b;
        d = c * a;
        e = d ^ b;
        f = e | c;
        g = f & d;
        h = g + e;
        
        /* Conditional with goto to create irregular CFG */
        if (i % 3 == 0) goto label1;
        if (i % 3 == 1) goto label2;
        
        a = h + i;
        b = a * i;
        continue;
        
    label1:
        a = h - i;
        b = a / (i + 1);
        continue;
        
    label2:
        a = h ^ i;
        b = a | i;
    }
    
    /* Force register clobbering */
    asm volatile(
        "# More register pressure\n"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
    
    return a + b + c + d + e + f + g + h;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int res1 = test_mcf(i * 12345);
        int res2 = helper_func(i, i * 6789);
        total += res1 ^ res2;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total);
    return 0;
}
