/* Complex DDG edge creation stress test
 * Designed to trigger ddg_edge allocation and initialization
 * in GCC's DDG builder (ddg.cc lines 749-757)
 */

#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v))
#define MEM_BARRIER() asm volatile("" ::: "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing with complex control flow */
NOOPT static int kernel_memory_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    int t1, t2, t3, t4, t5, t6, t7, t8;
    
    /* Multiple live variables to increase register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Create multiple basic blocks with complex control flow */
        if (i & 1) {
            /* True dependency chain (RAW) */
            t1 = arr1[i] + arr2[i];      /* Node A */
            t2 = t1 * 3;                 /* Node B depends on A */
            t3 = t2 / 2;                 /* Node C depends on B */
            
            /* Anti-dependency (WAR) */
            tmp1 = arr1[i];              /* Read arr1[i] */
            arr1[i] = t3 + i;            /* Write arr1[i] - anti-dep on previous read */
            
            /* Output dependency (WAW) */
            tmp2 = t1 + t2;              /* First write to tmp2 */
            if (i & 2) {
                tmp2 = t3 * 7;           /* Second write to tmp2 - output dep */
            }
            
            acc1 += tmp1 + tmp2;
        } else {
            /* Different dependency pattern in else branch */
            t4 = arr2[i] - arr1[i];
            t5 = t4 % 17;                /* Higher latency operation */
            t6 = t5 ^ 0x55AA55AA;
            
            /* Memory aliasing with pointer arithmetic */
            int *p1 = &arr1[i];
            int *p2 = &arr2[(i * 3) % n];
            if (p1 != p2) {
                *p1 = t6;                /* Memory write */
                t7 = *p2 + i;            /* Memory read - potential MEM_DEP */
            }
            
            /* Nested loop for additional basic blocks */
            for (int j = 0; j < 3; j++) {
                t8 = (t7 << j) | (t6 >> j);
                acc2 += t8;
            }
            
            acc3 ^= t4;
        }
        
        /* Cross-iteration dependencies with varying distances */
        if (i > 0) {
            /* Loop-carried true dependency (distance 1) */
            acc4 = acc4 + arr1[i-1];     /* Depends on previous iteration's store */
        }
        
        /* Mix all accumulators to create complex dataflow */
        tmp3 = (acc1 & 0xFF) | (acc2 & 0xFF00);
        tmp4 = (acc3 ^ acc4) + tmp3;
        
        /* Volatile to prevent elimination */
        KEEP_ALIVE(tmp4);
        
        sum += tmp4;
        
        /* Memory barrier to enforce ordering */
        MEM_BARRIER();
    }
    
    /* Combine results */
    int result = sum + acc1 + acc2 + acc3 + acc4;
    KEEP_ALIVE(result);
    return result;
}

/* Kernel 2: Arithmetic chains with varying latencies */
NOOPT static int kernel_arithmetic_chains(int *arr, int n) {
    int sum = 0;
    
    /* Multiple dependency chains running in parallel */
    int chain1 = 1, chain2 = 2, chain3 = 3, chain4 = 4;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < n; i++) {
        /* Chain 1: Integer arithmetic with varying latencies */
        tmp1 = chain1 + arr[i];          /* Low latency ADD */
        tmp1 = tmp1 * 3;                 /* Medium latency MUL */
        if (arr[i] > 0) {
            tmp1 = tmp1 / (arr[i] | 1);  /* High latency DIV */
        }
        chain1 = tmp1 & 0x7FFF;
        
        /* Chain 2: Shift/rotate operations */
        tmp2 = chain2 ^ arr[i];
        tmp2 = (tmp2 << 3) | (tmp2 >> 29);  /* Rotate */
        tmp2 = tmp2 % 19;                   /* High latency MOD */
        chain2 = tmp2 + i;
        
        /* Chain 3: Complex expression with multiple WAR/WAW */
        tmp3 = chain3;
        tmp3 = tmp3 * chain1;            /* WAW on tmp3 */
        int read_tmp3 = tmp3;            /* Read before next write (WAR) */
        tmp3 = read_tmp3 + chain2;       /* WAW on tmp3 */
        chain3 = tmp3 - arr[i];
        
        /* Chain 4: Control-dependent computations */
        if (i % 3 == 0) {
            tmp4 = chain4 * 2;
        } else if (i % 3 == 1) {
            tmp4 = chain4 + chain1;
        } else {
            tmp4 = chain4 - chain2;
        }
        
        /* Cross-chain dependencies */
        chain4 = (tmp4 + chain3) % 256;
        
        /* Loop-carried output dependency */
        static int loop_carried = 0;
        int old_val = loop_carried;
        loop_carried = chain1 + chain2 + chain3 + chain4;
        
        /* Use all chains to prevent dead code elimination */
        sum += old_val + loop_carried;
        
        /* Force memory operations */
        arr[i % 16] = sum & 0xFF;
        tmp1 = arr[(i + 1) % 16];        /* Potential MEM_DEP */
        
        KEEP_ALIVE(tmp1);
    }
    
    return sum;
}

/* Kernel 3: Nested loops with mixed dependencies */
NOOPT static int kernel_nested_loops(int *arr1, int *arr2, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_acc = 0;
        int outer_val = arr1[i] ^ arr2[i];
        
        /* Nested loop creates additional basic blocks */
        for (int j = 0; j < 4; j++) {
            int inner_tmp = outer_val + j;
            
            /* True dependency chain inside inner loop */
            int a = inner_tmp * 2;
            int b = a + (j * 3);
            int c = b % 13;
            
            /* Anti-dependency pattern */
            int read_val = arr2[j];
            arr2[j] = c + i;
            
            /* Output dependency in inner loop */
            int tmp = a + b;
            if (j & 1) {
                tmp = c * 7;            /* WAW on tmp */
            }
            
            inner_acc += tmp + read_val;
            
            /* Memory operation with potential aliasing */
            if ((i + j) & 1) {
                arr1[(i + j) % 8] = inner_acc;
            }
        }
        
        /* Loop-carried dependency with distance > 1 */
        static int history[4] = {0};
        int hist_idx = i % 4;
        int prev = history[hist_idx];
        history[hist_idx] = inner_acc;
        
        /* Complex expression using values from different iterations */
        total += inner_acc * 3 - prev * 2 + outer_val;
        
        /* Volatile access to prevent optimization */
        KEEP_ALIVE(total);
    }
    
    return total;
}

/* Simple PRNG to avoid library dependencies */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    /* Initialize data with pseudo-random values */
    int data1[SIZE];
    int data2[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        data1[i] = (int)(lcg_rand() % 1000);
        data2[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Call all kernels to maximize DDG edge creation opportunities */
    int result1 = kernel_memory_aliasing(data1, data2, ITERS);
    int result2 = kernel_arithmetic_chains(data1, ITERS);
    int result3 = kernel_nested_loops(data1, data2, ITERS);
    
    /* Combine results and use them to prevent dead code elimination */
    int final_result = result1 + result2 + result3;
    
    /* Volatile sink to ensure all computations are kept */
    sink = final_result;
    
    /* Also add to global accumulator */
    global_acc += final_result;
    
    /* Return value based on results to affect exit code */
    return (global_acc > 0) ? 0 : 1;
}
