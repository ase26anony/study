/* ddg_edge_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-ddg -fdump-rtl-sched1 ddg_edge_coverage.c -o ddg_test
 */

#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noipa))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Simple LCG for pseudo-random values */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Kernel 1: Heavy memory dependencies with aliasing */
NOINLINE static void kernel_mem_deps(int* data, int n, int stride) {
    int* p1 = data;
    int* p2 = data + stride;
    int* p3 = data + 2 * stride;
    
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < n; ++i) {
        /* True dependencies (RAW) with memory */
        tmp1 = *p1;           /* Read from p1 */
        a = tmp1 + i;         /* Use tmp1 */
        *p1 = a;              /* Write to p1 (WAW with previous iteration) */
        
        /* Anti-dependency (WAR) */
        tmp2 = *p2;           /* Read from p2 */
        *p2 = b + tmp2;       /* Write to p2 after read */
        b = tmp2 * 2;         /* Use tmp2 */
        
        /* Output dependency (WAW) with control flow */
        if (i & 1) {
            tmp3 = *p3;       /* Read */
            c = tmp3 % 7;     /* High latency op */
            *p3 = c;          /* Write */
        } else {
            tmp3 = *p1;       /* Different source */
            c = tmp3 / 3;     /* Another high latency op */
            *p3 = c;          /* Write (WAW in else branch) */
        }
        
        /* Complex chain with multiple basic blocks */
        d = a + b;
        e = c * d;
        
        /* Nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            f = e + j;
            /* Volatile asm to prevent elimination */
            __asm__ volatile("" : "+r" (f));
        }
        
        /* Pointer arithmetic that may alias */
        p1 += (i % 2) ? 1 : -1;
        p2 += (i % 3) ? 1 : 0;
        p3 = data + (i % 5);
        
        /* Keep values live */
        global_acc += a + b + c + d + e + f;
    }
    
    /* Sink result */
    sink = global_acc;
}

/* Kernel 2: Arithmetic chains with complex control flow */
NOINLINE static void kernel_arith_deps(int* data, int n) {
    int r0 = 0, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long arithmetic chain with true dependencies */
        r0 = data[i] + 1;            /* RAW: source for r1 */
        r1 = r0 * 2;                 /* RAW: depends on r0 */
        r2 = r1 - i;                 /* RAW: depends on r1 */
        r3 = r2 / 3;                 /* RAW: high latency */
        r4 = r3 % 5;                 /* RAW: high latency */
        
        /* Parallel chain with anti-dependencies */
        r5 = r4 + r0;                /* WAR: r0 used after earlier chain */
        r0 = r5 * 7;                 /* WAR: overwrite r0 */
        
        /* Output dependencies with control flow */
        if (i % 4 == 0) {
            r6 = r1 + r2;
            r7 = r6 << 2;
        } else if (i % 4 == 1) {
            r6 = r3 - r4;            /* WAW: r6 reassigned */
            r7 = r6 >> 1;
        } else if (i % 4 == 2) {
            r6 = r5 | r0;            /* WAW: r6 reassigned again */
            r7 = r6 & 0xFF;
        } else {
            r6 = r2 ^ r3;            /* WAW: r6 reassigned */
            r7 = ~r6;
        }
        
        /* More complex branching */
        switch (i % 3) {
            case 0:
                r8 = r7 * 11;
                r9 = r8 + data[i % SIZE];
                break;
            case 1:
                r8 = r7 / 13;        /* WAW: r8 reassigned */
                r9 = r8 - data[(i + 1) % SIZE];
                break;
            case 2:
                r8 = r7 % 17;        /* WAW: r8 reassigned */
                r9 = r8 ^ data[(i + 2) % SIZE];
                break;
        }
        
        /* Loop-carried dependency with distance > 0 */
        r10 = r9 + r10;              /* r10 depends on previous iteration's r10 */
        
        /* Volatile asm to prevent optimization */
        __asm__ volatile("" : : "r" (r0), "r" (r1), "r" (r2), "r" (r3),
                         "r" (r4), "r" (r5), "r" (r6), "r" (r7),
                         "r" (r8), "r" (r9), "r" (r10));
        
        global_acc += r10;
    }
    
    sink = global_acc;
}

/* Kernel 3: Mixed dependencies with pointer chasing */
NOINLINE static void kernel_mixed_deps(int* data, int n) {
    int* ptrs[4];
    int vals[8];
    
    /* Initialize pointers with potential aliasing */
    ptrs[0] = data;
    ptrs[1] = data + 1;
    ptrs[2] = data + 2;
    ptrs[3] = data + 3;
    
    for (int i = 0; i < n; ++i) {
        /* Memory dependencies with aliasing */
        int* p = ptrs[i % 4];
        int* q = ptrs[(i + 1) % 4];
        
        /* True memory dependency */
        vals[0] = *p;                /* Read from p */
        vals[1] = vals[0] + *q;      /* RAW: uses vals[0] */
        *p = vals[1];                /* Write to p (WAW) */
        
        /* Anti-dependency with memory */
        vals[2] = *q;                /* Read from q */
        *q = vals[2] * 2;            /* Write to q after read (WAR) */
        
        /* Complex arithmetic with loop-carried deps */
        vals[3] = vals[1] + vals[2];
        vals[4] = vals[3] * vals[0];
        
        /* Control-dependent computations */
        if (vals[4] > 0) {
            vals[5] = vals[4] / 3;
            vals[6] = vals[5] % 7;
        } else {
            vals[5] = vals[4] * 3;
            vals[6] = vals[5] + 11;
        }
        
        /* Output dependency chain */
        vals[7] = vals[6];           /* First assignment */
        for (int j = 0; j < 2; ++j) {
            vals[7] = vals[7] + j;   /* WAW: reassign vals[7] */
        }
        
        /* Pointer chasing that creates memory deps */
        ptrs[i % 4] = data + (vals[7] % (SIZE - 10));
        
        /* Force all values to be live */
        __asm__ volatile("" : : "m" (*ptrs[0]), "m" (*ptrs[1]),
                         "m" (*ptrs[2]), "m" (*ptrs[3]));
        
        global_acc += vals[0] + vals[1] + vals[2] + vals[3] +
                     vals[4] + vals[5] + vals[6] + vals[7];
    }
    
    sink = global_acc;
}

int main(void) {
    int data[SIZE];
    uint32_t seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (int)(lcg(&seed) % 1000);
    }
    
    /* Call kernels with different dependency patterns */
    kernel_mem_deps(data, ITERATIONS, 8);
    kernel_arith_deps(data, ITERATIONS);
    kernel_mixed_deps(data, ITERATIONS);
    
    /* Final volatile sink to prevent DCE */
    __asm__ volatile("" : : "r" (global_acc));
    
    return global_acc & 0xFF;  /* Return non-zero to indicate execution */
}
