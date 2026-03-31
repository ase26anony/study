/* ddg_edge_trigger.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Specifically targets ddg.cc lines 749-757 (ddg_edge initialization)
 */

#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" ::: "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic with complex control flow */
NOOPT static void kernel_memory_aliasing(int *data, int n, int *result) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    
    /* Two pointers that may alias */
    int *p1 = data;
    int *p2 = data + (n / 2);
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by complex if-else chain */
        if (i & 1) {
            /* Block A: Memory RAW and WAR dependencies */
            tmp1 = *p1;                     /* Read */
            *p1 = tmp1 + i;                 /* Write - WAR with above */
            tmp2 = *p2;                     /* Read - may alias with p1 */
            
            /* Arithmetic chain with true dependencies */
            tmp3 = tmp1 * tmp2;
            tmp4 = tmp3 / (i + 1);          /* High latency operation */
            acc1 += tmp4;
            
            /* Output dependency (WAW) */
            tmp5 = acc1;
            if (tmp5 > 1000) {
                tmp5 = tmp5 % 1000;         /* Another WAW on tmp5 */
            }
            
            /* Pointer update with distance > 0 */
            p1 += (i % 3) - 1;
            p2 += (i % 5) - 2;
        } else {
            /* Block B: Different dependency pattern */
            /* Anti-dependency (WAR) pattern */
            int old_val = *p2;
            *p2 = old_val * 2;              /* WAR: old_val read before p2 written */
            
            /* Output dependencies */
            int compute = i * i;
            compute = compute + old_val;    /* WAW on compute */
            
            /* Memory true dependency with distance */
            *p1 = compute;
            int verify = *p1;               /* RAW from previous store */
            
            acc2 += verify;
            
            /* Complex conditional nesting */
            if (i % 7 == 0) {
                /* Nested loop inside main loop */
                int inner = 0;
                for (int j = 0; j < 3; ++j) {
                    inner += data[(i + j) % SIZE];
                }
                acc3 += inner;
            }
        }
        
        /* Control dependencies based on loop variant */
        if (i % 11 == 0) {
            /* Another basic block */
            tmp1 = acc1 + acc2;
            tmp2 = tmp1 - acc3;             /* True dependency */
            acc1 = tmp2 % 997;              /* WAW on acc1 */
        } else if (i % 13 == 0) {
            /* Yet another block with different operations */
            float ftmp = (float)acc2;
            int itmp = (int)(ftmp * 0.5f);  /* Type conversion adds complexity */
            acc3 = itmp;                    /* WAW on acc3 */
        }
        
        /* Force all values to stay live */
        KEEP(acc1); KEEP(acc2); KEEP(acc3);
        KEEP(tmp1); KEEP(tmp2); KEEP(tmp3); KEEP(tmp4); KEEP(tmp5);
    }
    
    *result = acc1 + acc2 + acc3;
    MEM_BARRIER();
}

/* Kernel 2: Arithmetic dependency chains with varying latencies */
NOOPT static void kernel_arithmetic_chains(int *data, int n, int *result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, j = 9, k = 10;
    
    for (int i = 0; i < n; ++i) {
        /* Long arithmetic chain with true dependencies */
        a = b + data[i];                    /* RAW: b */
        b = c * a;                          /* RAW: c, a */
        c = d - b;                          /* RAW: d, b */
        d = e / (c + 1);                    /* RAW: e, c - high latency */
        e = f % (d + 2);                    /* RAW: f, d - high latency */
        
        /* Parallel chain with anti-dependencies */
        int tmp_f = f;                      /* Read f */
        f = g + i;                          /* Write f - WAR with above */
        int tmp_g = g;                      /* Read g */
        g = h * tmp_f;                      /* Write g - WAR, RAW on tmp_f */
        
        /* Output dependencies */
        int common = h + j;
        if (i & 2) {
            common = common * 3;            /* WAW on common */
        } else {
            common = common / 2;            /* WAW on common (different path) */
        }
        
        h = common + k;
        j = h - tmp_g;                      /* RAW on h, WAR on tmp_g */
        k = j * data[(i * 17) % SIZE];      /* RAW on j */
        
        /* Complex conditional with nested loops */
        if (i % 19 == 0) {
            /* Inner loop with carried dependencies */
            int inner_acc = 0;
            for (int m = 0; m < 4; ++m) {
                inner_acc += data[(i + m) % SIZE] * m;
                /* Loop-carried dependency */
                data[(i + m) % SIZE] = inner_acc;
            }
            e += inner_acc;                 /* WAW on e */
        }
        
        /* Keep all variables live */
        KEEP(a); KEEP(b); KEEP(c); KEEP(d); KEEP(e);
        KEEP(f); KEEP(g); KEEP(h); KEEP(j); KEEP(k);
    }
    
    *result = a + b + c + d + e + f + g + h + j + k;
    MEM_BARRIER();
}

/* Kernel 3: Complex control flow with mixed dependencies */
NOOPT static void kernel_control_flow(int *data, int n, int *result) {
    int x = 0, y = 0, z = 0;
    int t1, t2, t3, t4, t5, t6, t7, t8;
    
    for (int i = 0; i < n; ++i) {
        /* Switch-like control flow */
        switch (i % 5) {
            case 0:
                /* Memory dependencies with aliasing */
                t1 = data[i];
                data[(i * 3) % SIZE] = t1 + y;  /* WAR on t1 */
                t2 = data[(i * 3) % SIZE];      /* RAW from previous store */
                x = t2 * 2;
                break;
            case 1:
                /* Arithmetic chain */
                t3 = x + y;
                t4 = t3 - z;                    /* RAW on t3 */
                t5 = t4 % 17;                   /* High latency */
                y = t5;
                break;
            case 2:
                /* Output dependencies */
                t6 = x * y;
                t6 = t6 + z;                    /* WAW on t6 */
                t7 = data[(i * 7) % SIZE];
                data[(i * 11) % SIZE] = t6 + t7;
                z = t7;
                break;
            case 3:
                /* Anti-dependencies */
                int old_x = x;                  /* Read x */
                x = old_x + data[i];            /* Write x - WAR */
                int old_y = y;                  /* Read y */
                y = old_y * old_x;              /* Write y - WAR, RAW on old_x */
                break;
            default:
                /* Complex nested if */
                if (i % 3 == 0) {
                    t8 = x / (y + 1);           /* High latency, RAW on x, y */
                    for (int p = 0; p < 2; ++p) {
                        t8 += data[(i + p) % SIZE];
                    }
                    z = t8;
                } else {
                    z = x + y;
                }
                break;
        }
        
        /* Loop-carried dependency with distance > 0 */
        if (i > 10) {
            data[i % SIZE] = data[(i - 10) % SIZE] + x;
        }
        
        /* Force values to stay in registers */
        KEEP(x); KEEP(y); KEEP(z);
        KEEP(t1); KEEP(t2); KEEP(t3); KEEP(t4);
        KEEP(t5); KEEP(t6); KEEP(t7); KEEP(t8);
    }
    
    *result = x + y + z;
    MEM_BARRIER();
}

/* Simple PRNG to avoid library dependencies */
static int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    int data[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 1000;
    }
    
    int res1, res2, res3;
    
    /* Call kernels with different access patterns */
    kernel_memory_aliasing(data, ITERS, &res1);
    kernel_arithmetic_chains(data + SIZE/4, ITERS, &res2);
    kernel_control_flow(data + SIZE/2, ITERS, &res3);
    
    /* Combine results and force output */
    int final_result = res1 + res2 + res3;
    global_acc += final_result;
    
    /* Volatile sink to prevent dead code elimination */
    sink = global_acc;
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(final_result));
    
    return final_result & 0xFF;
}
