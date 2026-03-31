/* mcf_coverage.c - Program to trigger MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
static volatile int g_volatile_sink;

/* Large test function with complex control flow and register pressure */
static int __attribute__((noinline)) test_mcf_function(int seed) {
    /* Declare many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    float f0, f1, f2, f3, f4;
    void *p0, *p1, *p2;
    
    /* Initialize with seed to prevent constant propagation */
    v0 = seed;
    v1 = seed * 2;
    v2 = seed + 1;
    v3 = seed ^ 0x1234;
    v4 = seed | 0xABCD;
    
    /* Complex arithmetic chain creating data dependencies */
    for (int i = 0; i < 100; i++) {
        /* Force register pressure with overlapping computations */
        v5 = v0 + v1;
        v6 = v2 * v3;
        v7 = v4 ^ v5;
        v8 = v6 - v7;
        v9 = v8 >> 2;
        
        v10 = v9 * 3;
        v11 = v10 + v0;
        v12 = v11 - v1;
        v13 = v12 | v2;
        v14 = v13 & v3;
        
        v15 = v14 * v4;
        v16 = v15 / (v5 + 1);
        v17 = v16 ^ v6;
        v18 = v17 + v7;
        v19 = v18 - v8;
        
        v20 = v19 * v9;
        v21 = v20 + v10;
        v22 = v21 - v11;
        v23 = v22 | v12;
        v24 = v23 & v13;
        
        v25 = v24 * v14;
        v26 = v25 / (v15 + 1);
        v27 = v26 ^ v16;
        v28 = v27 + v17;
        v29 = v28 - v18;
        
        /* Float operations to pressure FP registers */
        f0 = (float)v19;
        f1 = (float)v20;
        f2 = f0 * f1;
        f3 = f2 / (f0 + 1.0f);
        f4 = f3 - f1;
        
        /* Pointer operations */
        p0 = &v21;
        p1 = &v22;
        p2 = (void*)((long)p0 ^ (long)p1);
        
        /* Inline assembly with register clobbers to force graph transformations */
        /* For x86/x86-64 */
        __asm__ volatile (
            "# MCF test asm block\n"
            "mov %0, %%eax\n"
            "add %1, %%eax\n"
            "mov %%eax, %0\n"
            : "+r" (v0), "+r" (v1)
            : 
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Complex conditional structure creating many basic blocks */
        if (v0 & 0x1) {
            v1 = v2 + v3;
            if (v1 > 1000) {
                v4 = v5 * 2;
                /* Another asm with different clobbers */
                __asm__ volatile (
                    "# Conditional asm\n"
                    : 
                    : "r" (v4)
                    : "rax", "rbx", "rcx"
                );
            } else {
                v4 = v5 / 2;
                goto label_a;
            }
        } else if (v0 & 0x2) {
            v1 = v2 - v3;
            if (v1 < 0) {
                v4 = v5 | v6;
            } else {
                v4 = v5 & v6;
            }
        } else {
            v1 = v2 ^ v3;
        }
        
        /* Switch with many cases creating control flow merges */
        switch (v0 & 0xF) {
            case 0: v2 = v3 + 1; break;
            case 1: v2 = v3 - 1; break;
            case 2: v2 = v3 * 2; break;
            case 3: v2 = v3 / 2; break;
            case 4: v2 = v3 | 0xFF; break;
            case 5: v2 = v3 & 0xFF; break;
            case 6: v2 = v3 ^ 0xFF; break;
            case 7: v2 = v3 << 2; break;
            case 8: v2 = v3 >> 2; break;
            case 9: v2 = ~v3; break;
            case 10: v2 = v3 + v4; break;
            case 11: v2 = v3 - v4; break;
            case 12: v2 = v3 * v4; break;
            case 13: v2 = abs(v3); break;
            case 14: v2 = v3 % (v4 + 1); break;
            case 15: v2 = v3 ^ v4; break;
            default: v2 = 0; break;
        }
        
        /* Nested loops with breaks/continues */
        for (int j = 0; j < 10; j++) {
            if (v2 > 1000) {
                v3 = v4 + j;
                if (j == 5) break;
            } else {
                v3 = v4 - j;
                if (j == 3) continue;
            }
            
            for (int k = 0; k < 5; k++) {
                v4 = v3 + k;
                if (k == v2 % 3) goto label_b;
            }
            
        label_b:
            v5 = v4 * 2;
        }
        
        /* Use all variables to prevent elimination */
        v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
             v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
             v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        /* Prevent loop invariant code motion */
        v0 += i;
    }
    
label_a:
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                 (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                 (int)(long)p0 + (int)(long)p1 + (int)(long)p2;
    
    /* Global sink to prevent dead code elimination */
    g_volatile_sink = result;
    
    return result;
}

/* Another function with different pattern to prevent IPA */
static int __attribute__((noinline)) test_mcf_function2(int seed) {
    int a[50];
    int sum = 0;
    
    for (int i = 0; i < 50; i++) {
        a[i] = seed + i;
        if (i % 3 == 0) {
            a[i] *= 2;
            /* More inline asm with clobbers */
            __asm__ volatile (
                "# Function2 asm\n"
                : 
                : "r" (a[i])
                : "rax", "rbx", "xmm0", "xmm1"
            );
        } else if (i % 3 == 1) {
            a[i] /= 2;
        } else {
            a[i] ^= 0x55;
        }
        
        /* Unstructured control flow */
        if (a[i] > 1000) {
            goto early_exit;
        }
        
        sum += a[i];
    }
    
    return sum;

early_exit:
    return sum ^ 0x1234;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += test_mcf_function(i);
        total += test_mcf_function2(i * 3);
        
        /* Mix in some volatile operations */
        __asm__ volatile (
            "# Main loop asm\n"
            : 
            : "r" (total)
            : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "xmm2"
        );
    }
    
    printf("Result: %d\n", total);
    return 0;
}
