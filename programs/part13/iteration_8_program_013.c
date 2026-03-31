/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

/* Global accumulator to prevent optimization */
static volatile int global_sink = 0;

/* Prevent inlining and IPA optimizations */
static void __attribute__((noinline,noipa))
kernel_memory_heavy(int *restrict data1, int *restrict data2, int n) {
    int i, sum = 0;
    int *p1, *p2;
    
    /* Complex memory aliasing patterns */
    for (i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if/else */
        if (i & 1) {
            /* True dependency (RAW) with memory */
            int temp = data1[i];
            data1[i] = temp * 2 + i;  /* WAW on data1[i] */
            
            /* Anti-dependency (WAR) */
            int read_before_write = data2[i];
            data2[i] = (i % 7) + 1;   /* WAR on data2[i] */
            sum += read_before_write;
            
            /* Pointer aliasing to create MEM_DEP edges */
            p1 = &data1[i];
            p2 = &data2[(i + 1) % n];
            *p1 = *p2 + sum;          /* Potential memory dependency */
        } else {
            /* Different dependency pattern in else branch */
            int chain1 = data1[i] + 3;
            int chain2 = chain1 * data2[i];  /* RAW on chain1 */
            int chain3 = chain2 / (data1[i] + 1); /* High latency divide */
            
            /* Output dependency (WAW) */
            int tmp = chain1;
            tmp = chain2;  /* WAW on tmp */
            tmp = chain3;  /* Another WAW */
            
            data1[i] = tmp;
            sum += chain3;
        }
        
        /* Nested loop to create more basic blocks */
        int j;
        for (j = 0; j < 3; ++j) {
            /* Cross-iteration dependency */
            data2[(i + j) % n] += sum;
        }
        
        /* Volatile asm to prevent elimination */
        __asm__ volatile("" : "+r"(sum) : : "memory");
    }
    
    global_sink += sum;
}

static void __attribute__((noinline,noipa))
kernel_arithmetic_chains(int *restrict a, int *restrict b, int n) {
    int i;
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
    /* Many live variables to increase register pressure */
    r1 = r2 = r3 = r4 = r5 = 1;
    r6 = r7 = r8 = r9 = r10 = 0;
    
    for (i = 0; i < n; ++i) {
        /* Long arithmetic dependency chain */
        r1 = a[i] + b[i];
        r2 = r1 * 3;           /* RAW on r1 */
        r3 = r2 - i;           /* RAW on r2 */
        r4 = r3 / (r1 + 1);    /* High latency, RAW on r1 and r3 */
        r5 = r4 % 17;          /* High latency modulo, RAW on r4 */
        
        /* Parallel chain with anti-dependencies */
        r6 = r5 + r2;          /* RAW on r5 and r2 */
        r7 = r6;               /* Copy */
        r6 = r3 + r4;          /* WAR on r6 */
        
        /* Control-dependent computations */
        if (i % 3 == 0) {
            r8 = r5 * r7;      /* RAW on r5, r7 */
            r9 = r8 >> 2;      /* RAW on r8 */
        } else if (i % 3 == 1) {
            r8 = r6 | r4;      /* WAR on r8, RAW on r6, r4 */
            r9 = r8 & 0xFF;    /* RAW on r8 */
        } else {
            r8 = r2 ^ r3;      /* WAR on r8, RAW on r2, r3 */
            r9 = r8 + r1;      /* RAW on r8, r1 */
        }
        
        /* Output dependencies (WAW) */
        r10 = r9;
        r10 = r10 + r5;        /* WAW on r10 */
        r10 = r10 * 2;         /* WAW on r10 */
        
        /* Store results creating memory dependencies */
        a[i] = r10;
        b[i] = r9;
        
        /* Force all values to be live */
        __asm__ volatile("" 
            : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4), "+r"(r5),
              "+r"(r6), "+r"(r7), "+r"(r8), "+r"(r9), "+r"(r10)
            : : "memory");
    }
    
    /* Use results to prevent elimination */
    global_sink += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
}

static void __attribute__((noinline,noipa))
kernel_control_flow(int *restrict arr, int n) {
    int i, acc = 0;
    
    for (i = 1; i < n - 1; ++i) {
        /* Complex control flow with dependencies across iterations */
        int left = arr[i - 1];
        int curr = arr[i];
        int right = arr[i + 1];
        
        /* Multi-way branching with different dependency patterns */
        switch (i % 4) {
            case 0:
                /* True dependencies with memory */
                arr[i] = left + curr;
                acc += arr[i] * 2;  /* RAW on arr[i] */
                break;
            case 1:
                /* Anti-dependencies */
                int old_val = arr[i];
                arr[i] = right - left;
                acc += old_val;     /* WAR on arr[i] via old_val */
                break;
            case 2:
                /* Output dependencies and computation */
                int tmp = left * right;
                tmp = tmp + curr;   /* WAW on tmp */
                tmp = tmp % 13;     /* WAW on tmp */
                arr[i] = tmp;
                acc ^= tmp;         /* RAW on tmp */
                break;
            case 3:
                /* Mixed dependencies */
                arr[i] = (arr[i] << 2) | (acc & 3);
                acc = acc + arr[i]; /* RAW on arr[i], WAR on acc */
                break;
        }
        
        /* Loop-carried dependency with distance > 0 */
        if (i > 10) {
            arr[i] += arr[i - 10];  /* Distance 10 dependency */
        }
        
        /* Prevent optimization */
        __asm__ volatile("" : "+r"(acc) : : "memory");
    }
    
    global_sink += acc;
}

/* Simple PRNG to initialize data without library calls */
static uint32_t lcg = 123456789;
static uint32_t rand_lcg(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main(void) {
    int data1[SIZE];
    int data2[SIZE];
    int data3[SIZE];
    int i;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < SIZE; ++i) {
        data1[i] = (int)(rand_lcg() % 1000);
        data2[i] = (int)(rand_lcg() % 1000);
        data3[i] = (int)(rand_lcg() % 1000);
    }
    
    /* Call kernels with different dependency patterns */
    kernel_memory_heavy(data1, data2, ITERS);
    kernel_arithmetic_chains(data1, data3, ITERS);
    kernel_control_flow(data2, ITERS);
    
    /* Combine results to prevent dead code elimination */
    int final = 0;
    for (i = 0; i < SIZE; ++i) {
        final += data1[i] + data2[i] + data3[i];
    }
    
    /* Volatile sink */
    __asm__ volatile("" : : "r"(final));
    
    return final % 256;
}
