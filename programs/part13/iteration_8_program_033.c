/* ddg_edge_coverage.c
 * Complex loop-carried dependency patterns to trigger DDG edge creation
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-ddg -fdump-rtl-sched1
 */

#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory-heavy with pointer aliasing and complex control flow */
NOINLINE static void kernel_memory_aliasing(int* data, int n) {
    int* p1 = data;
    int* p2 = data + n/2;
    int* p3 = data + n/4;
    
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if/else */
        if (i & 1) {
            /* True dependency chain (RAW) */
            tmp1 = *p1 + i;
            tmp2 = tmp1 * 2;
            tmp3 = tmp2 - *p2;
            
            /* Anti-dependency (WAR) - read before write */
            a = *p3;          /* Read */
            *p3 = tmp3;       /* Write to same location */
            
            /* Output dependency (WAW) */
            tmp4 = a + b;     /* First write to tmp4 */
            if (i & 2) {
                tmp4 = c * d; /* Second write to tmp4 */
            }
            
            /* Memory dependency with potential aliasing */
            *p1 = tmp4;
            b = *p2;          /* May alias with p1 */
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = *p2 - i;
            tmp2 = tmp1 / 3;  /* Higher latency operation */
            
            /* Nested loop inside basic block */
            for (int j = 0; j < 3; ++j) {
                c = c + tmp2 * j;
                d = d - *p1;
            }
            
            /* Complex memory access pattern */
            p1[0] = c;
            p2[1] = d;
            e = p3[2];
        }
        
        /* Loop-carried dependencies */
        f = f + a + b + c + d + e;
        
        /* Pointer arithmetic that may cause aliasing */
        p1 = data + ((i * 7) % n);
        p2 = data + ((i * 13) % n);
        p3 = data + ((i * 19) % n);
        
        /* Prevent dead code elimination */
        KEEP_ALIVE(tmp1);
        KEEP_ALIVE(tmp2);
        KEEP_ALIVE(tmp3);
        KEEP_ALIVE(tmp4);
    }
    
    global_acc += f;
    sink = f;
}

/* Kernel 2: Arithmetic-heavy with register dependencies and varying latencies */
NOINLINE static void kernel_arithmetic_chains(int* data, int n) {
    int r0 = 1, r1 = 2, r2 = 3, r3 = 4, r4 = 5, r5 = 6;
    int r6 = 7, r7 = 8, r8 = 9, r9 = 10, r10 = 11;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple parallel dependency chains */
        
        /* Chain 1: Mixed latency operations */
        r0 = r0 + data[i % SIZE];
        r1 = r0 * r1;        /* RAW on r0 */
        r2 = r1 % 17;        /* High latency modulo */
        r3 = r2 - r3;        /* RAW on r2 */
        
        /* Chain 2: With control dependencies */
        if (r3 > 0) {
            r4 = r4 * r4;
            r5 = r4 / 3;     /* High latency division */
        } else {
            r4 = r4 + r5;
            r5 = r5 - 1;
        }
        
        /* Chain 3: Cross-iteration dependencies */
        r6 = r6 + r7;
        r7 = r8 - r9;        /* WAW hazard with below */
        r8 = r10 * data[(i + 1) % SIZE];
        r7 = r6 + r8;        /* WAW on r7 */
        
        /* Chain 4: Anti-dependencies */
        int temp = r9;       /* Read r9 */
        r9 = r10 + i;        /* Write r9 */
        r10 = temp * 2;      /* Use old value of r9 */
        
        /* Complex conditional with nested blocks */
        switch (i % 4) {
            case 0:
                r0 = r1 + r2;
                r3 = r4 * r5;
                break;
            case 1:
                r1 = r2 - r3;
                r4 = r5 / 2;
                break;
            case 2:
                r2 = r3 % 7;
                r5 = r6 + r7;
                break;
            default:
                r3 = r4 | r5;
                r6 = r7 ^ r8;
                break;
        }
        
        /* Loop-carried output dependency */
        static int persistent = 0;
        int old_persistent = persistent;
        persistent = r0 + r1 + r2;
        r9 = old_persistent;
        
        /* Keep all registers live */
        KEEP_ALIVE(r0); KEEP_ALIVE(r1); KEEP_ALIVE(r2);
        KEEP_ALIVE(r3); KEEP_ALIVE(r4); KEEP_ALIVE(r5);
        KEEP_ALIVE(r6); KEEP_ALIVE(r7); KEEP_ALIVE(r8);
        KEEP_ALIVE(r9); KEEP_ALIVE(r10);
    }
    
    int result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    global_acc += result;
    sink = result;
}

/* Kernel 3: Complex control flow with nested loops */
NOINLINE static void kernel_control_flow(int* data, int n) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int t1, t2, t3, t4;
    
    for (int i = 0; i < n; ++i) {
        /* Outer loop with multiple basic blocks */
        
        /* Block A: Initial computations */
        t1 = data[i] * 3;
        t2 = data[i + 1] + 5;
        
        /* Conditional with inner loop */
        if (t1 > t2) {
            for (int j = 0; j < 4; ++j) {
                /* Inner loop dependencies */
                int inner_tmp = t1 + j;
                t2 = t2 - inner_tmp;  /* WAR: t2 read then written */
                t1 = t1 * 2;          /* WAW: t1 written again */
                acc1 += t1 + t2;
            }
        } else {
            /* Alternative path */
            for (int j = 0; j < 2; ++j) {
                t3 = t1 + t2;
                t4 = t3 % 11;         /* High latency */
                t1 = t4 * j;          /* RAW on t4 */
                acc2 += t1;
            }
        }
        
        /* Another conditional block */
        switch (i % 3) {
            case 0:
                t3 = acc1 * 7;
                acc3 = acc3 + t3;
                break;
            case 1:
                t4 = acc2 / 3;
                acc4 = acc4 - t4;
                break;
            case 2:
                /* Memory dependency chain */
                data[i % SIZE] = acc3;
                t3 = data[(i + 1) % SIZE];  /* May alias */
                acc1 = t3 + acc4;
                break;
        }
        
        /* Loop-carried anti-dependency */
        int read_before_write = acc2;
        acc2 = acc1 + i;
        acc3 = read_before_write * 2;
        
        /* Complex expression with multiple uses */
        acc4 = (acc1 * acc2) + (acc3 / 4) - (acc4 % 5);
        
        /* Force many live variables */
        KEEP_ALIVE(t1); KEEP_ALIVE(t2);
        KEEP_ALIVE(t3); KEEP_ALIVE(t4);
        KEEP_ALIVE(acc1); KEEP_ALIVE(acc2);
        KEEP_ALIVE(acc3); KEEP_ALIVE(acc4);
    }
    
    int result = acc1 + acc2 + acc3 + acc4;
    global_acc += result;
    sink = result;
}

/* Simple PRNG to initialize data without library calls */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    /* Large dataset to work with */
    int data[SIZE];
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 100;
    }
    
    /* Call kernels with different access patterns */
    kernel_memory_aliasing(data, ITERATIONS);
    kernel_arithmetic_chains(data, ITERATIONS);
    kernel_control_flow(data, ITERATIONS);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(global_acc));
    
    return global_acc & 0xFF;
}
