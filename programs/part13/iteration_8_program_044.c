/* ddg_edge_trigger.c
 * Program designed to trigger DDG edge creation with complex dependencies
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

/* Kernel 1: Heavy memory aliasing with mixed dependencies */
NOINLINE static void kernel_memory_aliasing(int* data, int n, int* result) {
    int* p1 = data;
    int* p2 = data + n/2;
    int* p3 = data + n/4;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency (RAW) chain */
            tmp1 = p1[i] + p2[i % (n/2)];
            tmp2 = tmp1 * 3;
            tmp3 = tmp2 / 2;
            acc1 += tmp3;
            
            /* Anti-dependency (WAR) */
            int read_before_write = p3[i % (n/4)];
            p3[i % (n/4)] = acc1;
            acc2 += read_before_write;
            
            /* Memory aliasing with potential true dependency */
            *p1 = acc1;
            acc3 += *p2;
        } else {
            /* Different dependency pattern in else branch */
            tmp4 = p2[i % (n/2)] - p3[i % (n/4)];
            
            /* Output dependency (WAW) */
            int tmp = tmp4 * 2;
            if (tmp > 100) {
                tmp = tmp / 3;  // WAW on tmp
            } else {
                tmp = tmp + 7;  // WAW on tmp
            }
            acc2 += tmp;
            
            /* Complex memory access pattern */
            p1[i % 16] = acc2;
            acc3 += p1[(i + 1) % 16];
        }
        
        /* Nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            if ((i + j) & 1) {
                acc3 += j * p1[(i + j) % 16];
            }
        }
        
        /* Cross-iteration dependency (loop-carried) */
        static int carry = 0;
        int old_carry = carry;
        carry = acc1 + acc2;
        acc3 += old_carry;
    }
    
    *result = acc1 + acc2 + acc3;
    KEEP_ALIVE(*result);
}

/* Kernel 2: Arithmetic chains with varying latencies and control flow */
NOINLINE static void kernel_arithmetic_chains(int* data, int n, int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int k = 11, l = 12, m = 13, o = 14, p = 15;
    
    int sum = 0;
    
    for (int idx = 0; idx < n; ++idx) {
        /* Long true dependency chain with mixed operations */
        int val = data[idx % SIZE];
        
        /* High latency operation (division) */
        if (val != 0) {
            a = b / (val | 1);  // Avoid division by zero
        } else {
            a = b + 1;
        }
        
        /* Medium latency operations */
        c = a * 3;
        d = c - val;
        
        /* Branch creating control dependency */
        if (d > 100) {
            e = d % 13;  // Another high latency op
            f = e * 2;
        } else {
            e = d + 7;
            f = e / 2;
        }
        
        /* More dependencies */
        g = f + a;
        h = g * c;
        
        /* Output dependencies (WAW) */
        int tmp = h;
        tmp = tmp + d;  // WAW
        tmp = tmp * 2;  // WAW
        
        /* Anti-dependencies (WAR) */
        int read_tmp = tmp;
        tmp = read_tmp + e;
        i = tmp;
        
        /* Another dependency chain */
        j = i + g;
        k = j - h;
        l = k * 3;
        m = l / 2;
        o = m + j;
        p = o - k;
        
        /* Mix all results */
        sum += a + b + c + d + e + f + g + h + i + j + k + l + m + o + p;
        
        /* Volatile to prevent elimination */
        KEEP_ALIVE(sum);
        
        /* Loop-carried dependency */
        b = sum % 1000;
    }
    
    *result = sum;
    global_acc += *result;
}

/* Kernel 3: Complex control flow with nested loops */
NOINLINE static void kernel_control_flow(int* data, int n, int* result) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Switch-like control flow */
        switch (i % 4) {
            case 0:
                r1 = data[i % SIZE] + r10;
                r2 = r1 * 2;
                /* Nested loop */
                for (int k = 0; k < 2; ++k) {
                    r3 += r2 + k;
                }
                break;
            case 1:
                r4 = data[(i + 1) % SIZE] - r3;
                r5 = r4 / 2;
                /* Conditional inside case */
                if (r5 > 50) {
                    r6 = r5 % 7;
                } else {
                    r6 = r5 + 3;
                }
                break;
            case 2:
                r7 = data[(i + 2) % SIZE] * r6;
                r8 = r7 + r2;
                /* Small unrolled loop */
                r9 += r8;
                r9 += r8 + 1;
                r9 += r8 + 2;
                break;
            case 3:
                r10 = data[(i + 3) % SIZE] ^ r9;
                /* Multiple assignments (WAW) */
                int temp = r10;
                temp = temp + r1;
                temp = temp - r4;
                temp = temp * r7;
                r1 = temp;  // Cross-variable assignment
                break;
        }
        
        /* Loop-carried dependencies across cases */
        static int state = 0;
        int old_state = state;
        
        /* Complex state update with data-dependent condition */
        if (data[i % SIZE] > 500) {
            state = (old_state + r1 + r4 + r7 + r10) % 100;
        } else {
            state = (old_state - r2 - r5 - r8) % 100;
        }
        
        /* Use state in computation */
        r3 += old_state;
    }
    
    *result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    KEEP_ALIVE(*result);
}

/* Simple PRNG to avoid library dependencies */
static inline int simple_rand(int* seed) {
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
    
    /* Call kernels with overlapping data regions to encourage aliasing */
    kernel_memory_aliasing(data, ITERATIONS, &res1);
    kernel_arithmetic_chains(data + SIZE/4, ITERATIONS, &res2);
    kernel_control_flow(data + SIZE/2, ITERATIONS, &res3);
    
    /* Combine results to prevent dead code elimination */
    int final_result = res1 + res2 + res3 + global_acc;
    
    /* Volatile sink */
    sink = final_result;
    
    /* Also use asm to ensure computation isn't optimized away */
    asm volatile("" : : "r"(final_result));
    
    return final_result & 0xFF;
}
