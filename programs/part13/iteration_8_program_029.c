/* ddg_edge_coverage.c
 * 
 * This program is designed to trigger the DDG (Data Dependency Graph) edge
 * creation logic in GCC's ddg.cc, specifically the initialization block
 * for ddg_edge structures (lines 749-757 in the target file).
 * 
 * It creates complex loop-carried dependencies across multiple basic blocks
 * to force the scheduler's DDG construction to allocate and initialize
 * many dependency edges with varying types, latencies, and distances.
 */

#include <stdint.h>
#include <stdio.h>

/* Global accumulator to prevent dead code elimination */
static volatile int global_sink = 0;

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noipa))

/* Memory-intensive kernel with pointer aliasing */
NOINLINE static void kernel_mem(int* arr, int* brr, int n) {
    int i;
    int tmp1, tmp2, tmp3, tmp4;
    int* p1;
    int* p2;
    
    /* Multiple live variables to increase register pressure */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    
    for (i = 0; i < n; ++i) {
        /* Create multiple basic blocks with if/else */
        if (i & 1) {
            /* True dependency (RAW) chain */
            tmp1 = arr[i] + brr[i];
            tmp2 = tmp1 * 2;
            tmp3 = tmp2 - arr[i];
            
            /* Anti-dependency (WAR) */
            a = arr[i];          /* Read arr[i] */
            arr[i] = tmp3;       /* Write arr[i] - anti-dependency on 'a' */
            
            /* Output dependency (WAW) on tmp4 */
            tmp4 = a + b;
            tmp4 = tmp3 * tmp4;  /* Second write to tmp4 */
            
            /* Memory dependency with potential aliasing */
            p1 = &arr[i];
            p2 = &brr[(i * 7) % n];  /* May alias with arr */
            *p1 = tmp4;
            c += *p2;                /* MEM_DEP possible */
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = brr[i] - arr[i];
            tmp2 = tmp1 / 3;         /* Higher latency operation */
            
            /* Nested loop to create more complex control flow */
            for (int j = 0; j < 3; ++j) {
                /* Loop-carried dependency within nested loop */
                d = d + tmp2 + j;
                e = e ^ d;           /* XOR creates data dependency */
            }
            
            /* Output dependency (WAW) */
            f = arr[i] + 1;
            f = brr[i] * f;          /* Second write to f */
            
            /* Anti-dependency (WAR) with array */
            b = brr[i];              /* Read brr[i] */
            brr[i] = f;              /* Write brr[i] */
        }
        
        /* Cross-iteration (loop-carried) dependencies */
        if (i > 0) {
            /* True loop-carried dependency */
            arr[i] += arr[i-1];      /* Distance = 1 */
            
            /* Another loop-carried with different distance */
            if (i > 2) {
                brr[i] ^= brr[i-3];  /* Distance = 3 */
            }
        }
        
        /* Use volatile asm to prevent elimination */
        __asm__ volatile("" : "+r"(a), "+r"(b), "+r"(c));
    }
    
    /* Feed results to global sink */
    global_sink += a + b + c + d + e + f;
}

/* Arithmetic-intensive kernel with complex dependency chains */
NOINLINE static void kernel_arith(int* arr, int n) {
    int i;
    /* Many live variables */
    int x0 = 1, x1 = 2, x2 = 3, x3 = 4, x4 = 5;
    int y0 = 6, y1 = 7, y2 = 8, y3 = 9, y4 = 10;
    
    for (i = 0; i < n; ++i) {
        /* Long true dependency chain */
        x0 = arr[i] + x0;
        x1 = x0 * x1;
        x2 = x1 - x2;
        x3 = x2 / (arr[i] + 1);  /* Division - higher latency */
        x4 = x3 % (x4 + 2);      /* Modulo - even higher latency */
        
        /* Parallel dependency chain */
        y0 = y4 ^ y0;
        y1 = y0 + y1;
        y2 = y1 * y2;
        y3 = y2 - y3;
        y4 = y3 + arr[i];
        
        /* Control dependencies */
        if (x0 > 0) {
            if (x1 & 1) {
                x2 = x2 + y0;
            } else {
                x3 = x3 - y1;
            }
        } else {
            x4 = x4 * y2;
        }
        
        /* Output dependencies (WAW) */
        int tmp = x0 + x1;
        tmp = x2 * x3;      /* Second write to tmp */
        tmp = tmp + x4;     /* Third write to tmp */
        
        /* Anti-dependencies (WAR) */
        int old_x0 = x0;
        x0 = tmp + arr[i];  /* Anti-dependency on old_x0 */
        
        /* Loop-carried with varying distances */
        if (i % 4 == 0) {
            y0 = y0 + arr[(i + 1) % n];  /* Forward reference */
        }
        
        /* Prevent optimization */
        __asm__ volatile("" : "+r"(x0), "+r"(x1), "+r"(y0), "+r"(y1));
    }
    
    global_sink += x0 + x1 + x2 + x3 + x4 + y0 + y1 + y2 + y3 + y4;
}

/* Kernel with mixed control flow and dependencies */
NOINLINE static void kernel_mixed(float* fa, int* ib, int n) {
    int i;
    float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4;
    
    for (i = 0; i < n; ++i) {
        /* Multiple basic blocks with switch-like pattern */
        switch (i % 5) {
            case 0:
                f1 = fa[i] * f1;
                i1 = ib[i] + i1;
                /* True dependency across types */
                f2 = f1 + (float)i1;
                break;
            case 1:
                i2 = ib[i] - i2;
                f3 = f2 / f3;
                /* Anti-dependency */
                float old_f1 = f1;
                f1 = f3 * 2.0f;
                i3 = (int)old_f1 + i3;
                break;
            case 2:
                /* Output dependency */
                int tmp = i1 * i2;
                tmp = i3 + tmp;
                tmp = tmp - i4;
                f2 = (float)tmp;
                break;
            case 3:
                /* Memory dependency with type conversion */
                fa[i] = f1 + f2;
                i4 = (int)fa[i] ^ i4;
                break;
            default:
                /* Complex nested loop */
                for (int j = 0; j < 2; ++j) {
                    f3 = f3 + (float)j;
                    i1 = i1 + (int)f3;
                }
                break;
        }
        
        /* Loop-carried dependency with condition */
        if (i > 10) {
            fa[i % n] += fa[(i - 5) % n];  /* Distance = 5 */
        }
        
        /* Cross-iteration on integer array */
        ib[i] += ib[(i + 2) % n];          /* Distance = -2 (forward) */
        
        /* Prevent optimization */
        __asm__ volatile("" : "+r"(i1), "+r"(i2), "+m"(fa[i]));
    }
    
    global_sink += (int)f1 + (int)f2 + (int)f3 + i1 + i2 + i3 + i4;
}

/* Simple LCG for pseudo-random values */
static inline int lcg(int* state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

int main(void) {
    const int N = 1024;
    const int ITERS = 10000;
    
    /* Allocate and initialize data */
    int data_int[N];
    float data_float[N];
    int data_int2[N];
    
    int seed = 42;
    for (int i = 0; i < N; ++i) {
        data_int[i] = lcg(&seed) % 100;
        data_float[i] = (float)(lcg(&seed) % 100) / 10.0f;
        data_int2[i] = lcg(&seed) % 100;
    }
    
    /* Call kernels with different dependency patterns */
    kernel_mem(data_int, data_int2, ITERS);
    kernel_arith(data_int, ITERS);
    kernel_mixed(data_float, data_int2, ITERS);
    
    /* Use result to prevent dead code elimination */
    __asm__ volatile("" : : "r"(global_sink));
    
    return global_sink != 0 ? 0 : 1;
}
