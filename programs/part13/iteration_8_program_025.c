/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
NOINLINE static int kernel_mem_aliasing(int *data, int *data2, int n) {
    int sum = 0;
    int *p1, *p2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if/else */
        if (i & 1) {
            /* True dependency (RAW) with memory */
            int temp = data[i];
            data[i] = temp * 2 + i;  /* WAW on data[i] */
            sum += data[i];
            
            /* Anti-dependency (WAR) with pointer aliasing */
            p1 = &data[i % SIZE];
            p2 = &data2[(i * 7) % SIZE];
            int read_val = *p1;      /* Read before write - anti-dep potential */
            *p2 = read_val + sum;    /* Write after read */
            
            /* Complex arithmetic chain with varying latencies */
            int a = sum % 13;        /* Higher latency operation */
            int b = a / 3;           /* Another high latency op */
            int c = b + i;           /* Low latency */
            int d = c * 2;           /* Medium latency */
            sum = d - a;             /* Output dependency on sum (WAW) */
        } else {
            /* Different path with nested loop for more basic blocks */
            int acc = 0;
            for (int j = 0; j < 3; ++j) {
                /* Loop-carried dependency within nested loop */
                acc = acc + data[(i + j) % SIZE] * j;
                
                /* Memory dependency with potential aliasing */
                data2[(i + j) % SIZE] = acc + data[i];
            }
            
            /* Mix of operations with different latencies */
            int x = acc;
            int y = x * x % 17;      /* Expensive operation */
            int z = y + (x >> 3);    /* Cheap operation */
            sum += z;
            
            /* Output dependency chain */
            int tmp = x;
            tmp = y;                 /* WAW on tmp */
            tmp = z;                 /* Another WAW */
            KEEP_ALIVE(tmp);
        }
        
        /* Control-dependent computation */
        int cond = i % 3;
        switch (cond) {
            case 0: {
                /* Memory anti-dependency pattern */
                int old = data[i];
                data[i] = sum + i;   /* WAR: data[i] written after read */
                sum += old;
                break;
            }
            case 1: {
                /* Arithmetic true dependency chain */
                int t1 = sum + i;
                int t2 = t1 * t1;    /* RAW on t1 */
                int t3 = t2 % 19;    /* RAW on t2 */
                sum = t3 - t1;       /* RAW on t3, WAW on sum */
                break;
            }
            default: {
                /* Pointer chasing with dependencies */
                int *ptr = &data[i % SIZE];
                int val = *ptr;
                *ptr = val + 1;      /* WAR on *ptr */
                sum += val;
                break;
            }
        }
        
        /* Force value to be live across iteration */
        KEEP_ALIVE(sum);
    }
    
    return sum;
}

/* Kernel 2: Arithmetic dependency chains with complex control flow */
NOINLINE static int kernel_arithmetic_chains(int *data, int n) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int *ptr_arr[SIZE];
    
    /* Initialize pointer array */
    for (int i = 0; i < SIZE && i < n; ++i) {
        ptr_arr[i] = &data[i];
    }
    
    for (int i = 1; i < n; ++i) {
        /* Multiple parallel dependency chains */
        
        /* Chain 1: Integer arithmetic with varying latencies */
        int a = data[i];
        int b = a * 3 + r1;          /* RAW on a and r1 */
        int c = b / 7;               /* High latency, RAW on b */
        r1 = c - a;                  /* WAW on r1, RAW on c and a */
        
        /* Chain 2: Modulo operations (high latency) */
        int x = r2;
        int y = x % 23 + i;          /* RAW on x */
        int z = y * y % 31;          /* RAW on y, high latency */
        r2 = z + x;                  /* WAW on r2, RAW on z and x */
        
        /* Chain 3: Mixed operations with control flow */
        if (i % 4 == 0) {
            int t1 = r3;
            int t2 = t1 + data[i-1]; /* RAW on t1 and memory */
            int t3 = t2 * 2;         /* RAW on t2 */
            r3 = t3;                 /* WAW on r3 */
        } else if (i % 4 == 1) {
            int t1 = r3;
            int t2 = t1 - data[i-1]; /* Different operation pattern */
            int t3 = t2 / 5;         /* High latency */
            r3 = t3 + t1;            /* WAW on r3 */
        } else {
            r3 = r3 * 3 + i;         /* WAW on r3 */
        }
        
        /* Chain 4: Pointer-based dependencies */
        int *p = ptr_arr[i % SIZE];
        int read_val = *p;           /* Memory read */
        *p = read_val + r4;          /* WAR on *p, RAW on r4 */
        r4 = read_val * 2;           /* WAW on r4, RAW on read_val */
        
        /* Chain 5: Long dependency chain */
        int v1 = r5;
        int v2 = v1 + i;
        int v3 = v2 * v1;            /* RAW on v2 and v1 */
        int v4 = v3 % 41;            /* High latency, RAW on v3 */
        int v5 = v4 - v2;            /* RAW on v4 and v2 */
        int v6 = v5 / 3;             /* High latency, RAW on v5 */
        r5 = v6 + v1;                /* WAW on r5, RAW on v6 and v1 */
        
        /* Cross-chain dependencies to create complex graph */
        if (i % 5 == 0) {
            r1 = r1 + r2;            /* RAW on r1 and r2, WAW on r1 */
            r3 = r3 - r4;            /* RAW on r3 and r4, WAW on r3 */
        }
        
        /* Use all results to keep them live */
        data[i] = r1 + r2 + r3 + r4 + r5;
        KEEP_ALIVE(data[i]);
    }
    
    return r1 + r2 + r3 + r4 + r5;
}

/* Kernel 3: Complex control flow with nested loops */
NOINLINE static int kernel_control_flow(int *data, int *data2, int n) {
    int total = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Outer loop with multiple basic blocks */
        int acc = data[i];
        
        /* Nested loop creating inner loop-carried dependencies */
        for (int j = 0; j < 4; ++j) {
            /* True dependency within nested loop */
            int tmp = acc;
            acc = tmp + data2[(i + j) % SIZE] * j;
            
            /* Anti-dependency with memory */
            int read = data[(i + j) % SIZE];
            data[(i + j) % SIZE] = acc + j;  /* WAR on data[] */
            
            /* Output dependency */
            int var = read;
            var = var * 2;            /* WAW on var */
            var = var + 1;            /* Another WAW */
            KEEP_ALIVE(var);
        }
        
        /* Switch with multiple cases creating different dependency patterns */
        switch (i % 5) {
            case 0: {
                /* Memory intensive */
                int *p1 = &data[i];
                int *p2 = &data2[i];
                int val1 = *p1;
                int val2 = *p2;
                *p1 = val1 + val2;    /* WAR on *p1 */
                *p2 = val1 - val2;    /* WAR on *p2 */
                total += val1 * val2;
                break;
            }
            case 1: {
                /* Arithmetic chain */
                int x = total;
                int y = x * x % 53;   /* High latency */
                int z = y + x;
                int w = z / 7;        /* High latency */
                total = w - x;        /* WAW on total */
                break;
            }
            case 2: {
                /* Mixed dependencies */
                int a = data[i];
                int b = a + total;
                data[i] = b;          /* WAW on data[i] */
                total = total + a;    /* RAW on total and a, WAW on total */
                break;
            }
            case 3: {
                /* Complex expression with multiple writes */
                int t = data[i];
                data[i] = t + i;      /* WAW on data[i] */
                t = data[i] * 2;      /* WAW on t, RAW on data[i] */
                t = t % 29;           /* WAW on t */
                total += t;
                break;
            }
            default: {
                /* Pointer aliasing scenario */
                int *ptr1 = &data[i % SIZE];
                int *ptr2 = &data[(i * 3 + 1) % SIZE];
                int v1 = *ptr1;
                int v2 = *ptr2;
                *ptr1 = v1 + v2;      /* WAR on *ptr1 */
                *ptr2 = v1 - v2;      /* WAR on *ptr2 */
                total = total + v1 * v2;  /* RAW on total, WAW on total */
                break;
            }
        }
        
        /* Loop-carried dependency with distance > 0 */
        if (i >= 2) {
            data[i] += data[i-2];     /* Loop-carried RAW with distance 2 */
        }
        
        KEEP_ALIVE(total);
        KEEP_ALIVE(acc);
    }
    
    return total;
}

/* Simple PRNG to avoid library calls */
static uint32_t simple_rand(uint32_t *seed) {
    *seed = *seed * 1103515245 + 12345;
    return *seed;
}

int main(void) {
    int data[SIZE];
    int data2[SIZE];
    uint32_t seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 1000;
        data2[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call all kernels to maximize DDG construction opportunities */
    int result = 0;
    
    result += kernel_mem_aliasing(data, data2, ITERATIONS);
    KEEP_ALIVE(result);
    
    result += kernel_arithmetic_chains(data, ITERATIONS);
    KEEP_ALIVE(result);
    
    result += kernel_control_flow(data, data2, ITERATIONS);
    KEEP_ALIVE(result);
    
    /* Use result to prevent dead code elimination */
    sink = result;
    
    return result & 255;  /* Return non-zero to indicate execution */
}
