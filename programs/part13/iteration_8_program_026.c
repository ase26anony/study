/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Global accumulator to keep variables live */
static int global_acc = 0;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v) : "memory")

/* Kernel 1: Memory-heavy with pointer aliasing and WAR/WAW dependencies */
NOINLINE static int kernel_mem(int *arr1, int *arr2, int n) {
    int sum = 0;
    int tmp1, tmp2, tmp3;
    
    /* Two pointers that may alias */
    int *p1 = arr1;
    int *p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Basic block 1: RAW dependencies */
        int a = p1[i] + i;
        int b = a * 2;
        int c = b - p2[i];
        
        /* Basic block 2: WAR dependencies (read before write to same location) */
        int read_first = p1[i];          /* Read */
        p1[i] = c + read_first;          /* Write to same location - WAR */
        
        /* Basic block 3: WAW dependencies */
        tmp1 = a + b;                    /* First write to tmp1 */
        if (i & 1) {
            tmp1 = c * 3;                /* Second write to tmp1 - WAW in conditional */
            tmp2 = tmp1 + read_first;
        } else {
            tmp2 = b + read_first;
            tmp1 = tmp2 - a;             /* Another WAW to tmp1 */
        }
        
        /* Basic block 4: More complex RAW chain with different latencies */
        int d = tmp1 % 7;                /* Higher latency operation */
        int e = d / 2;                   /* Another higher latency */
        int f = e + tmp2;
        
        /* Memory anti-dependency across iterations */
        p2[i] = f + (i > 0 ? p2[i-1] : 0);  /* WAR with next iteration's read */
        
        /* Output dependency */
        tmp3 = sum;                      /* First write */
        sum = tmp3 + f;                  /* Second write - WAW on sum through tmp3 */
        
        /* Force memory dependencies to stay */
        KEEP_ALIVE(p1[i]);
        KEEP_ALIVE(p2[i]);
    }
    
    /* Complex control flow at loop end to create more basic blocks */
    if (sum > 1000) {
        tmp1 = sum % 11;
        tmp2 = tmp1 * tmp1;
        sum = tmp2;
    }
    
    return sum;
}

/* Kernel 2: Arithmetic-heavy with nested loops and complex control flow */
NOINLINE static int kernel_arith(int *arr, int n) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int t1, t2, t3, t4, t5;
    
    for (int i = 0; i < n; ++i) {
        /* Start with a long RAW chain */
        t1 = arr[i] + i;
        t2 = t1 * t1;
        t3 = t2 - arr[i];
        t4 = t3 % 13;                    /* Higher latency */
        t5 = t4 / 5;                     /* Higher latency */
        
        /* Nested loop to create more complex DDG */
        for (int j = 0; j < 3; ++j) {
            /* Output dependency in nested loop */
            int local_tmp = t5 + j;
            
            /* RAW in nested loop */
            local_tmp = local_tmp * (j + 1);
            
            /* Anti-dependency with outer loop variable */
            int anti_tmp = t1;           /* Read t1 */
            t1 = anti_tmp + local_tmp;   /* Write t1 - WAR */
            
            acc1 += local_tmp;
        }
        
        /* Control flow splits */
        if (i % 3 == 0) {
            t2 = t5 + acc1;              /* WAW on t2 */
            acc2 = t2 * 2;
        } else if (i % 3 == 1) {
            t3 = acc1 - t5;              /* WAW on t3 */
            acc2 = t3 / 2;
        } else {
            t4 = acc1 | t5;              /* WAW on t4 */
            acc2 = t4 ^ 0xFF;
        }
        
        /* Another RAW chain with different operation types */
        int r1 = acc2 + 1;
        int r2 = r1 * r1;
        int r3 = r2 >> 2;
        int r4 = r3 & 0x3F;
        
        /* Cross-iteration dependency */
        acc3 = acc3 + r4 + (i > 0 ? arr[i-1] : 0);
        
        /* Keep variables live */
        KEEP_ALIVE(t1);
        KEEP_ALIVE(t5);
        KEEP_ALIVE(acc1);
    }
    
    /* Final computation with dependencies */
    t1 = acc1 + acc2;
    t2 = t1 * acc3;
    t3 = t2 % 17;
    
    return t3;
}

/* Kernel 3: Complex control flow with mixed dependencies */
NOINLINE static int kernel_control(int *arr1, int *arr2, int n) {
    int result = 0;
    int vec[4] = {0};
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks through switch/case */
        switch (i % 4) {
            case 0: {
                /* RAW chain with memory */
                int a = arr1[i] + 1;
                int b = a * arr2[i];
                arr1[i] = b;            /* WAW on memory */
                vec[0] = b + vec[3];    /* Loop-carried */
                break;
            }
            case 1: {
                /* Anti-dependencies */
                int read_a = arr1[i];   /* Read */
                int c = read_a % 7;
                arr1[i] = c + i;        /* Write - WAR */
                vec[1] = vec[0] + c;    /* RAW from previous iteration */
                break;
            }
            case 2: {
                /* Output dependencies */
                int tmp = vec[1];       /* First assignment */
                tmp = tmp * 3;          /* WAW */
                tmp = tmp - arr2[i];    /* WAW again */
                arr2[i] = tmp;
                vec[2] = tmp + vec[1];
                break;
            }
            case 3: {
                /* Mixed dependencies */
                int x = arr1[i] + arr2[i];
                int y = x / 2;
                int z = y | vec[2];
                
                /* Create WAR */
                int old_x = x;
                x = z + old_x;
                
                vec[3] = x + vec[2];
                break;
            }
        }
        
        /* Cross-case dependency */
        if (i % 2 == 0) {
            result += vec[0] - vec[2];
        } else {
            result += vec[1] + vec[3];
        }
        
        /* Force all vec elements to be live */
        KEEP_ALIVE(vec[0]);
        KEEP_ALIVE(vec[1]);
        KEEP_ALIVE(vec[2]);
        KEEP_ALIVE(vec[3]);
    }
    
    return result;
}

/* Simple PRNG to avoid library calls */
static uint32_t lcg = 123456789;
static uint32_t rand_lcg(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main(void) {
    /* Initialize data with pseudo-random values */
    int data[SIZE];
    int data2[SIZE];
    
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (int)(rand_lcg() % 1000);
        data2[i] = (int)(rand_lcg() % 1000);
    }
    
    /* Call kernels with complex dependency patterns */
    int r1 = kernel_mem(data, data2, ITERS);
    int r2 = kernel_arith(data, ITERS);
    int r3 = kernel_control(data, data2, ITERS);
    
    /* Combine results to prevent elimination */
    global_acc = r1 + r2 + r3;
    
    /* Volatile sink to ensure all computations are kept */
    sink = global_acc;
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(global_acc));
    
    return global_acc & 0xFF;
}
