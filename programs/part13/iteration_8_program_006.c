/* ddg_edge_trigger.c
 * Designed to trigger ddg_edge allocation and initialization
 * in GCC's DDG construction during modulo scheduling
 */

#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v) : "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
NOOPT static void kernel_mem_aliasing(int* arr1, int* arr2, int n) {
    int i;
    int tmp1, tmp2, tmp3, tmp4;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Create potential pointer aliasing */
    int* p1 = arr1;
    int* p2 = arr2;
    int* p3 = arr1 + 1;
    
    for (i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency chain (RAW) */
            tmp1 = *p1 + i;          /* Read from memory */
            tmp2 = tmp1 * 2;         /* Use result */
            tmp3 = tmp2 - *p2;       /* Another memory read */
            
            /* Anti-dependency (WAR) */
            int read_before_write = *p3;
            *p3 = tmp3;              /* Write to potentially aliased location */
            acc1 += read_before_write;
            
            /* Output dependency (WAW) */
            tmp4 = acc1;
            if (tmp3 > 100) {
                tmp4 = tmp2;         /* Reassign tmp4 */
            }
            acc2 += tmp4;
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = *p2 - i;
            tmp2 = tmp1 / 3;         /* Higher latency division */
            tmp3 = tmp2 % 7;         /* Another high latency op */
            
            /* Memory anti-dependency */
            int old_val = *p1;
            *p1 = tmp3;
            acc3 += old_val;
            
            /* Complex output dependency chain */
            tmp4 = acc3;
            for (int j = 0; j < 3; ++j) {
                tmp4 += j;           /* Nested loop creates more edges */
            }
            acc4 = tmp4;             /* WAW on acc4 in next iteration */
        }
        
        /* Pointer arithmetic that may alias */
        p1 = &arr1[(i * 17) % SIZE];
        p2 = &arr2[(i * 13) % SIZE];
        p3 = &arr1[(i * 19) % SIZE];
        
        /* Mix results to keep all variables live */
        int mixed = (acc1 ^ acc2) | (acc3 & acc4);
        KEEP_ALIVE(mixed);
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Kernel 2: Arithmetic dependency chains with control flow */
NOOPT static void kernel_arithmetic_chains(int* data, int n) {
    int i;
    int a, b, c, d, e, f, g, h;
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    
    for (i = 0; i < n; ++i) {
        /* Long true dependency chain */
        a = data[i] + 1;
        b = a * data[(i + 1) % SIZE];
        c = b - (i % 256);
        d = c / ((data[i] & 31) + 1);  /* Variable divisor for latency */
        e = d % 17;
        f = e ^ 0x55AA55AA;
        
        /* Branch creates control dependencies */
        if (f > 0) {
            g = f + r1;
            r1 = g;                     /* Loop-carried dependency */
        } else {
            g = f - r2;
            r2 = g;                     /* Another loop-carried dep */
        }
        
        /* Nested loop for additional basic blocks */
        int sum = 0;
        for (int j = 0; j < 2; ++j) {
            sum += data[(i + j) % SIZE] * j;
        }
        
        /* Output dependencies (WAW) */
        h = sum;
        if (i & 2) {
            h = g * 2;                  /* Reassign h */
        } else {
            h = sum / 2;                /* Another reassignment */
        }
        
        /* Anti-dependencies (WAR) */
        int temp = r3;
        r3 = h + temp;
        r4 = r3 - temp;                 /* Read r3 after write */
        
        /* Complex expression to prevent elimination */
        r5 = (r1 * r2) + (r3 / ((r4 & 255) + 1)) - (r5 ^ i);
        
        KEEP_ALIVE(r5);
    }
    
    global_acc += r1 + r2 + r3 + r4 + r5;
}

/* Kernel 3: Mixed dependencies with unpredictable control flow */
NOOPT static void kernel_mixed(int* arr, int n) {
    int i;
    int x1 = 0, x2 = 0, x3 = 0, x4 = 0;
    int y1 = 1, y2 = 2, y3 = 3, y4 = 4;
    
    for (i = 0; i < n; ++i) {
        /* Switch-like structure for multiple basic blocks */
        switch (i & 3) {
            case 0:
                /* Memory and register dependencies */
                x1 = arr[i] + y1;
                y1 = x1 * 2;            /* WAR: y1 read then written */
                arr[(i + 1) % SIZE] = y1; /* Memory write */
                break;
            case 1:
                /* Different dependency pattern */
                x2 = arr[i] - y2;
                y2 = x2 / ((i & 7) + 1);
                x3 = y2 + x1;           /* Cross-iteration dependency */
                break;
            case 2:
                /* Output dependencies */
                x4 = y3;
                x4 = y4 * 3;            /* WAW on x4 */
                y3 = x4 + arr[i];
                y4 = y3 - 1;
                break;
            case 3:
                /* Complex chain */
                int t1 = arr[i];
                int t2 = t1 ^ y1;
                int t3 = t2 | y2;
                int t4 = t3 & y3;
                y1 = t4 + y4;           /* WAR on y1 from case 0 */
                break;
        }
        
        /* Loop-carried dependencies with distance > 0 */
        static int carry = 0;
        int old_carry = carry;
        carry = (x1 + x2 + x3 + x4) & 0xFF;
        
        /* Memory anti-dependency across iterations */
        int* ptr = &arr[i % SIZE];
        int read_val = *ptr;
        *ptr = carry + old_carry;
        
        /* Force memory dependency edge */
        x1 = read_val + x1;
        
        /* Volatile asm to prevent reordering */
        asm volatile("" : "+r"(x1), "+r"(x2), "+r"(x3), "+r"(x4)
                     : : "memory");
    }
    
    global_acc += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Simple PRNG to initialize data without library calls */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main(void) {
    int data1[SIZE], data2[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = simple_rand(&seed) % 1000;
        data2[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call kernels with complex dependency patterns */
    kernel_mem_aliasing(data1, data2, ITERS);
    kernel_arithmetic_chains(data1, ITERS);
    kernel_mixed(data2, ITERS);
    
    /* Use result to prevent dead code elimination */
    sink = global_acc;
    
    return sink & 0xFF;
}
