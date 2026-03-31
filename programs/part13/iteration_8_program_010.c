/* ddg_edge_trigger.c
 * Program designed to trigger GCC's DDG edge creation logic
 * for uncovered lines in ddg.cc (edge initialization)
 */

#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

/* Global accumulator to prevent optimization */
static volatile int global_sink = 0;

/* Prevent inlining and IPA optimizations */
static __attribute__((noinline, noipa))
void kernel_memory_aliasing(int* restrict data1, int* restrict data2, 
                           int* restrict data3, int n) {
    int i;
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    /* Complex loop with memory aliasing and multiple dependencies */
    for (i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency chain (RAW) */
            tmp1 = data1[i] + data2[i];          /* Node 1 */
            tmp2 = tmp1 * data3[i];              /* Node 2: depends on tmp1 */
            tmp3 = tmp2 / (data1[i] + 1);        /* Node 3: depends on tmp2 and data1[i] */
            
            /* Anti-dependency (WAR) */
            int read_before = data1[i];          /* Read data1[i] */
            data1[i] = tmp3 + i;                 /* Write data1[i] - anti-dep on read_before */
            
            /* Output dependency (WAW) */
            tmp4 = read_before * 2;              /* First write to tmp4 */
            if (tmp3 > 100) {
                tmp4 = tmp3 / 2;                 /* Second write to tmp4 - output dep */
            }
            
            /* Memory dependency with potential aliasing */
            int* p1 = &data1[i];
            int* p2 = &data2[(i + 1) % n];
            *p1 = *p2 + tmp4;                    /* MEM_DEP potential */
            sum += *p1;                          /* Another read - creates edge */
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = data2[i] - data3[i];
            tmp2 = tmp1 % 17;                    /* Higher latency operation */
            
            /* Nested loop inside basic block */
            int j;
            for (j = 0; j < 3; ++j) {
                tmp2 += data1[(i + j) % n] * j;  /* Complex addressing */
            }
            
            /* Control-dependent computation */
            tmp3 = (tmp2 > 0) ? tmp2 : -tmp2;
            data3[i] = tmp3;                     /* Write to array */
            
            /* Another output dependency */
            int accumulator = data2[i];
            accumulator = tmp1 + tmp3;           /* WAW on accumulator */
            sum += accumulator;
        }
        
        /* Cross-iteration dependency (loop-carried) */
        if (i > 0) {
            data2[i] += data2[i-1];              /* Distance=1 dependency */
        }
        
        /* Volatile asm to prevent elimination */
        __asm__ volatile("" : "+r"(sum) : : "memory");
    }
    
    /* Feed result to global sink */
    global_sink += sum;
}

static __attribute__((noinline, noipa))
void kernel_arithmetic_chains(int* data, int n) {
    int i;
    int a, b, c, d, e, f, g, h, x, y, z;
    
    /* Many live variables to increase register pressure */
    a = b = c = d = e = f = g = h = x = y = z = 0;
    
    for (i = 0; i < n; ++i) {
        /* Long arithmetic dependency chain */
        a = data[i] + i;
        b = a * 3 - data[i];                     /* RAW on a */
        c = b / (a + 1);                         /* RAW on b and a */
        d = c % 7 + b;                           /* RAW on c and b */
        e = d << 2;                              /* RAW on d */
        f = e ^ 0xAAAA;                          /* RAW on e */
        g = f | (e & 0x5555);                    /* RAW on f and e */
        h = g + f - e;                           /* RAW on g, f, e */
        
        /* Parallel chains that converge */
        x = data[n - i - 1] * 2;
        y = x + i;
        z = y - x;
        
        /* Convergence point with multiple dependencies */
        if (h > z) {
            data[i] = h + z;                     /* Multiple predecessors */
        } else {
            data[i] = h - z;
        }
        
        /* Anti-dependencies with array */
        int temp = data[(i + 1) % n];            /* Read */
        data[(i + 1) % n] = h + 1;               /* Write - WAR */
        
        /* Complex condition with side effects */
        if ((i % 3) == 0) {
            a = b + c;                           /* WAW on a */
            x = y * z;                           /* WAW on x */
        } else if ((i % 3) == 1) {
            b = c + d;                           /* WAW on b */
            y = x / (z + 1);                     /* WAW on y */
        } else {
            c = d + e;                           /* WAW on c */
            z = y - x;                           /* WAW on z */
        }
        
        /* Force all variables live */
        __asm__ volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d),
                               "+r"(e), "+r"(f), "+r"(g), "+r"(h),
                               "+r"(x), "+r"(y), "+r"(z) : : "memory");
    }
    
    /* Use all variables to prevent elimination */
    global_sink += a + b + c + d + e + f + g + h + x + y + z;
}

static __attribute__((noinline, noipa))
void kernel_control_flow(int* data1, int* data2, int n) {
    int i;
    int result = 0;
    
    for (i = 0; i < n; ++i) {
        /* Complex control flow with multiple basic blocks */
        switch (i % 4) {
            case 0: {
                int t1 = data1[i] + data2[i];
                int t2 = t1 * 2;
                int t3 = data2[i] - t1;
                result += t2 * t3;
                
                /* Nested if with dependencies */
                if (t2 > t3) {
                    data1[i] = t2;
                    data2[i] = t3;
                } else {
                    data1[i] = t3;
                    data2[i] = t2;
                }
                break;
            }
            case 1: {
                /* Different operations with dependencies */
                int u1 = data1[i] ^ data2[i];
                int u2 = u1 & 0xFF;
                int u3 = u2 | (i & 0xFF);
                
                /* Loop-carried output dependency */
                static int persistent = 0;
                int old = persistent;
                persistent = u3 + i;
                result += old;
                break;
            }
            case 2: {
                /* Memory dependency chain */
                int* ptr1 = &data1[i];
                int* ptr2 = &data2[(i * 2) % n];
                
                int v1 = *ptr1;
                *ptr1 = v1 + *ptr2;      /* WAR on v1, MEM_DEP between ptr1/ptr2 */
                int v2 = *ptr2;
                *ptr2 = v2 - v1;         /* More memory deps */
                result += *ptr1 + *ptr2;
                break;
            }
            case 3: {
                /* Arithmetic with high latency ops */
                int w1 = data1[i] % 19;          /* High latency */
                int w2 = data2[i] / 7;           /* High latency */
                int w3 = w1 * w2;
                
                /* Dependency across iterations */
                if (i > 10) {
                    w3 += data1[i-10];           /* Distance=10 dependency */
                }
                
                data1[i] = w3;
                result += w3;
                break;
            }
        }
        
        /* Volatile to prevent reordering */
        __asm__ volatile("" : "+r"(result) : : "memory");
    }
    
    global_sink += result;
}

/* Simple PRNG to initialize data without library calls */
static unsigned int lcg_seed = 12345;
static unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Large arrays to work with */
    int data1[SIZE];
    int data2[SIZE];
    int data3[SIZE];
    
    int i;
    
    /* Initialize with pseudo-random data */
    for (i = 0; i < SIZE; ++i) {
        data1[i] = (int)(lcg_rand() % 1000);
        data2[i] = (int)(lcg_rand() % 1000);
        data3[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Call kernels with different dependency patterns */
    kernel_memory_aliasing(data1, data2, data3, ITERS);
    kernel_arithmetic_chains(data1, ITERS);
    kernel_control_flow(data2, data3, ITERS);
    
    /* Use results to prevent dead code elimination */
    int final_result = 0;
    for (i = 0; i < SIZE; ++i) {
        final_result += data1[i] + data2[i] + data3[i];
    }
    
    /* Volatile sink for final result */
    __asm__ volatile("" : : "r"(final_result) : "memory");
    
    return final_result % 256;
}
