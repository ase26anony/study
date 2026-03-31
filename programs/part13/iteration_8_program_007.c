/* ddg_edge_coverage.c
 * 
 * This program is designed to trigger the DDG (Data Dependency Graph) edge
 * creation logic in GCC's ddg.cc, specifically the initialization block
 * for ddg_edge structures (lines 749-757 in the target file).
 * 
 * It creates complex loop-carried dependencies across multiple basic blocks
 * to force the scheduler's DDG construction to allocate and initialize
 * many dependency edges between instructions.
 */

#include <stdint.h>
#include <stdio.h>

/* Global accumulator to prevent dead code elimination */
static volatile int global_sink = 0;

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noipa))

/* Memory-intensive kernel with pointer aliasing */
NOINLINE static void kernel_mem_dep(int* arr, int* brr, int n, int* result) {
    int sum = 0;
    int* p1 = arr;
    int* p2 = brr;
    
    /* Complex loop with multiple dependency types */
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if/else */
        if (i & 1) {
            /* True dependency (RAW) chain */
            int t1 = p1[i] * 3;
            int t2 = t1 + p2[i];
            int t3 = t2 / 2;
            
            /* Anti-dependency (WAR) */
            int read_before = p2[i];
            p2[i] = t3 + i;
            sum += read_before;
            
            /* Output dependency (WAW) */
            int tmp = p1[i];
            if (t3 > 100) {
                tmp = t2 * 2;  /* WAW on tmp */
            }
            sum += tmp;
        } else {
            /* Different dependency pattern in else branch */
            int a = p1[i] + p2[i];
            int b = a % 7;  /* Higher latency operation */
            int c = b * b;
            
            /* Memory dependency with potential aliasing */
            *(arr + (i % 16)) = c;
            sum += *(brr + (i % 16));
            
            /* Nested loop to create more complex CFG */
            for (int j = 0; j < 3; ++j) {
                int d = c + j;
                sum += d;
            }
        }
        
        /* Cross-iteration dependency (loop-carried) */
        if (i > 0) {
            p1[i] += p1[i-1];  /* True loop-carried dependency */
        }
        
        /* Volatile asm to prevent elimination */
        __asm__ volatile("" : : "r"(sum));
    }
    
    *result = sum;
    global_sink += *result;
}

/* Arithmetic-intensive kernel with long dependency chains */
NOINLINE static void kernel_arith_dep(int* data, int n, int* result) {
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5;
    int r5 = 6, r6 = 7, r7 = 8, r8 = 9, r9 = 10;
    
    for (int i = 0; i < n; ++i) {
        /* Long true dependency chain */
        r0 = data[i] + r9;
        r1 = r0 * r0;
        r2 = r1 - r0;
        r3 = r2 / (data[i] | 1);  /* Avoid division by zero */
        r4 = r3 % 17;
        r5 = r4 ^ r3;
        r6 = r5 << 2;
        r7 = r6 >> 1;
        r8 = r7 + i;
        r9 = r8 * 3;
        
        /* Control-dependent computations */
        if (r9 > 1000) {
            int t = r9;
            r9 = r8;  /* WAR on r9 */
            r8 = t;   /* Output dependency chain */
        } else if (r9 < 0) {
            r9 = -r9;
            r8 = r9 * 2;
        } else {
            r8 = r9 + 100;
        }
        
        /* Multiple output dependencies */
        int tmp = r0;
        tmp = r1 + tmp;  /* WAW on tmp */
        tmp = r2 * tmp;  /* Another WAW */
        
        /* Use all variables to keep them live */
        __asm__ volatile("" : : "r"(r0), "r"(r1), "r"(r2), "r"(r3), 
                         "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9), "r"(tmp));
    }
    
    *result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    global_sink += *result;
}

/* Kernel with complex control flow and mixed dependencies */
NOINLINE static void kernel_control_dep(int* arr, int n, int* result) {
    int sum = 0;
    int acc[4] = {0, 0, 0, 0};
    
    for (int i = 0; i < n; ++i) {
        /* Switch-like control flow */
        switch (i % 4) {
            case 0: {
                /* Memory anti-dependency */
                int old_val = arr[i];
                arr[i] = acc[0] + i;
                sum += old_val;
                acc[0] = arr[i] * 2;
                break;
            }
            case 1: {
                /* Arithmetic chain with output dep */
                int tmp = acc[1];
                tmp = tmp + arr[i];
                tmp = tmp - acc[0];  /* WAW on tmp */
                acc[1] = tmp;
                sum += tmp;
                break;
            }
            case 2: {
                /* Nested loop with carried dependency */
                int val = acc[2];
                for (int k = 0; k < 2; ++k) {
                    val = val * 3 + k;
                }
                acc[2] = val;
                sum += val;
                
                /* Cross-case dependency */
                arr[i] += acc[1];
                break;
            }
            case 3: {
                /* Complex expression with multiple deps */
                int x = acc[3];
                int y = arr[i];
                int z = x * y;
                
                /* True dependency chain */
                z = z + (x % 13);
                z = z | (y & 0xFF);
                z = z ^ (i << 3);
                
                acc[3] = z;
                sum += z;
                break;
            }
        }
        
        /* Loop-carried dependency through acc array */
        if (i > 3) {
            arr[i] += acc[i % 4];
        }
        
        /* Prevent optimization */
        __asm__ volatile("" : : "r"(sum));
    }
    
    *result = sum + acc[0] + acc[1] + acc[2] + acc[3];
    global_sink += *result;
}

/* Simple PRNG for initialization (no library calls) */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    /* Large arrays to work with */
    int data[1024];
    int data2[1024];
    int seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 1024; ++i) {
        data[i] = simple_rand(&seed) % 1000;
        data2[i] = simple_rand(&seed) % 1000;
    }
    
    int res1, res2, res3;
    
    /* Call kernels with different dependency patterns */
    kernel_mem_dep(data, data2, 1000, &res1);
    kernel_arith_dep(data, 1000, &res2);
    kernel_control_dep(data2, 1000, &res3);
    
    /* Combine results and use volatile asm to prevent elimination */
    int final_result = res1 + res2 + res3;
    __asm__ volatile("" : : "r"(final_result));
    
    /* Also use global_sink */
    __asm__ volatile("" : : "r"(global_sink));
    
    return final_result % 256;
}
