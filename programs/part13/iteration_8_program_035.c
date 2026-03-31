/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v) : "memory")

/* Kernel 1: Memory-heavy with pointer aliasing and WAR/WAW dependencies */
NOINLINE static int kernel_mem_aliasing(int *arr, int n, int stride) {
    int sum = 0;
    int *p1, *p2;
    int tmp1, tmp2, tmp3;
    
    for (int i = 0; i < n; i++) {
        /* Multiple basic blocks created by if-else */
        if (i & 1) {
            /* True dependency chain with memory */
            p1 = &arr[(i * stride) % SIZE];
            p2 = &arr[((i * stride) + 1) % SIZE];
            
            /* RAW: Read after write */
            tmp1 = *p1;
            *p1 = tmp1 + i;          /* WAW potential on *p1 */
            
            /* WAR: Write after read with possible aliasing */
            tmp2 = *p2;
            *p1 = tmp2 * 2;          /* WAW on *p1, WAR if p1==p2 */
            
            /* Complex chain */
            tmp3 = tmp1 + tmp2;
            sum += tmp3;
            
            /* Output dependency (WAW) */
            tmp1 = sum & 0xFF;       /* WAW on tmp1 */
        } else {
            /* Different path with nested loop for control complexity */
            int inner_sum = 0;
            for (int j = 0; j < 3; j++) {
                /* Memory anti-dependency */
                int idx = (i + j) % SIZE;
                int val = arr[idx];   /* Read */
                arr[idx] = val * j;   /* Write - WAR */
                inner_sum += arr[idx];
            }
            
            /* Cross-iteration dependency */
            static int cross_iter = 0;
            cross_iter = cross_iter + inner_sum;  /* Loop-carried WAW */
            sum += cross_iter;
        }
        
        /* Control-dependent computation */
        int ctrl_var;
        if (sum & 0x100) {
            ctrl_var = arr[i % SIZE] / 3;  /* Higher latency divide */
        } else {
            ctrl_var = arr[i % SIZE] % 7;  /* Different higher latency op */
        }
        
        /* Force multiple live variables */
        KEEP_ALIVE(tmp1);
        KEEP_ALIVE(tmp2);
        KEEP_ALIVE(tmp3);
        sum += ctrl_var;
    }
    
    KEEP_ALIVE(sum);
    return sum;
}

/* Kernel 2: Arithmetic chains with varying latencies and register pressure */
NOINLINE static int kernel_arithmetic_chains(int *arr, int n) {
    /* Many live variables to increase register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int acc = 0;
    
    for (int i = 0; i < n; i++) {
        /* Long true dependency chain (RAW) */
        a = arr[i % SIZE] + b;
        c = a * d;          /* Uses a */
        e = c / (b + 1);    /* Higher latency divide, uses c */
        f = e - a;          /* Uses e and a */
        
        /* Parallel chain with cross-iteration dependencies */
        g = g + arr[(i + 1) % SIZE];  /* Loop-carried RAW on g */
        h = h * 2 - g;                /* Loop-carried RAW on h, uses g */
        
        /* Anti-dependencies (WAR) */
        int old_b = b;      /* Read b */
        b = f + g;          /* Write b - WAR */
        acc += old_b;
        
        /* Output dependencies (WAW) */
        d = a + c;          /* Write d */
        d = h - e;          /* WAW on d - different value */
        
        /* Complex conditional with nested loop */
        if (i % 8 == 0) {
            int inner_acc = 0;
            for (int j = 0; j < 4; j++) {
                inner_acc += arr[(i + j) % SIZE] % (j + 2);  /* High latency mod */
            }
            d = inner_acc;  /* WAW on d again */
        } else if (i % 8 == 4) {
            d = d * d;      /* Different WAW on d */
        }
        
        /* Mix in memory operations */
        arr[(i * 3) % SIZE] = acc;
        int mem_val = arr[(i * 7) % SIZE];  /* Potential WAR with above */
        acc += mem_val;
        
        /* Keep all variables live */
        KEEP_ALIVE(a); KEEP_ALIVE(b); KEEP_ALIVE(c);
        KEEP_ALIVE(d); KEEP_ALIVE(e); KEEP_ALIVE(f);
        KEEP_ALIVE(g); KEEP_ALIVE(h);
    }
    
    global_acc += acc;
    KEEP_ALIVE(acc);
    return acc;
}

/* Kernel 3: Complex control flow with mixed dependencies */
NOINLINE static int kernel_control_flow(int *arr, int n, int *out) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int t1, t2, t3, t4;
    
    for (int i = 1; i < n; i++) {
        /* Switch-like control flow creating multiple basic blocks */
        switch (i % 4) {
            case 0:
                /* Memory chain with pointer arithmetic */
                int *ptr1 = &arr[i % SIZE];
                int *ptr2 = &arr[(i + 1) % SIZE];
                
                t1 = *ptr1;
                *ptr2 = t1 + i;      /* WAR if ptr1 == ptr2 */
                t2 = *ptr1;          /* RAW on *ptr1 */
                sum1 += t2;
                break;
                
            case 1:
                /* Arithmetic chain with loop-carried dependency */
                static int carry = 0;
                t3 = arr[i % SIZE] % 5;  /* High latency */
                carry = carry + t3;       /* Loop-carried WAW */
                sum2 += carry;
                break;
                
            case 2:
                /* Nested loops with dependencies */
                for (int k = 0; k < 2; k++) {
                    t4 = arr[(i + k) % SIZE];
                    out[k] = t4 * k;      /* WAW on out[k] across iterations */
                    sum3 += out[k];
                }
                break;
                
            case 3:
                /* Mixed dependencies */
                int idx = i % SIZE;
                int val = arr[idx];       /* Read */
                arr[idx] = sum1 + sum2;   /* Write - WAR */
                sum3 = val + sum3;        /* RAW on sum3 */
                break;
        }
        
        /* Cross-case dependencies */
        if (i % 3 == 0) {
            sum1 = sum2 + sum3;  /* WAW on sum1 */
        } else {
            sum2 = sum1 - sum3;  /* WAW on sum2 */
        }
        
        /* Force control and data dependency merge */
        int cond = arr[i % SIZE] > 0;
        int res = cond ? sum1 : sum2;
        out[i % 16] = res;  /* Memory WAW */
        
        KEEP_ALIVE(t1); KEEP_ALIVE(t2); KEEP_ALIVE(t3); KEEP_ALIVE(t4);
    }
    
    return sum1 + sum2 + sum3;
}

/* Simple PRNG to initialize data without library calls */
static inline int lcg_rand(int *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

int main(void) {
    /* Large arrays to work with */
    int data[SIZE];
    int out[SIZE];
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        data[i] = lcg_rand(&seed) % 1000;
        out[i] = 0;
    }
    
    /* Call kernels with different dependency patterns */
    int result = 0;
    
    /* Kernel 1: Memory aliasing patterns */
    result += kernel_mem_aliasing(data, ITERS, 7);
    
    /* Kernel 2: Arithmetic chains */
    result += kernel_arithmetic_chains(data, ITERS);
    
    /* Kernel 3: Complex control flow */
    result += kernel_control_flow(data, ITERS, out);
    
    /* Add global accumulator */
    result += global_acc;
    
    /* Final volatile sink to prevent DCE */
    sink = result;
    KEEP_ALIVE(result);
    
    return result & 0xFF;
}
