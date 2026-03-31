/* ddg_edge_trigger.c
 * Program designed to trigger GCC's DDG edge creation logic
 * for uncovered lines in ddg.cc (edge initialization block)
 */

#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))
#define NOINLINE __attribute__((noinline, noipa))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory-heavy with pointer aliasing */
NOINLINE static void kernel_memory_aliasing(int* arr1, int* arr2, int n) {
    int i;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Create multiple live variables */
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    
    for (i = 0; i < n; ++i) {
        /* Basic block 1: RAW dependencies */
        tmp1 = arr1[i] + arr2[i];      /* RAW source for next */
        acc1 = tmp1 * 2;               /* Uses tmp1 */
        
        /* WAR dependencies */
        tmp2 = arr1[i & (SIZE-1)];     /* Read before write */
        arr1[i & (SIZE-1)] = acc1;     /* Write after read (WAR) */
        
        /* WAW dependencies */
        tmp3 = arr2[i & (SIZE-1)];     /* First write to tmp3 */
        if (i & 1) {
            /* Control flow creates different basic blocks */
            tmp3 = acc1 + tmp2;        /* Second write to tmp3 (WAW) */
            acc2 = tmp3 % 7;           /* High latency operation */
        } else {
            tmp3 = tmp2 - acc1;        /* Alternative WAW */
            acc2 = tmp3 / 3;           /* Another high latency op */
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; ++j) {
            /* More dependencies in nested scope */
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            e = f + a;
            f = a + b;
            
            /* Memory aliasing with pointer arithmetic */
            int* p1 = &arr1[(i + j) & (SIZE-1)];
            int* p2 = &arr2[(i - j + SIZE) & (SIZE-1)];
            
            /* Potential aliasing */
            *p1 = *p2 + acc2;
            acc3 += *p1;               /* MEM_DEP likely */
        }
        
        /* Another basic block with different dependencies */
        if (i % 4 == 0) {
            /* Long arithmetic chain */
            tmp4 = acc1 + acc2;
            acc4 = tmp4 * tmp4;
            acc4 = acc4 - tmp4;        /* WAW on acc4 */
            acc4 = acc4 / 2;           /* Another WAW */
        } else if (i % 4 == 1) {
            tmp4 = acc1 - acc2;
            acc4 = tmp4 | 0xFF;
        } else if (i % 4 == 2) {
            tmp4 = acc1 * acc2;
            acc4 = tmp4 & 0xFFFF;
        } else {
            tmp4 = acc2 - acc1;
            acc4 = tmp4 ^ 0xAAAA;
        }
        
        /* Loop-carried dependency */
        arr2[(i + 1) & (SIZE-1)] = acc4 + arr2[i & (SIZE-1)];
    }
    
    /* Keep results alive */
    KEEP_ALIVE(acc1);
    KEEP_ALIVE(acc2);
    KEEP_ALIVE(acc3);
    KEEP_ALIVE(acc4);
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Kernel 2: Arithmetic-heavy with complex control flow */
NOINLINE static void kernel_arithmetic_chains(int* arr, int n) {
    int i;
    /* Many live variables to increase register pressure */
    int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6, v6 = 7, v7 = 8;
    int v8 = 9, v9 = 10, v10 = 11, v11 = 12, v12 = 13, v13 = 14, v14 = 15;
    
    for (i = 0; i < n; ++i) {
        /* Multiple interleaved dependency chains */
        
        /* Chain 1: Integer arithmetic */
        v0 = v1 + arr[i & (SIZE-1)];   /* RAW: v1 */
        v1 = v0 * v2;                  /* RAW: v0, v2 */
        v2 = v1 - v3;                  /* RAW: v1, v3 */
        v3 = v2 / (v4 | 1);            /* RAW: v2, v4 (high latency) */
        
        /* Chain 2: Different operations */
        v4 = v5 ^ v6;                  /* RAW: v5, v6 */
        v5 = v4 << (i & 3);            /* RAW: v4 */
        v6 = v5 >> 1;                  /* RAW: v5 */
        
        /* Chain 3: With control flow */
        if (i & 8) {
            v7 = v8 + v9;
            v8 = v7 * v10;
            v9 = v8 % 13;              /* High latency */
        } else {
            v7 = v9 - v8;
            v8 = v10 ^ v7;
            v9 = v8 & 0xFF;
        }
        
        /* Chain 4: Cross-chain dependencies */
        v10 = v3 + v7;                 /* RAW from chain 1 and 3 */
        v11 = v10 * v2;
        v12 = v11 - v6;
        
        /* Loop-carried output dependency (WAW) */
        if (i % 3 == 0) {
            arr[i & (SIZE-1)] = v12;
        } else if (i % 3 == 1) {
            arr[i & (SIZE-1)] = v11;   /* WAW on arr[] */
        } else {
            arr[i & (SIZE-1)] = v10;   /* WAW on arr[] */
        }
        
        /* Anti-dependency (WAR) */
        v13 = arr[(i + 1) & (SIZE-1)]; /* Read */
        arr[(i + 1) & (SIZE-1)] = v12; /* Write to same location */
        v14 = v13 + v12;               /* Use read value */
        
        /* Another nested loop for basic block creation */
        for (int k = 0; k < 2; ++k) {
            /* Swap operations create dependencies */
            int t = v0; v0 = v1; v1 = t;  /* WAW/WAR mix */
            t = v2; v2 = v3; v3 = t;
            t = v4; v4 = v5; v5 = t;
        }
    }
    
    /* Force all variables to be live */
    int sum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14;
    KEEP_ALIVE(sum);
    global_acc += sum;
}

/* Kernel 3: Mixed dependencies with unpredictable control flow */
NOINLINE static void kernel_mixed_control_flow(int* arr1, int* arr2, int n) {
    int i;
    int x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0;
    
    for (i = 0; i < n; ++i) {
        /* Unpredictable branch to create control dependencies */
        int cond = arr1[i & (SIZE-1)] & 0xF;
        
        switch (cond & 3) {
            case 0:
                x0 = x1 + x2;
                x1 = x0 * 3;
                x2 = x1 - arr2[i & (SIZE-1)];
                break;
            case 1:
                x0 = x2 - x1;
                x1 = arr2[i & (SIZE-1)] ^ x0;
                x2 = x1 | 0xAA;
                break;
            case 2:
                x0 = x1 * x2;
                x1 = x0 % 17;          /* High latency */
                x2 = arr1[i & (SIZE-1)] + x1;
                break;
            default:
                x0 = x2 / (x1 | 1);    /* High latency */
                x1 = x0 + 5;
                x2 = x1 & arr2[i & (SIZE-1)];
                break;
        }
        
        /* Loop-carried true dependency (distance > 0) */
        x3 = x0 + x3;                  /* x3 depends on previous iteration's x3 */
        
        /* Memory dependency with potential aliasing */
        int idx = (i * 7) & (SIZE-1);
        x4 = arr1[idx];
        arr2[idx] = x3 + x4;           /* WAR on arr2[idx] if same idx */
        
        /* Another loop-carried dependency */
        if (i > 0) {
            x5 = arr1[(i-1) & (SIZE-1)] + x5;  /* Distance 1 */
        }
        
        /* Complex condition with multiple basic blocks */
        if (cond & 4) {
            for (int m = 0; m < 2; ++m) {
                int t = x0; x0 = x1; x1 = x2; x2 = t;
                arr1[(i + m) & (SIZE-1)] = x0 + x1 + x2;
            }
        } else {
            x0 = x0 ^ x1;
            x1 = x1 ^ x2;
            x2 = x2 ^ x0;
        }
    }
    
    int total = x0 + x1 + x2 + x3 + x4 + x5;
    KEEP_ALIVE(total);
    global_acc += total;
}

/* Simple PRNG to avoid library calls */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main(void) {
    int data1[SIZE];
    int data2[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = simple_rand(&seed);
        data2[i] = simple_rand(&seed);
    }
    
    /* Call kernels with different dependency patterns */
    kernel_memory_aliasing(data1, data2, ITERATIONS);
    kernel_arithmetic_chains(data1, ITERATIONS);
    kernel_mixed_control_flow(data1, data2, ITERATIONS);
    
    /* Use result to prevent dead code elimination */
    KEEP_ALIVE(global_acc);
    
    return global_acc & 0xFF;
}
