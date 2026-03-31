/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Prevent optimization */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" : : : "memory")

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
__attribute__((noinline, noipa))
static void kernel_memory_aliasing(int* data, int n, int* result) {
    int sum = 0;
    int* p1 = data;
    int* p2 = data + n/2;
    
    for (int i = 0; i < n; ++i) {
        /* True dependency (RAW) through memory */
        int val1 = *p1;
        *p1 = val1 * 2 + i;
        
        /* Anti-dependency (WAR) - read before write to same location */
        int temp = *p2;
        *p2 = temp + val1;
        
        /* Output dependency (WAW) on local variable */
        int tmp = val1;
        if (i & 1) {
            tmp = temp * 3;
        } else {
            tmp = val1 / 2;
        }
        tmp = tmp + i;  // WAW on tmp
        
        /* Complex pointer arithmetic that may alias */
        p1 = data + ((i * 17) % n);
        p2 = data + ((i * 13 + 7) % n);
        
        /* Mix with register dependencies */
        int r1 = tmp;
        int r2 = r1 * r1;      // RAW
        int r3 = r2 - r1;      // RAW
        r1 = r3 + i;           // WAW on r1
        
        sum += r1 + temp;
        
        /* Control dependency */
        if (i % 3 == 0) {
            sum += *p1;
        } else if (i % 3 == 1) {
            sum -= *p2;
        } else {
            sum *= 2;
        }
    }
    
    *result = sum;
    KEEP_ALIVE(sum);
}

/* Kernel 2: Arithmetic chains with varying latencies */
__attribute__((noinline, noipa))
static void kernel_arithmetic_chains(int* data, int n, int* result) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    
    for (int i = 0; i < n; ++i) {
        /* Long arithmetic chain with mixed operations */
        t1 = data[i];
        t2 = t1 * t1;           // RAW
        t3 = t2 % 17;           // RAW (high latency)
        t4 = t3 + i;            // RAW
        t5 = t4 / 3;            // RAW (high latency)
        t6 = t5 - t1;           // RAW
        t7 = t6 << 2;           // RAW
        t8 = t7 | 0xFF;         // RAW
        t9 = t8 ^ t3;           // RAW
        t10 = t9 & 0xFFFF;      // RAW
        
        /* Parallel chains that converge */
        int chain_a = t1 + t2;
        int chain_b = t3 - t4;
        chain_a = chain_a * 2;  // WAW
        chain_b = chain_b / 2;  // WAW
        
        /* Convergence point with anti-dependency */
        int old_acc1 = acc1;    // Read acc1
        acc1 = chain_a + chain_b + old_acc1;  // Write acc1 (WAR)
        
        /* Nested loop for additional basic blocks */
        for (int j = 0; j < 3; ++j) {
            int inner = t10 + j;
            acc2 += inner * (j + 1);
            
            /* Output dependency in inner loop */
            inner = inner + acc2;  // WAW on inner
            acc3 ^= inner;
        }
        
        /* Complex control flow */
        switch (i % 4) {
            case 0:
                acc1 += t10;
                break;
            case 1:
                acc2 -= t10;
                break;
            case 2:
                acc3 *= t10 | 1;
                break;
            case 3:
                acc1 = acc2 + acc3;  // WAW on acc1
                break;
        }
        
        /* Memory dependency with potential aliasing */
        data[(i + 1) % n] = acc1;
        int mem_val = data[i % n];  // Could be RAW or WAR depending on overlap
        
        acc2 += mem_val;
    }
    
    *result = acc1 + acc2 + acc3;
    KEEP_ALIVE(acc1);
    KEEP_ALIVE(acc2);
    KEEP_ALIVE(acc3);
}

/* Kernel 3: Complex control flow with mixed dependencies */
__attribute__((noinline, noipa))
static void kernel_control_flow(int* data, int n, int* result) {
    int x = 0, y = 0, z = 0;
    int a, b, c, d, e, f;
    
    for (int i = 0; i < n; ++i) {
        /* Initial computations */
        a = data[i];
        b = a + i;
        c = b * 2;
        
        /* Branch with different dependency patterns */
        if (i & 1) {
            /* Path 1: Memory intensive */
            d = data[n - i - 1];
            data[i] = c + d;  // Memory write
            e = data[i];      // Memory read (could be RAW or WAR)
            f = e * 3;
            
            /* Anti-dependency chain */
            int old_x = x;    // Read x
            x = f + old_x;    // Write x (WAR)
            y = x - a;
        } else {
            /* Path 2: Compute intensive */
            d = c % 17;
            e = d * d;
            f = e / 5;
            
            /* Output dependencies */
            int tmp = x;
            tmp = y + f;      // WAW on tmp
            tmp = tmp * 2;    // WAW on tmp
            
            x = tmp;
            y = x + i;
        }
        
        /* Loop-carried dependency with distance > 0 */
        z = z + x;  // z has loop-carried dependency
        
        /* Another level of nesting */
        for (int k = 0; k < 2; ++k) {
            int inner_tmp = y + k;
            if (k == 0) {
                inner_tmp = inner_tmp * inner_tmp;
            } else {
                inner_tmp = inner_tmp / 3;
            }
            z += inner_tmp;
        }
        
        /* Complex condition with side effects */
        int cond = (i % 8);
        if (cond == 0) {
            data[(i + 2) % n] = z;
        } else if (cond < 4) {
            y = data[(i + 1) % n] + y;
        } else {
            x = x ^ y;
        }
        
        /* Volatile to prevent reordering */
        MEM_BARRIER();
    }
    
    *result = x + y + z;
    KEEP_ALIVE(x);
    KEEP_ALIVE(y);
    KEEP_ALIVE(z);
}

/* Simple PRNG to initialize data without library calls */
static int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main(void) {
    int data[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 1000;
    }
    
    int res1, res2, res3;
    
    /* Call kernels with different access patterns */
    kernel_memory_aliasing(data, ITERATIONS, &res1);
    kernel_arithmetic_chains(data, ITERATIONS / 2, &res2);
    kernel_control_flow(data, ITERATIONS, &res3);
    
    /* Combine results to prevent elimination */
    int final_result = res1 + res2 + res3;
    
    /* Use result to prevent dead code elimination */
    sink = final_result;
    
    /* Also use asm to ensure the value is used */
    asm volatile("" : : "r"(final_result));
    
    return final_result & 0xFF;
}
