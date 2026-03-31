/* ddg_edge_trigger.c
 * Complex loop structure to trigger DDG edge creation in GCC
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

/* Kernel 1: Mixed memory and register dependencies with complex control flow */
NOOPT static int kernel1(int *arr1, int *arr2, int n) {
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    int t1, t2, t3, t4, t5, t6, t7, t8;
    
    for (int i = 0; i < n; ++i) {
        /* Basic block 1: True dependencies (RAW) */
        a = arr1[i] + arr2[i];      /* RAW source for next */
        b = a * 2;                  /* RAW depends on a */
        c = b % 17;                 /* RAW depends on b, higher latency */
        
        /* Basic block 2: Anti-dependencies (WAR) with branching */
        if (i & 1) {
            /* Read before write - WAR dependency */
            t1 = arr1[i];           /* Read arr1[i] */
            arr1[i] = b + c;        /* Write arr1[i] - WAR with t1 */
            d = t1 * 3;             /* Use t1 */
        } else {
            /* Alternative path with output dependencies (WAW) */
            t2 = arr1[i] + 5;       /* First assignment to t2 */
            t2 = arr2[i] * 7;       /* Second assignment to t2 - WAW */
            d = t2 / 2;             /* Use t2 */
        }
        
        /* Basic block 3: Memory aliasing dependencies */
        int *p1 = &arr1[i];
        int *p2 = &arr2[(i * 17) % n];  /* May alias with p1 */
        
        *p1 = d + i;                /* Memory write */
        e = *p2 + *p1;              /* Memory read - potential MEM_DEP */
        
        /* Basic block 4: Nested loop for additional complexity */
        for (int j = 0; j < 3; ++j) {
            /* Output dependency chain */
            static int counter = 0;  /* Static to force memory */
            counter = counter + j;   /* WAW on counter */
            f = f + counter;         /* RAW on f (loop-carried) */
        }
        
        /* Basic block 5: Complex arithmetic chain */
        t3 = (e << 3) | (f & 0xFF);
        t4 = t3 % (i + 1);          /* Variable divisor for latency */
        t5 = t4 * t4 - t3;
        t6 = (t5 >> 2) + (t4 << 1);
        
        /* Control dependency */
        if (t6 > 1000) {
            t7 = t6 / 3;
            arr2[i] = t7;           /* Memory write */
        } else {
            t8 = t6 * 5;
            arr2[i] = t8;           /* Memory write - different basic block */
        }
        
        /* Loop-carried output dependency */
        global_acc = global_acc + arr1[i] - arr2[i];  /* WAW on global_acc */
        
        /* Force all values to be live */
        KEEP(a); KEEP(b); KEEP(c); KEEP(d);
        KEEP(e); KEEP(f); KEEP(t3); KEEP(t4);
        KEEP(t5); KEEP(t6);
    }
    
    MEM_BARRIER();
    return a + b + c + d + e + f;
}

/* Kernel 2: Heavy arithmetic dependencies with pointer chasing */
NOOPT static int kernel2(int *base, int offset, int n) {
    int sum = 0;
    int *ptr = base;
    
    for (int i = 0; i < n; ++i) {
        int idx = (i * 31) % (n - 1);
        int *p1 = base + idx;
        int *p2 = base + ((idx + offset) % (n - 1));
        
        /* True dependency chain with varying latencies */
        int val1 = *p1;
        int val2 = val1 * val1;     /* RAW on val1 */
        int val3 = val2 / (val1 + 1); /* RAW on val1, val2, higher latency */
        int val4 = val3 % 19;       /* RAW on val3 */
        
        /* Anti-dependency with memory */
        int temp = *p2;             /* Read *p2 */
        *p2 = val4 + i;             /* Write *p2 - WAR with temp */
        
        /* Output dependency chain */
        int accum = val2;
        accum = accum + val3;       /* WAW on accum */
        accum = accum * 2;          /* WAW on accum */
        
        /* Complex condition for control dependencies */
        if ((val1 ^ val2) & 0xF) {
            if (val3 > val4) {
                *p1 = accum + temp; /* Memory write */
            } else {
                *p1 = accum - temp; /* Different basic block */
            }
        } else {
            *p1 = accum;            /* Another basic block */
        }
        
        /* Loop-carried true dependency */
        sum = sum + *p1 + *p2;      /* RAW on sum (loop-carried) */
        
        /* Pointer chasing with potential aliasing */
        ptr = base + ((ptr - base + 1) % (n - 1));
        int chase_val = *ptr;
        sum = sum ^ chase_val;      /* Additional dependency */
        
        KEEP(val1); KEEP(val2); KEEP(val3); KEEP(val4);
        KEEP(temp); KEEP(accum); KEEP(chase_val);
    }
    
    MEM_BARRIER();
    return sum;
}

/* Kernel 3: Complex control flow with nested loops */
NOOPT static int kernel3(int *data, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; ++i) {
        /* Multiple nested conditionals */
        if (data[i] > 0) {
            if (data[i] & 1) {
                /* True dependency chain */
                int x = data[i - 1] + data[i];
                int y = x * x;              /* RAW on x */
                int z = y % (x + 1);        /* RAW on x, y */
                
                /* Anti-dependency */
                int old = data[i + 1];      /* Read */
                data[i + 1] = z + i;        /* Write - WAR with old */
                result += old;              /* Use old value */
                
                /* Small inner loop with carried dependency */
                for (int k = 0; k < 2; k++) {
                    static int inner = 0;
                    inner = inner + z + k;  /* WAW on inner */
                    result = result ^ inner; /* RAW on result (carried) */
                }
            } else {
                /* Different path with output dependencies */
                int tmp = data[i] * 3;
                tmp = tmp + data[i - 1];    /* WAW on tmp */
                tmp = tmp >> 1;             /* WAW on tmp */
                
                /* Memory dependency with potential aliasing */
                int *alias1 = &data[i];
                int *alias2 = &data[(i * 7) % n];
                
                *alias1 = tmp;
                result += *alias2;          /* MEM_DEP possible */
            }
        } else {
            /* Third path with arithmetic chain */
            int chain1 = data[i] - data[i + 1];
            int chain2 = chain1 * chain1;   /* RAW on chain1 */
            int chain3 = chain2 | 0xFFFF;   /* RAW on chain2 */
            int chain4 = chain3 & chain1;   /* RAW on chain1, chain3 */
            
            data[i] = chain4;
            result -= chain4;
        }
        
        /* Complex condition with side effects */
        switch (i % 4) {
            case 0:
                data[i] = result + 1;
                break;
            case 1:
                data[i] = result - 1;
                break;
            case 2:
                data[i] = result * 2;
                break;
            case 3:
                data[i] = result / 2;       /* Higher latency */
                break;
        }
        
        KEEP(result);
    }
    
    MEM_BARRIER();
    return result;
}

/* Simple PRNG to initialize data without library calls */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main(void) {
    int data1[SIZE];
    int data2[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = simple_rand(&seed) % 1000;
        data2[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call kernels with complex dependency patterns */
    int r1 = kernel1(data1, data2, ITERS);
    int r2 = kernel2(data1, SIZE/2, ITERS);
    int r3 = kernel3(data2, ITERS);
    
    /* Combine results and force output */
    int total = r1 + r2 + r3 + global_acc;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(total));
    
    /* Use volatile sink */
    sink = total;
    
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
