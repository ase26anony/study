/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
static volatile int g_seed = 0;

/* Complex function with high register pressure and control flow */
int __attribute__((noinline)) 
complex_mcf_function(int iterations, int mode) 
{
    /* Declare many variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    float f0, f1, f2, f3, f4;
    int *p0, *p1, *p2;
    int result = 0;
    
    /* Initialize variables with complex dependencies */
    v0 = iterations + mode;
    v1 = v0 * 2;
    v2 = v1 - mode;
    v3 = v2 / (mode + 1);
    v4 = v3 ^ v0;
    
    /* Create pointer aliasing to complicate analysis */
    p0 = &v0;
    p1 = &v1;
    p2 = &v2;
    
    /* Float operations to pressure FP registers */
    f0 = (float)v0;
    f1 = (float)v1;
    f2 = f0 * f1;
    f3 = f2 / (f0 + 1.0f);
    f4 = f3 - f2;
    
    /* Main complex loop with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Complex conditional creating many basic blocks */
        switch (i % 13) {
            case 0:
                v5 = v0 + v1;
                v6 = v5 * v2;
                /* Inline asm with register clobbering */
                asm volatile ("# Force register pressure" 
                             : "=r"(v7) 
                             : "0"(v6)
                             : "eax", "ebx", "ecx", "edx");
                break;
            case 1:
                v8 = v2 - v3;
                v9 = v8 | v4;
                asm volatile ("# More pressure" 
                             : "=r"(v10) 
                             : "0"(v9)
                             : "esi", "edi");
                break;
            case 2:
                v11 = v3 ^ v4;
                v12 = v11 & v0;
                break;
            case 3:
                v13 = v4 * v1;
                v14 = v13 / (v2 + 1);
                /* Force spill/reload */
                asm volatile ("# Clobber multiple regs" 
                             : "=r"(v15) 
                             : "0"(v14)
                             : "eax", "ebx", "ecx", "edx", 
                               "esi", "edi", "r8", "r9", "r10");
                break;
            case 4:
                v16 = v5 + v6;
                v17 = v16 - v7;
                break;
            case 5:
                v18 = v8 * v9;
                v19 = v18 ^ v10;
                break;
            case 6:
                v20 = v11 | v12;
                v21 = v20 & v13;
                break;
            case 7:
                v22 = v14 + v15;
                v23 = v22 * v16;
                break;
            case 8:
                v24 = v17 - v18;
                v25 = v24 / (v19 + 1);
                break;
            case 9:
                v26 = v20 ^ v21;
                v27 = v26 | v22;
                break;
            case 10:
                v28 = v23 & v24;
                v29 = v28 * v25;
                break;
            case 11:
                /* Complex floating point operation */
                f0 = f1 + f2;
                f1 = f3 * f4;
                f2 = f0 - f1;
                v0 = (int)f2;
                break;
            case 12:
                /* Pointer chasing */
                *p0 = *p1 + *p2;
                p1 = p0;
                p0 = &v29;
                break;
        }
        
        /* Nested conditionals creating more basic blocks */
        if (i & 1) {
            if (i & 2) {
                v0 = v1 + v2;
                if (i & 4) {
                    v3 = v4 * v5;
                    goto label_a;
                } else {
                    v6 = v7 - v8;
                }
            } else {
                if (i & 8) {
                    v9 = v10 ^ v11;
                } else {
                    v12 = v13 | v14;
                }
            }
        } else {
            if (i & 16) {
                v15 = v16 & v17;
            } else {
                v18 = v19 + v20;
            }
        }
        
        /* Another level of nesting */
        if (mode == 0) {
            v21 = v22 * v23;
        } else if (mode == 1) {
            v24 = v25 - v26;
        } else if (mode == 2) {
            v27 = v28 ^ v29;
        } else {
            v0 = v1 | v2;
        }
        
    label_a:
        /* Cross-jump target */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                  v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                  v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        /* Loop with break/continue creating exit edges */
        for (int j = 0; j < 3; j++) {
            if (j == 1 && (i % 7 == 0)) {
                continue;  /* Creates loop exit */
            }
            if (j == 2 && (i % 11 == 0)) {
                break;     /* Creates another exit */
            }
            result += j;
        }
    }
    
    /* Final complex computation */
    result = (result ^ v0) | (v1 & v2) * (v3 - v4) / (v5 + 1);
    
    return result;
}

/* Another complex function to prevent inlining */
int __attribute__((noinline))
secondary_function(int x, int y) 
{
    int a = x, b = y, c, d, e, f, g, h, i, j;
    
    /* Chain of dependent operations */
    c = a + b;
    d = c * a;
    e = d - b;
    f = e ^ c;
    g = f | d;
    h = g & e;
    i = h * f;
    j = i - g;
    
    /* Inline asm with many clobbers */
    asm volatile ("# Secondary pressure\n\t"
                  "# More clobbers"
                  : "=r"(a), "=r"(b)
                  : "0"(j), "1"(i)
                  : "rax", "rbx", "rcx", "rdx", 
                    "rsi", "rdi", "r8", "r9", "r10", "r11",
                    "r12", "r13", "r14", "r15", "xmm0", "xmm1");
    
    return a + b + c + d + e + f + g + h + i + j;
}

int main(void) 
{
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += complex_mcf_function(50 + (i % 20), i % 4);
        total += secondary_function(i, i * 2);
        
        /* Add some global side effects */
        g_seed = (g_seed * 1103515245 + 12345) & 0x7fffffff;
        if (g_seed & 1) {
            total ^= g_seed;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
