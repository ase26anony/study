/* ddg_edge_coverage.c
 * 
 * This program is designed to trigger the creation of ddg_edge structures
 * in GCC's DDG (Data Dependency Graph) builder, specifically targeting
 * the edge initialization code in ddg.cc lines 749-757.
 * 
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-ddg -fdump-rtl-sched1 ddg_edge_coverage.c -o ddg_test
 */

#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

/* Global sink to prevent dead code elimination */
static volatile int global_sink;

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noipa))

/* Complex kernel 1: Memory aliasing with mixed dependencies */
NOINLINE static int kernel_memory_aliasing(int *data, int n, int offset) {
    int *p1 = data;
    int *p2 = data + offset;
    int *p3 = data + offset * 2;
    
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < n; ++i) {
        /* True dependency (RAW) chain */
        tmp1 = p1[i] + i;
        tmp2 = tmp1 * p2[i];
        tmp3 = tmp2 / (p3[i] + 1);
        
        /* Anti-dependency (WAR): read before write */
        int read_before_write = p1[i];
        p1[i] = tmp3 + read_before_write;
        
        /* Output dependency (WAW) with branching */
        if (i & 1) {
            tmp4 = tmp2 % 7;  /* Higher latency operation */
        } else {
            tmp4 = tmp1 - tmp3; /* Lower latency operation */
        }
        
        /* Another WAW on tmp4 */
        tmp4 = tmp4 + (p2[i] << 2);
        
        /* Memory aliasing: p1 and p2 may alias */
        *p2 = *p1 + tmp4;
        sum += *p2;
        
        /* Control-dependent computation */
        if (tmp4 > 100) {
            p3[i] = sum % 256;
        } else {
            p3[i] = tmp4 % 128;
        }
        
        /* Nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            /* Inter-iteration dependency (loop-carried) */
            static int carry = 0;
            int local_carry = carry;
            carry = (local_carry + p1[i] + j) & 0xFF;
            sum += local_carry;
        }
    }
    
    /* Use volatile asm to keep variables live */
    asm volatile("" : : "r"(sum), "r"(tmp1), "r"(tmp2), "r"(tmp3), "r"(tmp4));
    return sum;
}

/* Complex kernel 2: Arithmetic chains with varying latencies */
NOINLINE static int kernel_arithmetic_chains(int *data, int n) {
    int a, b, c, d, e, f, g, h, j, k, l, m;
    int sum = 0;
    
    /* Initialize many live variables */
    a = data[0];
    b = data[1];
    c = data[2];
    d = data[3];
    e = data[4];
    f = data[5];
    g = data[6];
    h = data[7];
    
    for (int i = 0; i < n; ++i) {
        /* Long true dependency chain with mixed operations */
        int idx = i % 256;
        
        /* Chain 1: Integer operations with varying latencies */
        a = data[idx] + b;
        b = a * c;
        c = b / (data[idx + 1] + 1);  /* Higher latency division */
        d = c - a;
        e = d % 13;  /* Higher latency modulo */
        
        /* Chain 2: Parallel chain with anti-dependencies */
        f = g + h;
        g = data[idx + 2] * f;  /* WAR: f read before potential write */
        h = g >> 2;
        
        /* Output dependencies (WAW) with branching */
        if (idx & 3) {
            j = e + f;
        } else {
            j = g - h;
        }
        
        /* Another WAW on j */
        j = j * 2 + (idx & 1);
        
        /* Control flow creates multiple basic blocks */
        switch (idx % 4) {
            case 0:
                k = a + b;
                l = k * j;
                break;
            case 1:
                k = c - d;
                l = k / (j + 1);
                break;
            case 2:
                k = e ^ f;
                l = k & g;
                break;
            default:
                k = h | j;
                l = k << 1;
                break;
        }
        
        /* Loop-carried dependency with distance > 0 */
        static int prev_l = 0;
        m = l + prev_l;
        prev_l = l;
        
        /* Memory dependency with potential aliasing */
        data[(idx + 8) % 256] = m;
        sum += data[idx % 256];
        
        /* Use all variables to keep them live */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                         "r"(f), "r"(g), "r"(h), "r"(j), "r"(k), "r"(l), "r"(m));
    }
    
    global_sink = sum;
    return sum + a + b + c + d + e + f + g + h + j + k + l + m;
}

/* Complex kernel 3: Control flow intensive with nested loops */
NOINLINE static int kernel_control_flow(int *data, int n) {
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple conditionally executed blocks */
        if (i % 3 == 0) {
            /* Block A: Memory intensive */
            int *ptr1 = data + (i % 64);
            int *ptr2 = data + ((i + 1) % 64);
            int val1 = *ptr1;
            *ptr2 = val1 + i;
            acc1 += *ptr1;
        } else if (i % 3 == 1) {
            /* Block B: Arithmetic chain */
            int chain = data[i % 64];
            chain = chain * 3 + 1;
            chain = chain / 2;
            chain = chain % 1023;
            acc2 += chain;
        } else {
            /* Block C: Mixed operations */
            int tmp = data[i % 64];
            for (int j = 0; j < 2; ++j) {
                tmp = (tmp * 1103515245 + 12345) & 0x7FFFFFFF;
                acc3 += tmp;
            }
        }
        
        /* Always executed: creates convergence point */
        if (i > 0) {
            /* Loop-carried anti-dependency */
            int prev_sum = sum;
            sum = prev_sum + acc1 + acc2 + acc3;
            acc4 = prev_sum;  /* WAR on prev_sum */
        }
        
        /* Nested loop with its own dependencies */
        int inner_acc = 0;
        for (int k = 0; k < 4; ++k) {
            inner_acc = inner_acc * 31 + data[(i + k) % 64];
            /* Output dependency in inner loop */
            int inner_tmp = inner_acc;
            inner_tmp = inner_tmp ^ k;
            acc4 += inner_tmp;
        }
        
        /* Final reduction with output dependency */
        sum = sum + acc4;
        acc4 = 0;
    }
    
    /* Force all accumulators to be live */
    asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3), "r"(acc4));
    return sum;
}

/* Simple PRNG to initialize data without library calls */
static void init_data(int *data, int size) {
    uint32_t seed = 123456789;
    for (int i = 0; i < size; ++i) {
        seed = seed * 1103515245 + 12345;
        data[i] = (int)(seed % 1000);
    }
}

int main() {
    /* Large data set to work with */
    int data[SIZE];
    
    /* Initialize with pseudo-random values */
    init_data(data, SIZE);
    
    /* Call each kernel with different access patterns */
    int result = 0;
    
    /* Kernel 1: Memory aliasing patterns */
    result += kernel_memory_aliasing(data, ITERATIONS, 16);
    
    /* Kernel 2: Arithmetic chains */
    result += kernel_arithmetic_chains(data + 256, ITERATIONS / 2);
    
    /* Kernel 3: Control flow intensive */
    result += kernel_control_flow(data + 512, ITERATIONS);
    
    /* Use result to prevent elimination */
    global_sink = result;
    
    /* Final volatile asm to prevent dead code elimination */
    asm volatile("" : : "r"(result));
    
    return result & 0xFF;
}
