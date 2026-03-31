/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink; /* Prevent dead code elimination */

/* Global accumulator to keep variables live */
static int global_acc = 0;

/* Kernel 1: Memory-heavy with pointer aliasing and WAR/WAW dependencies */
__attribute__((noinline, noipa))
static void kernel_memory_aliasing(int* arr1, int* arr2, int n) {
    int i;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    for (i = 0; i < n; ++i) {
        /* Create multiple basic blocks with if/else */
        if (i & 1) {
            /* True dependency chain (RAW) */
            tmp1 = arr1[i] * 3;
            tmp2 = tmp1 + arr2[i];      /* RAW on tmp1 */
            tmp3 = tmp2 / 2;            /* RAW on tmp2 */
            
            /* Anti-dependency (WAR) */
            int read_before = arr1[i];  /* Read arr1[i] */
            arr1[i] = tmp3;             /* Write arr1[i] - WAR with read_before */
            
            /* Output dependency (WAW) */
            tmp4 = read_before + i;
            tmp4 = tmp3 * 2;            /* WAW on tmp4 */
            
            acc1 += tmp4;
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = arr2[i] - arr1[i];
            tmp2 = tmp1 * tmp1;         /* RAW on tmp1 */
            
            /* Memory aliasing with potential WAR */
            int* p1 = &arr1[i];
            int* p2 = &arr1[(i + 1) % n];
            int* p3 = &arr2[i];
            
            int val1 = *p1;             /* Read through p1 */
            *p3 = val1 + tmp2;          /* Write through p3 - no WAR with p1 */
            
            /* Complex RAW chain with higher latency operations */
            tmp3 = val1 % 7;            /* Higher latency modulo */
            tmp4 = tmp3 / 3;            /* Higher latency division - RAW on tmp3 */
            tmp5 = tmp4 + *p2;          /* RAW on tmp4 + memory read */
            
            acc2 += tmp5;
        }
        
        /* Loop-carried dependency with distance 1 */
        if (i > 0) {
            /* Cross-iteration RAW: uses value from previous iteration */
            acc3 = acc3 + arr1[i-1];    /* Loop-carried on acc3 */
        }
        
        /* Another loop-carried dependency with distance 2 */
        if (i > 1) {
            acc4 = acc4 + arr2[i-2];    /* Loop-carried on acc4 */
        }
        
        /* Force memory barrier to prevent reordering */
        __asm__ volatile("" : : "r"(arr1[i]), "r"(arr2[i]));
    }
    
    /* Feed results to global accumulator and volatile sink */
    global_acc += acc1 + acc2 + acc3 + acc4;
    __asm__ volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3), "r"(acc4));
}

/* Kernel 2: Arithmetic-heavy with nested loops and complex control flow */
__attribute__((noinline, noipa))
static void kernel_arithmetic_chains(int* arr, int n) {
    int i, j;
    int a, b, c, d, e, f, g, h;
    int sum = 0;
    
    for (i = 0; i < n; ++i) {
        /* Multiple parallel dependency chains */
        a = arr[i] + i;
        b = a * 2;                      /* RAW on a */
        c = b - i;                      /* RAW on b */
        
        /* Branch creating control dependency */
        if (arr[i] > 0) {
            d = c % 5;                  /* RAW on c, higher latency */
            e = d * d;                  /* RAW on d */
            f = e + arr[i];             /* RAW on e */
        } else {
            d = c / 3;                  /* RAW on c, higher latency */
            e = d - 100;                /* RAW on d */
            f = e * 2;                  /* RAW on e */
        }
        
        /* Nested loop to create more basic blocks */
        for (j = 0; j < 3; ++j) {
            /* WAW dependency in nested loop */
            g = f + j;                  /* RAW on f */
            g = g * (j + 1);            /* WAW on g */
            
            /* Anti-dependency in nested scope */
            int temp = arr[j % n];      /* Read */
            arr[j % n] = g;             /* Write - WAR with temp */
            
            h = temp + g;               /* RAW on both */
            sum += h;
        }
        
        /* Loop-carried output dependency */
        static int persistent = 0;
        persistent = sum;               /* WAW loop-carried on persistent */
        
        /* Complex expression with multiple uses */
        arr[i] = (a + b + c + d + e + f) % 256;
        
        /* Volatile to prevent elimination */
        __asm__ volatile("" : : "r"(a), "r"(b), "r"(c), "r"(sum));
    }
    
    global_acc += sum;
    sink = persistent;  /* Use persistent to keep it live */
}

/* Kernel 3: Mixed dependencies with unpredictable control flow */
__attribute__((noinline, noipa))
static void kernel_mixed_control_flow(int* data, int n) {
    int i;
    int x = 0, y = 0, z = 0;
    int r1, r2, r3, r4, r5;
    
    for (i = 0; i < n; ++i) {
        /* Switch-like control flow */
        switch (i % 4) {
            case 0:
                r1 = data[i] + x;
                r2 = r1 * 3;            /* RAW on r1 */
                x = r2;                 /* Loop-carried WAW on x */
                break;
            case 1:
                r1 = data[i] - y;
                r2 = r1 / 2;            /* RAW on r1, higher latency */
                y = r2;                 /* Loop-carried WAW on y */
                break;
            case 2:
                r1 = data[i] * z;
                r2 = r1 % 7;            /* RAW on r1, higher latency */
                z = r2;                 /* Loop-carried WAW on z */
                break;
            case 3:
                r1 = x + y + z;
                r2 = r1 & 0xFF;         /* RAW on r1 */
                data[i] = r2;           /* Memory write */
                break;
        }
        
        /* Interleaved dependency chains across iterations */
        if (i > 2) {
            r3 = data[i-1] + data[i-2] + data[i-3];
            r4 = r3 * data[i];          /* RAW on r3 */
            r5 = r4 / (i + 1);          /* RAW on r4, higher latency */
            
            /* Multiple uses create pressure */
            x = x + r5;
            y = y - r5;
            z = z ^ r5;
        }
        
        /* Memory aliasing with computed indices */
        int idx1 = i % n;
        int idx2 = (i * 7 + 3) % n;
        int idx3 = (i * 13 + 5) % n;
        
        int read_val = data[idx1];      /* Read */
        data[idx2] = read_val + i;      /* Write to potentially aliased location */
        data[idx3] = data[idx2] * 2;    /* RAW through memory */
        
        /* Force dependency barrier */
        __asm__ volatile("" : : "r"(x), "r"(y), "r"(z), "r"(read_val));
    }
    
    global_acc += x + y + z;
    __asm__ volatile("" : : "r"(x), "r"(y), "r"(z));
}

/* Simple PRNG to initialize data without library calls */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    int data1[SIZE];
    int data2[SIZE];
    int i;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < SIZE; ++i) {
        data1[i] = (int)(lcg_rand() % 1000);
        data2[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Call kernels with complex dependency patterns */
    kernel_memory_aliasing(data1, data2, ITERATIONS);
    kernel_arithmetic_chains(data1, ITERATIONS);
    kernel_mixed_control_flow(data2, ITERATIONS);
    
    /* Use results to prevent dead code elimination */
    sink = global_acc;
    
    return sink & 0xFF;  /* Return non-zero to indicate execution */
}
