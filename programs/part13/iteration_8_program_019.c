/* Complex dependency pattern generator for DDG edge coverage */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing with complex control flow */
NOINLINE static void kernel_memory_aliasing(int* arr1, int* arr2, int n) {
    int i;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency chain (RAW) */
            tmp1 = arr1[i] + arr2[i];          /* Node A */
            tmp2 = tmp1 * 3;                   /* Node B depends on A */
            tmp3 = tmp2 / 2;                   /* Node C depends on B */
            
            /* Anti-dependency (WAR) */
            int read_before_write = arr1[i];   /* Read arr1[i] */
            arr1[i] = tmp3;                    /* Write arr1[i] - anti-dep on read */
            
            /* Output dependency (WAW) */
            tmp4 = read_before_write;          /* First write to tmp4 */
            if (tmp3 > 100) {
                tmp4 = tmp3 * 2;               /* Second write to tmp4 - WAW */
            }
            
            acc1 += tmp4;
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = arr2[i] - arr1[i];
            tmp2 = tmp1 % 17;                  /* Higher latency operation */
            tmp3 = tmp2 * tmp2;
            
            /* Memory aliasing with pointer arithmetic */
            int* p1 = &arr1[i];
            int* p2 = &arr1[(i + 1) % n];
            *p1 = tmp3;                        /* Store via p1 */
            tmp4 = *p2;                        /* Load via p2 - potential memory dep */
            
            /* Nested loop for additional complexity */
            for (int j = 0; j < 3; ++j) {
                tmp4 += j;
            }
            
            acc2 += tmp4;
        }
        
        /* Loop-carried dependency across iterations */
        static int lc_var = 0;
        tmp5 = lc_var + i;                     /* Depends on previous iteration's lc_var */
        lc_var = tmp5 % 256;                   /* Write for next iteration */
        
        /* Complex conditional with multiple dependencies */
        if ((i % 3) == 0) {
            acc3 += tmp5 * 2;
        } else if ((i % 3) == 1) {
            acc3 += tmp5 / 2;
        } else {
            acc3 += tmp5 + arr1[i % n];
        }
        
        /* Force memory dependency through aliasing */
        arr2[(i + 2) % n] = acc1 + acc2 + acc3;
    }
    
    /* Keep results alive */
    global_acc += acc1 + acc2 + acc3;
    KEEP_ALIVE(acc1);
    KEEP_ALIVE(acc2);
    KEEP_ALIVE(acc3);
}

/* Kernel 2: Arithmetic chains with varying latencies */
NOINLINE static void kernel_arithmetic_chains(int* data, int n) {
    int a, b, c, d, e, f, g, h, j, k;
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long true dependency chain */
        a = data[i] + i;                       /* Node 1 */
        b = a * 2;                             /* Node 2 depends on 1 */
        c = b % 13;                            /* Node 3 depends on 2 (higher latency) */
        d = c - i;                             /* Node 4 depends on 3 */
        e = d / 3;                             /* Node 5 depends on 4 */
        f = e * e;                             /* Node 6 depends on 5 */
        
        /* Parallel chain that merges */
        g = data[(i + 1) % n] * 3;             /* Node 7 */
        h = g + 7;                             /* Node 8 depends on 7 */
        
        /* Output dependencies (WAW) */
        int tmp = a + g;                       /* First write to tmp */
        if (f > h) {
            tmp = f - h;                       /* Second write to tmp - WAW */
        } else {
            tmp = h - f;                       /* Third write to tmp - WAW */
        }
        
        /* Anti-dependencies (WAR) */
        j = tmp;                               /* Read tmp */
        tmp = j * 2;                           /* Write tmp - anti-dep on read */
        k = tmp + j;                           /* Use both values */
        
        /* Loop-carried dependency with distance > 1 */
        static int prev[3] = {0, 0, 0};
        int idx = i % 3;
        int carried = prev[idx] + k;           /* Depends on iteration i-3 */
        prev[idx] = carried % 100;
        
        /* Mix operations with different latencies */
        if (i & 4) {
            sum += carried / 5;                /* Division - higher latency */
        } else {
            sum += carried * 3;                /* Multiplication */
        }
        
        /* Store result creating memory dependency for next iteration */
        data[(i + 5) % n] = sum;
    }
    
    global_acc += sum;
    KEEP_ALIVE(sum);
}

/* Kernel 3: Complex control flow with nested loops */
NOINLINE static void kernel_control_flow(int* arr, int n) {
    int x = 0, y = 0, z = 0;
    int counter = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Switch-like control flow creating multiple basic blocks */
        switch (i % 4) {
            case 0: {
                /* Memory intensive block */
                int* ptr1 = &arr[i];
                int* ptr2 = &arr[(i + 1) % n];
                int val1 = *ptr1;              /* Load */
                *ptr2 = val1 + i;              /* Store - potential memory dep */
                x += val1;
                break;
            }
            case 1: {
                /* Arithmetic chain block */
                int t1 = arr[i] + x;
                int t2 = t1 * t1;
                int t3 = t2 % 17;
                int t4 = t3 - y;
                y = t4;
                break;
            }
            case 2: {
                /* Nested loop with dependencies */
                int inner_sum = 0;
                for (int j = 0; j < 5; ++j) {
                    inner_sum += arr[(i + j) % n] + j;
                }
                z += inner_sum;
                break;
            }
            case 3: {
                /* Mixed dependencies */
                int a = arr[i];
                int b = a + z;                  /* RAW on z from previous iteration */
                arr[(i + 2) % n] = b;           /* Store */
                int c = arr[(i + 3) % n];       /* Load - potential memory dep with store */
                x = c + b;
                break;
            }
        }
        
        /* Loop-carried output dependency */
        static int out_var = 0;
        int old_out = out_var;
        out_var = (old_out + i) & 0xFF;
        
        /* Complex condition with data flow */
        if (counter++ > 10) {
            y = x + z;
            counter = 0;
        }
        
        /* Force register pressure with many live variables */
        int r1 = x, r2 = y, r3 = z, r4 = out_var;
        int r5 = r1 + r2, r6 = r3 * r4, r7 = r5 - r6;
        arr[i % n] = r7;
    }
    
    global_acc += x + y + z;
    KEEP_ALIVE(x);
    KEEP_ALIVE(y);
    KEEP_ALIVE(z);
}

/* Simple PRNG to initialize data without library calls */
static int lcg(int* state) {
    *state = (*state * 1103515245 + 12345) & 0x7FFFFFFF;
    return *state;
}

int main(void) {
    int data1[SIZE];
    int data2[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = lcg(&seed) % 1000;
        data2[i] = lcg(&seed) % 1000;
    }
    
    /* Call kernels with complex dependency patterns */
    kernel_memory_aliasing(data1, data2, ITERS);
    kernel_arithmetic_chains(data1, ITERS);
    kernel_control_flow(data2, ITERS);
    
    /* Use result to prevent dead code elimination */
    sink = global_acc;
    KEEP_ALIVE(global_acc);
    
    return global_acc & 0xFF;
}
