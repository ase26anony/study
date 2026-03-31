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

/* Kernel 1: Memory aliasing and pointer arithmetic with WAR/WAW dependencies */
NOINLINE static void kernel_memory_aliasing(int* arr, int n, int* result) {
    int* p1 = arr;
    int* p2 = arr + n/2;
    int* p3 = arr + n/4;
    
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency (RAW) chain */
            int val1 = *p1;
            tmp1 = val1 * 2;
            tmp2 = tmp1 + i;
            
            /* Anti-dependency (WAR) - read before write */
            int old_p2 = *p2;
            *p2 = tmp2;
            sum += old_p2;
            
            /* Output dependency (WAW) on tmp3 */
            tmp3 = val1 % 17;
        } else {
            /* Different dependency pattern in else branch */
            int val3 = *p3;
            tmp3 = val3 / 3;  /* WAW on tmp3 from different basic block */
            
            /* Memory dependency with possible aliasing */
            *p1 = val3 + i;
            sum += *p2;  /* Could alias with p1 */
            
            /* Another RAW chain */
            tmp1 = val3 * val3;
            tmp2 = tmp1 - i;
        }
        
        /* Nested loop to create more complex control flow */
        for (int j = 0; j < 3; ++j) {
            /* Complex arithmetic with varying latency */
            if ((i + j) & 2) {
                tmp3 = (tmp3 * 7) % 13;  /* High latency modulo */
                sum += tmp3;
            } else {
                tmp3 = tmp3 + j;  /* Low latency add */
            }
        }
        
        /* Pointer updates with wrap-around */
        p1 = (p1 == arr + n - 1) ? arr : p1 + 1;
        p2 = (p2 == arr + n - 1) ? arr : p2 + 1;
        p3 = (p3 == arr + n - 1) ? arr : p3 + 1;
        
        /* Keep variables live */
        KEEP_ALIVE(tmp1);
        KEEP_ALIVE(tmp2);
        KEEP_ALIVE(tmp3);
    }
    
    *result = sum;
    global_acc += sum;
}

/* Kernel 2: Arithmetic chains with control dependencies and loop-carried deps */
NOINLINE static void kernel_arithmetic_chains(int* arr, int n, int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int sum = 0;
    
    /* Loop-carried dependencies with distance > 0 */
    for (int i = 0; i < n; ++i) {
        int t1, t2, t3, t4;
        
        /* Multiple parallel dependency chains */
        t1 = a * i;      /* RAW: used below */
        t2 = b + t1;     /* RAW on t1 */
        t3 = c - t2;     /* RAW on t2 */
        
        /* Output dependencies (WAW) */
        a = t3;          /* WAW on a */
        
        /* Anti-dependencies (WAR) */
        int old_b = b;   /* Read b before write */
        b = t1 + old_b;  /* Write b */
        sum += old_b;    /* Use old value */
        
        /* Control-dependent computations */
        if (i % 7 == 0) {
            t4 = d * 3;      /* High latency multiply */
            e = t4 / 2;      /* High latency divide */
        } else if (i % 5 == 0) {
            t4 = e + 11;     /* Different latency */
            e = t4 % 7;      /* Modulo operation */
        } else {
            t4 = d + e;
            e = t4 * 2;
        }
        
        /* Cross-iteration dependencies */
        d = c + i;      /* Loop-carried to next iteration */
        c = t3 + arr[i % 8];  /* Memory access with small stride */
        
        /* Complex expression with many operands */
        sum += (a * b + c * d - e) % 31;
        
        /* Volatile asm to prevent elimination */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e));
    }
    
    *result = sum;
    global_acc += sum;
}

/* Kernel 3: Mixed dependencies with nested loops and function calls */
NOINLINE static int helper_compute(int x, int y) {
    /* Force no-inline */
    asm volatile("");
    return (x * y) % 19;
}

NOINLINE static void kernel_mixed_patterns(int* arr, int n, int* result) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp[8] = {0};
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks with switch */
        switch (i & 3) {
            case 0:
                /* Memory dependencies with aliasing */
                tmp[0] = arr[i];
                tmp[1] = arr[i + 1];
                acc1 += tmp[0] * tmp[1];
                
                /* WAR on arr */
                int old_val = arr[i % 8];
                arr[i % 8] = acc1;
                acc2 += old_val;
                break;
                
            case 1:
                /* Long RAW chain */
                int x1 = acc1 + i;
                int x2 = x1 * 2;
                int x3 = x2 - acc2;
                int x4 = x3 % 17;
                int x5 = x4 + x1;
                acc3 = x5;
                break;
                
            case 2:
                /* Function call creates control flow */
                acc1 = helper_compute(acc1, i);
                
                /* WAW on acc2 */
                acc2 = acc3 + i;
                acc2 = acc1 * 2;  /* Second write to acc2 */
                break;
                
            default:
                /* Complex nested loop */
                for (int k = 0; k < 4; ++k) {
                    acc3 += (acc1 >> k) & 1;
                    tmp[k] = acc3 + k;
                }
                break;
        }
        
        /* Loop-carried output dependency */
        static int persistent = 0;
        persistent = acc1 + acc2 + acc3;
        sum += persistent;
        
        /* Keep all accumulators live */
        KEEP_ALIVE(acc1);
        KEEP_ALIVE(acc2);
        KEEP_ALIVE(acc3);
    }
    
    /* Final reduction with dependencies */
    for (int i = 0; i < 8; ++i) {
        sum += tmp[i];
        tmp[i] = sum;  /* WAR then WAW in same statement */
    }
    
    *result = sum;
    global_acc += sum;
}

/* Simple PRNG to avoid library calls */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    int data[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 100;
    }
    
    int res1, res2, res3;
    
    /* Call kernels with different access patterns */
    kernel_memory_aliasing(data, ITERATIONS, &res1);
    kernel_arithmetic_chains(data + SIZE/4, ITERATIONS, &res2);
    kernel_mixed_patterns(data + SIZE/2, ITERATIONS, &res3);
    
    /* Combine results to prevent dead code elimination */
    int final_result = res1 + res2 + res3 + global_acc;
    
    /* Force output to be used */
    sink = final_result;
    
    /* Return something based on computation */
    return (final_result > 0) ? 0 : 1;
}
