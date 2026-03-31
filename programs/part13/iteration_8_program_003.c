/* ddg_edge_trigger.c - Complex loop patterns to trigger DDG edge initialization */

#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory-heavy with pointer aliasing and complex control flow */
NOINLINE static void kernel_memory_aliasing(int *arr1, int *arr2, int n) {
    int i;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    
    /* Multiple live variables to prevent optimization */
    int live1 = 0, live2 = 0, live3 = 0, live4 = 0;
    int live5 = 0, live6 = 0, live7 = 0, live8 = 0;
    
    for (i = 0; i < n; ++i) {
        /* Basic block 1: True dependencies (RAW) with arithmetic chain */
        tmp1 = arr1[i] * 3;
        tmp2 = tmp1 + arr2[i];      /* RAW on tmp1 */
        tmp3 = tmp2 / 2;            /* RAW on tmp2 */
        
        /* Anti-dependency (WAR) pattern */
        int read_before = arr1[i + 1];  /* Read */
        arr1[i + 1] = tmp3;             /* Write to same location - WAR */
        acc1 += read_before;
        
        /* Output dependency (WAW) pattern */
        tmp4 = tmp3 * 2;
        /* ... other computations ... */
        tmp4 = acc1 + i;                /* WAW on tmp4 */
        
        /* Nested if-else creating multiple basic blocks */
        if (i & 1) {
            /* Branch 1: Memory operations with potential aliasing */
            int *p1 = &arr1[i];
            int *p2 = &arr1[(i * 7) % n];  /* May alias with p1 */
            
            *p1 = tmp4 + 5;                /* Write through p1 */
            tmp5 = *p2 + 3;                /* Read through p2 - MEM_DEP possible */
            
            /* Arithmetic chain with varying latency ops */
            live1 = tmp5 % 17;             /* High latency modulo */
            live2 = live1 * live1;
            live3 = live2 >> 4;
        } else {
            /* Branch 2: Different dependency pattern */
            tmp6 = arr2[i] - tmp4;         /* RAW on tmp4 */
            
            /* Another WAW */
            int tmp_var = tmp6 * 2;
            tmp_var = tmp_var + i;         /* WAW on tmp_var */
            
            /* Control-dependent computation */
            live4 = tmp_var & 0xFF;
            live5 = live4 | 0x80;
        }
        
        /* Loop-carried dependency (distance=1) */
        acc2 = acc2 + live3;               /* RAW on acc2 from previous iteration */
        
        /* Another loop-carried anti-dependency */
        int old_val = arr2[i];
        arr2[i] = acc2;                    /* WAR on arr2[i] across iterations */
        acc3 += old_val;
        
        /* Complex conditional with multiple basic blocks */
        if (i % 3 == 0) {
            live6 = acc3 * 7;
            if (i % 6 == 0) {
                live7 = live6 / 3;
                acc4 = live7 + i;
            } else {
                live8 = live6 % 5;
                acc4 = live8 - i;
            }
        } else if (i % 3 == 1) {
            live6 = acc3 + 11;
            acc4 = live6 * 2;
        } else {
            live6 = acc3 - 13;
            acc4 = live6 / 2;
        }
        
        /* Force all values to be live */
        KEEP_ALIVE(tmp1);
        KEEP_ALIVE(tmp2);
        KEEP_ALIVE(tmp3);
        KEEP_ALIVE(tmp4);
        KEEP_ALIVE(tmp5);
        KEEP_ALIVE(tmp6);
    }
    
    /* Feed results to global accumulator */
    global_acc += acc1 + acc2 + acc3 + acc4;
    KEEP_ALIVE(acc1);
    KEEP_ALIVE(acc2);
    KEEP_ALIVE(acc3);
    KEEP_ALIVE(acc4);
}

/* Kernel 2: Arithmetic-heavy with nested loops */
NOINLINE static void kernel_arithmetic_chains(double *data, int n) {
    int i, j;
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    double prod1 = 1.0, prod2 = 1.0;
    
    /* Many live variables */
    double t1, t2, t3, t4, t5, t6, t7, t8;
    
    for (i = 0; i < n; ++i) {
        /* Long arithmetic chain with mixed operations */
        t1 = data[i] * 1.5;               /* Multiplication */
        t2 = t1 + 2.7;                    /* RAW on t1 */
        t3 = t2 / 1.3;                    /* RAW on t2 */
        t4 = t3 - 0.8;                    /* RAW on t3 */
        t5 = t4 * t4;                     /* RAW on t4 */
        
        /* Nested inner loop creating complex DDG */
        for (j = 0; j < 3; ++j) {
            /* Output dependency in inner loop */
            double inner_tmp = t5 + j;
            /* ... computations ... */
            inner_tmp = inner_tmp * 0.5;   /* WAW on inner_tmp */
            
            /* Anti-dependency with array */
            double read_val = data[j];
            data[j] = inner_tmp;           /* WAR on data[j] */
            sum1 += read_val;
        }
        
        /* Conditional with different latency operations */
        if (i % 4 == 0) {
            t6 = t5 * 3.14159;            /* High precision constant */
            t7 = t6 / 2.71828;            /* Division - higher latency */
        } else {
            t6 = t5 + 1.414;              /* Addition - lower latency */
            t7 = t6 - 0.707;              /* Subtraction */
        }
        
        /* Loop-carried dependency through sum2 */
        sum2 = sum2 * 0.99 + t7;          /* RAW on sum2 from prev iteration */
        
        /* Another dependency chain */
        t8 = sum2 * data[i % 8];
        prod1 *= t8;                      /* RAW on prod1 */
        
        /* Complex condition */
        if ((i & 7) == 0) {
            prod2 /= (t8 + 1.0);          /* Division - high latency */
        }
        
        KEEP_ALIVE(t1);
        KEEP_ALIVE(t2);
        KEEP_ALIVE(t3);
        KEEP_ALIVE(t4);
        KEEP_ALIVE(t5);
        KEEP_ALIVE(t6);
        KEEP_ALIVE(t7);
        KEEP_ALIVE(t8);
    }
    
    /* Use results */
    double final = sum1 + sum2 + sum3 + prod1 + prod2;
    global_acc += (int)final;
    KEEP_ALIVE(final);
}

/* Kernel 3: Complex control flow with mixed dependencies */
NOINLINE static void kernel_control_flow(int *a, int *b, int *c, int n) {
    int i;
    int x = 0, y = 0, z = 0;
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
    
    for (i = 1; i < n - 1; ++i) {
        /* Multiple interleaved dependency chains */
        int val1 = a[i - 1];              /* Loop-carried RAW (distance=1) */
        int val2 = b[i];
        int val3 = c[i + 1];              /* Loop-carried RAW (distance=1) */
        
        /* Chain 1 */
        r1 = val1 * 3;
        r2 = r1 + val2;                   /* RAW on r1 */
        
        /* Chain 2 with anti-dependency */
        int temp = a[i];                  /* Read */
        a[i] = r2;                        /* Write - WAR on a[i] */
        r3 = temp + val3;
        
        /* Multi-way conditional creating many basic blocks */
        switch (i % 5) {
            case 0:
                r4 = r3 << 2;             /* RAW on r3 */
                r5 = r4 | 0xF;
                break;
            case 1:
                r4 = r3 >> 1;             /* RAW on r3 */
                r5 = r4 & 0x7F;
                break;
            case 2:
                r4 = r3 * 7;              /* RAW on r3 */
                r5 = r4 % 19;             /* High latency modulo */
                break;
            case 3:
                r4 = r3 + 11;             /* RAW on r3 */
                r5 = r4 ^ 0xAA;
                break;
            default:
                r4 = r3 - 13;             /* RAW on r3 */
                r5 = r4 * 3;
                break;
        }
        
        /* Output dependencies */
        int output_var = r5;
        if (i % 3 == 0) {
            output_var = output_var + i;  /* WAW on output_var */
        } else {
            output_var = output_var - i;  /* WAW on output_var */
        }
        
        /* Loop-carried through arrays */
        b[i] = output_var;                /* Write */
        x += a[i - 1];                    /* Read - loop-carried RAW on a */
        
        /* Another loop-carried chain */
        y = y * 2 + output_var;           /* RAW on y from prev iteration */
        
        /* Complex condition with nested if */
        if (i % 7 == 0) {
            if (output_var > 100) {
                z = z + output_var;       /* RAW on z */
            } else {
                z = z - output_var;       /* RAW on z */
            }
        } else if (i % 7 == 3) {
            z = z * 2;                    /* RAW on z */
        }
        
        KEEP_ALIVE(r1);
        KEEP_ALIVE(r2);
        KEEP_ALIVE(r3);
        KEEP_ALIVE(r4);
        KEEP_ALIVE(r5);
        KEEP_ALIVE(r6);
    }
    
    global_acc += x + y + z;
    KEEP_ALIVE(x);
    KEEP_ALIVE(y);
    KEEP_ALIVE(z);
}

/* Simple PRNG to initialize data without library calls */
static int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main(void) {
    int data_int[SIZE];
    double data_double[SIZE];
    int data_b[SIZE];
    int data_c[SIZE];
    
    int seed = 42;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data_int[i] = simple_rand(&seed) % 1000;
        data_double[i] = (simple_rand(&seed) % 1000) / 10.0;
        data_b[i] = simple_rand(&seed) % 1000;
        data_c[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call kernels with complex dependency patterns */
    kernel_memory_aliasing(data_int, data_b, ITERS);
    kernel_arithmetic_chains(data_double, ITERS);
    kernel_control_flow(data_int, data_b, data_c, ITERS);
    
    /* Use result to prevent dead code elimination */
    sink = global_acc;
    KEEP_ALIVE(sink);
    
    return global_acc & 0xFF;
}
