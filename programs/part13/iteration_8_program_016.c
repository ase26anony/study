/* ddg_test.c - Complex loop-carried dependency generator for DDG edge coverage */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

/* Global accumulator to prevent optimization */
static volatile int global_sink = 0;

/* Prevent function inlining and IPA */
static __attribute__((noinline, noipa))
void kernel1_memory_heavy(int *restrict data, int *restrict data2, int n) {
    int i;
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Complex loop with memory dependencies and multiple basic blocks */
    for (i = 0; i < n; ++i) {
        /* Basic block 1: Memory RAW and WAR dependencies */
        int idx = i & (SIZE - 1);
        int *p1 = &data[idx];
        int *p2 = &data2[idx];
        
        /* RAW: Read before write chain */
        tmp1 = *p1;              /* Read */
        *p1 = tmp1 + i;          /* Write - creates WAR with next iteration */
        
        /* Anti-dependency (WAR) with potential aliasing */
        tmp2 = *p2;
        *p2 = tmp2 * 2;          /* WAR: p2 written after read */
        
        /* Basic block 2: Conditional with output dependencies */
        if (i & 1) {
            /* WAW: Multiple writes to same variable */
            tmp3 = acc1 + tmp1;
            tmp3 = tmp3 * 3;     /* WAW on tmp3 */
            acc1 = tmp3;
            
            /* Complex arithmetic chain with varying latency ops */
            tmp4 = (tmp2 % 17) + 1;  /* High latency modulo */
            acc2 = acc2 / (tmp4 + 1); /* Medium latency division */
        } else {
            /* Different WAW pattern */
            tmp3 = acc3 - tmp2;
            tmp3 = tmp3 >> 2;    /* WAW on tmp3 */
            acc3 = tmp3;
            
            /* Another arithmetic chain */
            tmp4 = tmp1 * tmp1;
            acc4 = acc4 + (tmp4 % 13); /* Mix of operations */
        }
        
        /* Basic block 3: Nested loop for additional complexity */
        int j;
        int local_acc = 0;
        for (j = 0; j < 3; ++j) {
            /* Loop-carried dependency within inner loop */
            local_acc = local_acc + (tmp1 >> j);
            /* Memory dependency with outer loop array */
            data[(idx + j) & (SIZE - 1)] += local_acc;
        }
        
        /* Cross-iteration dependency (loop-carried) */
        sum = sum + acc1 + acc2 + acc3 + acc4;
        
        /* Force dependency chain to continue */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    /* Prevent dead code elimination */
    global_sink += sum;
}

static __attribute__((noinline, noipa))
void kernel2_arithmetic_chains(int *data, int n) {
    int i;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int x = 0, y = 0, z = 0;
    
    /* Loop with long arithmetic dependency chains */
    for (i = 0; i < n; ++i) {
        /* Long RAW chain with mixed operations */
        a = b + data[i & (SIZE - 1)];    /* RAW: uses b */
        b = c * a;                       /* RAW: uses a, c */
        c = d - b;                       /* RAW: uses b, d */
        d = e / (c + 1);                 /* RAW: uses c, e (potential div by zero prevention) */
        e = a % (d + 2);                 /* RAW: uses a, d */
        
        /* Parallel chain with control dependencies */
        if ((i ^ a) & 1) {
            x = y + z;
            y = x * 2;
            z = y - x;
        } else {
            x = z - y;
            y = x / 2;
            z = y + x;
        }
        
        /* Output dependencies (WAW) */
        int tmp = a + b;
        tmp = tmp + c;      /* WAW on tmp */
        tmp = tmp * d;      /* WAW on tmp */
        
        /* Anti-dependencies (WAR) through array */
        int idx = (i * 7) & (SIZE - 1);
        int read_val = data[idx];      /* Read */
        data[idx] = tmp + read_val;    /* Write - WAR */
        
        /* Loop-carried dependency with distance > 0 */
        static int carry = 0;
        int old_carry = carry;
        carry = tmp + old_carry;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(carry), "r"(tmp) : "memory");
    }
    
    global_sink += a + b + c + d + e + x + y + z;
}

static __attribute__((noinline, noipa))
void kernel3_complex_control_flow(int *data, int *data2, int n) {
    int i;
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    
    for (i = 0; i < n; ++i) {
        /* Multiple nested conditionals creating complex CFG */
        int cond1 = i & 3;
        int cond2 = data[i & (SIZE - 1)] & 1;
        
        if (cond1 == 0) {
            /* Basic block A */
            r1 = r5 + data2[i & (SIZE - 1)];  /* Loop-carried from r5 */
            if (cond2) {
                r2 = r1 * 2;
                data[i & (SIZE - 1)] = r2;    /* Memory write */
            } else {
                r2 = r1 / 2;
                data2[i & (SIZE - 1)] = r2;   /* Memory write to different array */
            }
        } else if (cond1 == 1) {
            /* Basic block B */
            r3 = r2 - r1;
            /* Small unrolled loop inside basic block */
            int t = r3;
            t = t + (t >> 1);
            t = t - (t & 1);
            t = t * 3;
            r3 = t;
        } else if (cond1 == 2) {
            /* Basic block C */
            r4 = r3 ^ r2;
            /* Pointer aliasing challenge */
            int *ptr1 = &data[i & (SIZE - 1)];
            int *ptr2 = &data2[i & (SIZE - 1)];
            *ptr1 = r4;
            r5 = *ptr2;  /* May alias with ptr1? Compiler doesn't know */
        } else {
            /* Basic block D */
            r5 = r4 | r3;
            /* Complex expression with multiple dependencies */
            r5 = (r5 * 13 + 17) % 23;
        }
        
        /* Cross-basic-block dependency */
        int sink = r1 + r2 + r3 + r4 + r5;
        asm volatile("" : "+r"(sink) : : "memory");
    }
    
    global_sink += r1 + r2 + r3 + r4 + r5;
}

/* Simple PRNG to avoid library dependencies */
static uint32_t lcg = 123456789;
static uint32_t rand_lcg(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main(void) {
    /* Initialize data with pseudo-random values */
    int data[SIZE];
    int data2[SIZE];
    
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (int)(rand_lcg() & 0xFFF);
        data2[i] = (int)(rand_lcg() & 0xFFF);
    }
    
    /* Call kernels with complex dependency patterns */
    kernel1_memory_heavy(data, data2, ITERATIONS);
    kernel2_arithmetic_chains(data, ITERATIONS);
    kernel3_complex_control_flow(data, data2, ITERATIONS);
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(global_sink) : "memory");
    
    return global_sink & 0xFF;
}
