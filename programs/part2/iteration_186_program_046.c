/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Complex test function with high register pressure */
static int __attribute__((noinline)) test_mcf_pressure(int seed, int iterations) {
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
    
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2, *p3 = &v3;
    
    int result = 0;
    int i, j;
    
    /* Complex loop with register-intensive computations */
    for (i = 0; i < iterations; i++) {
        /* Chain of dependent arithmetic operations */
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
        
        /* Floating point operations to pressure FP registers */
        f0 = f1 * f2;
        f1 = f3 + f4;
        f2 = f0 - f1;
        f3 = f2 * 1.5f;
        f4 = f3 / 2.0f;
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + v0;
        p1 = (char*)p2 - v1;
        p2 = (char*)p3 + v2;
        p3 = (char*)p0 - v3;
        
        /* Inline assembly with register clobbers to force graph transformations */
        /* For x86: */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (v21)
            : "r" (v22)
            : "%eax", "%ebx", "%ecx", "%edx", "memory"
        );
        
        /* Complex conditional structure creating many basic blocks */
        if (i % 3 == 0) {
            v12 = v13 + v14;
            if (v12 > 100) {
                v13 = v15 * 2;
                goto label_a;
            } else {
                v13 = v16 / 2;
                if (v13 < 50) {
                    v14 = v17 ^ v18;
                } else {
                    v14 = v19 | v20;
                }
            }
        } else if (i % 3 == 1) {
            v15 = v21 - v22;
            if (v15 < 0) {
                v16 = v23 + v24;
            } else {
                v16 = v25 - v26;
            }
        } else {
            v17 = v27 * v28;
            if (v17 > 1000) {
                v18 = v29 >> 2;
            }
        }
        
        /* Switch statement with many cases for control flow complexity */
        switch (i % 10) {
            case 0: v19 = v0 + v1; break;
            case 1: v19 = v2 - v3; break;
            case 2: v19 = v4 * v5; break;
            case 3: v19 = v6 / (v7 + 1); break;
            case 4: v19 = v8 ^ v9; break;
            case 5: v19 = v10 | v11; break;
            case 6: v19 = v12 & v13; break;
            case 7: v19 = v14 << 1; break;
            case 8: v19 = v15 >> 2; break;
            case 9: v19 = v16 + v17; break;
            default: v19 = v18 - v19; break;
        }
        
        /* Nested loop for additional complexity */
        for (j = 0; j < 3; j++) {
            v20 = v21 + j;
            v21 = v22 - j;
            v22 = v23 * (j + 1);
            
            if (j == 1) {
                /* Another inline asm with different clobbers */
                asm volatile (
                    "movl %0, %%ebx\n\t"
                    "subl %1, %%ebx\n\t"
                    "movl %%ebx, %0\n\t"
                    : "+r" (v23)
                    : "r" (v24)
                    : "%eax", "%ebx", "%ecx", "%edx", "memory"
                );
            }
        }
        
        /* Use goto to create non-trivial control flow */
        if (v24 > 1000) {
            goto label_b;
        }
        
        continue;
        
    label_a:
        v25 = v26 + v27;
        if (v25 % 2 == 0) {
            v26 = v28 * 3;
        } else {
            v26 = v29 / 3;
        }
        continue;
        
    label_b:
        v27 = v0 ^ v1;
        v28 = v2 | v3;
        if (v27 > v28) {
            v29 = v4 + v5;
        } else {
            v29 = v6 - v7;
        }
        
        /* Another asm with memory clobber */
        asm volatile (
            "movl %0, %%ecx\n\t"
            "addl $1, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "+r" (v29)
            :: "%eax", "%ebx", "%ecx", "%edx", "memory"
        );
    }
    
    /* Compute final checksum from all variables */
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
             v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
             v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
             (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    
    return result;
}

/* Wrapper function to prevent inlining and create more call context */
static int __attribute__((noinline)) run_mcf_test(int base) {
    int sum = 0;
    
    /* Call test function multiple times with different parameters */
    sum += test_mcf_pressure(base, 10);
    sum += test_mcf_pressure(base + 1, 15);
    sum += test_mcf_pressure(base + 2, 20);
    sum += test_mcf_pressure(base + 3, 25);
    sum += test_mcf_pressure(base + 4, 30);
    
    return sum;
}

int main(int argc, char** argv) {
    int total = 0;
    int i;
    
    /* Run multiple iterations to ensure MCF pass runs */
    for (i = 0; i < 5; i++) {
        total += run_mcf_test(i * 100);
    }
    
    printf("MCF test result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
