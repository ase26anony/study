/* ddg_edge_trigger.c - Complex loop patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v) : "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic with complex control flow */
NOINLINE static int kernel_mem_aliasing(int *data, int n, int stride) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int *p1 = data;
    int *p2 = data + (n / 2);
    
    for (int i = 0; i < n; ++i) {
        /* True dependency chain with memory */
        int val1 = *p1;
        int val2 = val1 * 3;
        *p1 = val2 + i;  /* WAW on *p1 */
        
        /* Anti-dependency with potential aliasing */
        int read_before_write = *p2;
        *p2 = acc1 + i;  /* WAR on *p2 */
        acc1 += read_before_write;
        
        /* Complex control flow creating multiple basic blocks */
        if (i & 1) {
            /* Nested loop inside condition */
            int tmp = val2;
            for (int j = 0; j < 3; ++j) {
                tmp = (tmp * 17) % 7919;  /* High latency operation */
            }
            acc2 ^= tmp;
            
            /* Output dependency chain */
            int tmp2 = acc3;
            tmp2 = tmp2 * 2 + 1;  /* WAW on tmp2 */
            tmp2 = tmp2 / 3;      /* Another WAW */
            acc3 = tmp2;
        } else {
            /* Different dependency pattern in else branch */
            int tmp3 = acc2;
            tmp3 = (tmp3 << 3) | (tmp3 >> 29);  /* Rotation */
            acc2 = tmp3 + *p1;  /* RAW from *p1 above */
            
            /* Memory dependency with offset */
            int *p3 = p1 + (i % 8);
            int val3 = *p3;
            *p3 = val3 ^ acc3;  /* Potential WAW if p3 aliases p1 */
        }
        
        /* Pointer update with stride - may cause aliasing */
        p1 += stride;
        p2 += (stride * 2) % 7;
        
        /* Force values to stay live */
        KEEP_ALIVE(acc1);
        KEEP_ALIVE(acc2);
        KEEP_ALIVE(acc3);
    }
    
    return acc1 + acc2 * 3 - acc3;
}

/* Kernel 2: Arithmetic dependency chains with varying latencies */
NOINLINE static int kernel_arithmetic_chains(int *data, int n) {
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    int chain1 = 0, chain2 = 0, chain3 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long true dependency chain with mixed operations */
        int a = data[i] ^ r1;
        int b = a * r2 + i;      /* RAW on a */
        int c = b / (r3 + 1);    /* RAW on b, high latency */
        int d = c % 257;         /* RAW on c */
        int e = d << (i % 8);    /* RAW on d */
        chain1 = e ^ chain1;
        
        /* Parallel chain with anti-dependencies */
        int x = r4;
        r4 = chain2 + i;         /* WAR on r4 */
        int y = x * 2;           /* RAW on x */
        chain2 = y | r4;         /* RAW on y, WAR on chain2 */
        
        /* Chain with output dependencies and control */
        int tmp = r5;
        if ((i ^ data[i]) & 0xF) {
            tmp = tmp * 3 + 7;   /* WAW on tmp */
            tmp = tmp / 2;       /* Another WAW */
        } else {
            tmp = tmp + 11;      /* Different WAW path */
            tmp = tmp ^ 0xAAAA;  /* Follow-up WAW */
        }
        r5 = tmp;
        
        /* Loop-carried dependency with distance > 0 */
        int lc = data[(i + 1) % n] - data[(i + 2) % n];
        chain3 = chain3 * 13 + lc;  /* RAW on chain3 from previous iteration */
        
        /* Mix chains to create cross-dependencies */
        r1 = (r1 + chain1) % 1024;
        r2 = (r2 ^ chain2) | 1;
        r3 = (r3 * chain3) % 19 + 1;
        
        /* Volatile asm to prevent elimination */
        asm volatile("" : "+r"(chain1), "+r"(chain2), "+r"(chain3));
    }
    
    return chain1 + chain2 * 2 - chain3 * 3 + r5;
}

/* Kernel 3: Nested loops with complex data flow */
NOINLINE static int kernel_nested_control(int *data, int n) {
    int sum = 0;
    int prod = 1;
    int xor_acc = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Outer loop computations */
        int outer_val = data[i] + i;
        int *ptr = data + (i % 16);
        
        /* Nested loop with dependencies */
        int inner_sum = 0;
        for (int j = 0; j < 4; ++j) {
            /* True dependency within inner loop */
            int inner_tmp = outer_val + j;
            inner_tmp = inner_tmp * inner_tmp - j;  /* WAW */
            
            /* Anti-dependency with memory */
            int mem_read = *ptr;
            *ptr = inner_tmp;  /* WAR on *ptr */
            inner_sum += mem_read;
            
            /* Control-dependent computation */
            if (j & 1) {
                inner_sum = (inner_sum << 1) | 1;
            } else {
                inner_sum = inner_sum ^ 0x5555;
            }
        }
        
        /* Multiple assignment to same variable (output dep) */
        int result = inner_sum;
        result = result % 1009;      /* WAW */
        result = result * 3 + 7;     /* WAW */
        
        /* Update accumulators with different dependencies */
        if (i % 3 == 0) {
            sum = sum + result;      /* RAW on sum */
            prod = prod * (result | 1);  /* RAW on prod */
        } else if (i % 3 == 1) {
            sum = sum ^ result;      /* Different update to sum */
            xor_acc = xor_acc ^ result;  /* RAW on xor_acc */
        } else {
            int old_prod = prod;     /* Anti-dependency */
            prod = result + i;       /* WAR on prod */
            sum = sum + old_prod;    /* RAW on sum, RAW on old_prod */
        }
        
        /* Complex pointer arithmetic that may alias */
        ptr = data + ((i * 7) % n);
        *ptr = (*ptr + sum) % 256;   /* Memory dependency */
        
        /* Keep variables live across iterations */
        KEEP_ALIVE(sum);
        KEEP_ALIVE(prod);
        KEEP_ALIVE(xor_acc);
    }
    
    return sum + prod - xor_acc;
}

/* Simple PRNG to initialize data without library calls */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main(void) {
    int data[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call kernels with different access patterns */
    int result = 0;
    
    result += kernel_mem_aliasing(data, ITERS, 1);
    result += kernel_arithmetic_chains(data, ITERS);
    result += kernel_nested_control(data, ITERS / 2);
    
    /* Mix in some more complex patterns with different strides */
    result ^= kernel_mem_aliasing(data + SIZE/4, ITERS/2, 3);
    result ^= kernel_arithmetic_chains(data + SIZE/2, ITERS/3);
    
    /* Force result to be used */
    global_acc = result;
    sink = result;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result));
    
    return result & 0xFF;
}
