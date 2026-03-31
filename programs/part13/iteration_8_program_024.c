/* ddg_test.c - Complex loop-carried dependency generator for DDG edge coverage */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" ::: "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
NOOPT static int kernel_mem_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    int *p1, *p2, *p3;
    
    /* Multiple pointers that may alias */
    p1 = arr1;
    p2 = arr1 + (n / 2);
    p3 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Basic block 1: RAW dependencies */
        tmp1 = *p1 + i;          /* Read from p1 */
        tmp2 = tmp1 * 2;         /* RAW: depends on tmp1 */
        tmp3 = tmp2 / 3;         /* RAW: depends on tmp2 */
        
        /* Basic block 2: WAR dependencies with branching */
        if (i & 1) {
            /* WAR: Read before write to same location */
            tmp4 = *p2;          /* Read p2 */
            *p2 = tmp3 + i;      /* Write p2 - anti-dep on previous read */
            tmp5 = tmp4 * 7;     /* RAW: depends on tmp4 */
        } else {
            /* Different computation path */
            tmp4 = *p3 + i;      /* Read p3 */
            *p3 = tmp3 - i;      /* Write p3 */
            tmp5 = tmp4 / 5;     /* RAW: depends on tmp4 */
        }
        
        /* Basic block 3: WAW dependencies and memory aliasing */
        /* Multiple writes to same variable (output dep) */
        int accum = tmp2 + tmp5;
        if (i % 3 == 0) {
            accum = tmp1 * tmp4;  /* WAW: overwrites accum */
        }
        
        /* Potential memory dependency through aliasing */
        *p1 = accum + *p3;       /* Write p1 - may alias with p2/p3 reads */
        
        /* Complex pointer arithmetic that may cause aliasing */
        p1 = arr1 + ((i * 17) % (n - 1));
        p2 = arr1 + ((i * 13) % (n - 1));
        p3 = arr2 + ((i * 11) % (n - 1));
        
        /* Keep values live */
        sum += accum + tmp3;
        
        /* Memory barrier to prevent reordering */
        MEM_BARRIER();
    }
    
    KEEP(sum);
    global_acc += sum;
    return sum;
}

/* Kernel 2: Arithmetic chains with varying latencies and control flow */
NOOPT static int kernel_arithmetic_chains(int *arr, int n) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int x = 0, y = 0, z = 0;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long arithmetic chain with mixed operations (different latencies) */
        a = arr[i] + b;          /* Fast operation */
        b = a * c;               /* Medium latency */
        
        /* Division/modulo - higher latency operations */
        if (arr[i] % 7 == 0) {
            c = b / (a + 1);     /* High latency division */
            d = c % 13;          /* High latency modulo */
        } else {
            c = b * 3;           /* Medium latency */
            d = c - a;           /* Fast operation */
        }
        
        /* Nested loop inside main loop for additional basic blocks */
        int inner_sum = 0;
        for (int j = 0; j < 3; ++j) {
            /* More dependencies inside nested loop */
            e = d + j + arr[(i + j) % n];
            inner_sum += e;
            
            /* WAW in nested scope */
            int local = e * 2;
            if (j & 1) {
                local = e / 2;    /* WAW on local */
            }
            inner_sum += local;
        }
        
        /* Complex conditional with output dependencies */
        x = inner_sum + a;
        if (i % 5 == 0) {
            y = x * b;           /* RAW on x, b */
            x = y - c;           /* WAW on x, RAW on y, c */
        } else if (i % 5 == 1) {
            y = x / (b + 1);     /* Different latency path */
            x = y + d;           /* WAW on x */
        } else {
            y = x % 17;          /* High latency modulo */
            x = y * e;           /* WAW on x */
        }
        
        z = x + y + z;           /* Loop-carried dependency on z */
        
        /* Mix of operations to vary data types */
        result += z + (arr[i] & 0xFF);
        
        /* Force register pressure with many live variables */
        KEEP(a); KEEP(b); KEEP(c); KEEP(d); KEEP(e);
        KEEP(x); KEEP(y); KEEP(z);
    }
    
    KEEP(result);
    global_acc += result;
    return result;
}

/* Kernel 3: Complex control flow with mixed dependency types */
NOOPT static int kernel_control_flow(int *arr1, int *arr2, int n) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int t1, t2, t3, t4, t5, t6;
    
    for (int i = 1; i < n - 1; ++i) {
        /* Multiple basic blocks with data flow across them */
        
        /* Block A: Memory operations with potential aliasing */
        t1 = arr1[i - 1];
        t2 = arr1[i];
        t3 = arr1[i + 1];
        
        /* Block B: Arithmetic with RAW */
        t4 = t1 + t2 + t3;
        t5 = t4 * 2;
        
        /* Block C: Conditional with WAR */
        if (t4 > 100) {
            int old_t6 = t6;      /* Read t6 before write - WAR if t6 used earlier */
            arr2[i] = t5;         /* Memory write */
            t6 = old_t6 + arr2[i]; /* RAW on arr2[i], WAR on old_t6 */
        } else {
            arr2[i] = t4 / 2;     /* Different memory write */
            t6 = t5 - t4;         /* RAW on t5, t4 */
        }
        
        /* Block D: Loop-carried output dependencies */
        static int persistent = 0;
        int prev_persistent = persistent;
        persistent = t6 + i;      /* WAW on persistent (loop-carried) */
        
        /* Block E: Complex expression with all variables */
        acc1 = acc1 + t1;         /* Loop-carried RAW */
        acc2 = acc2 ^ t2;         /* Loop-carried RAW, different op */
        acc3 = acc3 | t3;         /* Loop-carried RAW, different op */
        
        /* Cross-iteration memory dependency */
        arr1[i] = prev_persistent + acc1 + acc2 + acc3;
        
        /* Volatile to prevent elimination */
        asm volatile("" : : "r"(t4), "r"(t5), "r"(t6));
    }
    
    int final = acc1 + acc2 + acc3;
    KEEP(final);
    global_acc += final;
    return final;
}

/* Simple PRNG to avoid library dependencies */
static uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(void) {
    int data1[SIZE];
    int data2[SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = (int)(lcg(&seed) % 1000);
        data2[i] = (int)(lcg(&seed) % 1000);
    }
    
    /* Call kernels with complex dependency patterns */
    int r1 = kernel_mem_aliasing(data1, data2, ITERATIONS);
    int r2 = kernel_arithmetic_chains(data1, ITERATIONS);
    int r3 = kernel_control_flow(data1, data2, ITERATIONS);
    
    /* Combine results and use volatile to prevent dead code elimination */
    int total = r1 + r2 + r3 + global_acc;
    sink = total;
    
    /* Return non-zero to indicate execution */
    return total != 0;
}
