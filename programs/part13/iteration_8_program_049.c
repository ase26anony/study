/* ddg_edge_coverage.c
 * 
 * This program is designed to trigger the DDG edge creation logic
 * in GCC's scheduler, specifically the initialization of ddg_edge
 * structures (lines 749-757 in ddg.cc).
 *
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-ddg -fdump-rtl-sched1 ddg_edge_coverage.c -o ddg_test
 */

#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
static volatile int global_sink = 0;

/* Simple LCG for pseudo-random values without library calls */
static inline unsigned int lcg(unsigned int *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Kernel 1: Heavy on memory dependencies with pointer aliasing */
__attribute__((noinline, noipa))
static void kernel_memory_deps(int *arr1, int *arr2, int n, int *result) {
    int temp1, temp2, temp3, temp4, temp5;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Multiple pointers that may alias */
    int *p1 = arr1;
    int *p2 = arr1 + (n/2);  /* Potentially aliases with p1 */
    int *p3 = arr2;
    int *p4 = arr2 + 1;
    
    for (int i = 0; i < n; ++i) {
        /* True dependencies (RAW) with memory */
        temp1 = *p1;                /* Read */
        temp2 = temp1 * 2;          /* Use result */
        *p1 = temp2 + i;            /* Write - creates WAW with next iteration */
        
        /* Anti-dependency (WAR) */
        temp3 = *p2;                /* Read before write */
        *p2 = temp3 + temp1;        /* Write after read */
        
        /* Output dependency (WAW) */
        temp4 = *p3;                /* First assignment to temp4 */
        if (i & 1) {
            temp4 = *p4 * 3;        /* Second assignment - WAW */
            acc1 += temp4;
        } else {
            temp4 = temp3 / 2;      /* Alternative WAW */
            acc2 += temp4;
        }
        
        /* Complex memory aliasing pattern */
        if (i % 3 == 0) {
            *p3 = temp2 + temp4;    /* May alias with p1/p2 reads */
        } else if (i % 3 == 1) {
            *p4 = temp1 - temp3;    /* Different store */
        }
        
        /* Pointer arithmetic to create varying access patterns */
        p1 = arr1 + ((i * 7) % n);
        p2 = arr1 + ((i * 11) % n);
        p3 = arr2 + ((i * 13) % (n-1));
        p4 = arr2 + ((i * 17) % (n-1));
        
        /* Accumulate to prevent optimization */
        acc3 += temp1 + temp2;
        acc4 += temp3 * temp4;
    }
    
    /* Force results to be live */
    asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3), "r"(acc4));
    *result = acc1 + acc2 + acc3 + acc4;
}

/* Kernel 2: Arithmetic chains with varying latencies and control flow */
__attribute__((noinline, noipa))
static void kernel_arithmetic_deps(int *arr, int n, int *result) {
    int a, b, c, d, e, f, g, h;
    int acc = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long arithmetic chain with true dependencies */
        a = arr[i] + i;             /* Low latency */
        b = a * 3;                  /* Depends on a */
        c = b / 2;                  /* Higher latency division */
        
        /* Branch creating control dependencies */
        if (i % 4 == 0) {
            d = c << 2;             /* Shift */
            e = d ^ 0x55AA55AA;     /* Bitwise */
        } else if (i % 4 == 1) {
            d = c >> 1;             /* Different shift */
            e = d | 0x12345678;     /* Bitwise OR */
        } else if (i % 4 == 2) {
            d = c % 17;             /* High latency modulo */
            e = d & 0xF0F0F0F0;     /* Bitwise AND */
        } else {
            d = c * 7;              /* Multiplication */
            e = d + 0xABCD;         /* Addition */
        }
        
        /* Nested loop for additional basic blocks */
        for (int j = 0; j < 3; ++j) {
            f = e + j;              /* Loop-carried dependency */
            g = f * (j + 1);        /* Varying computation */
            
            /* Anti-dependency within nested loop */
            h = g;                  /* Read g */
            g = h + arr[j];         /* Write g after read - WAR */
            
            acc += g;
        }
        
        /* Output dependency across iterations */
        static int persistent = 0;  /* Static to force WAW across calls */
        int old_persistent = persistent;
        persistent = e + i;          /* WAW with next iteration's assignment */
        
        /* Complex expression with mixed operations */
        arr[i] = (a + b) * (c - d) / (e + 1) + old_persistent;
        
        /* Volatile asm to prevent elimination */
        asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
    }
    
    *result = acc;
}

/* Kernel 3: Complex control flow with mixed dependencies */
__attribute__((noinline, noipa))
static void kernel_control_flow_deps(int *arr1, int *arr2, int n, int *result) {
    int x1, x2, x3, x4, x5, x6, x7, x8;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple parallel dependency chains */
        x1 = arr1[i];
        x2 = arr2[i];
        
        /* Chain A */
        x3 = x1 * x2;
        x4 = x3 + i;
        
        /* Chain B with conditional */
        if (x1 > x2) {
            x5 = x1 - x2;
            x6 = x5 << 1;
        } else {
            x5 = x2 - x1;
            x6 = x5 >> 1;
        }
        
        /* Chain C with loop-carried dependency */
        static int carry = 0;
        int old_carry = carry;
        x7 = x4 + old_carry;        /* RAW on carry from previous iteration */
        carry = x6 + i;             /* WAW on carry for next iteration */
        
        /* Memory dependency with potential aliasing */
        int *ptr = (i % 2) ? &arr1[i] : &arr2[i];
        x8 = *ptr;                  /* Read */
        *ptr = x7 + x8;             /* Write - creates WAR */
        
        /* Multiple basic blocks with switches */
        switch (i % 5) {
            case 0:
                sum1 += x3 * x5;
                break;
            case 1:
                sum1 += x4 / (x6 + 1);
                break;
            case 2:
                sum1 += x7 ^ x8;
                break;
            case 3:
                sum1 += (x3 + x5) % 256;
                break;
            case 4:
                sum1 += x4 | x6;
                break;
        }
        
        /* Another level of nesting */
        for (int k = 0; k < 2; ++k) {
            int tmp = sum1;
            sum2 += tmp + k;        /* WAR on sum1 */
            sum1 = sum2 - tmp;      /* WAW on sum1 */
        }
        
        /* Complex condition with short-circuit evaluation */
        if (i > n/2 && (x1 < x2 || x3 > x4)) {
            sum3 += x5 * x7;
        } else if (i < n/4) {
            sum3 += x6 / (x8 + 1);
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(x1), "r"(x2), "r"(x3), "r"(x4), 
                              "r"(x5), "r"(x6), "r"(x7), "r"(x8));
    }
    
    *result = sum1 + sum2 + sum3;
}

int main() {
    const int N = 1024;
    const int ITERS = 1000;
    
    /* Initialize data arrays */
    int data1[N], data2[N];
    unsigned int seed1 = 123456789, seed2 = 987654321;
    
    for (int i = 0; i < N; ++i) {
        data1[i] = lcg(&seed1) % 1000;
        data2[i] = lcg(&seed2) % 1000;
    }
    
    int res1, res2, res3;
    
    /* Call kernels with different dependency patterns */
    kernel_memory_deps(data1, data2, ITERS, &res1);
    kernel_arithmetic_deps(data1, ITERS, &res2);
    kernel_control_flow_deps(data1, data2, ITERS, &res3);
    
    /* Combine results and force output */
    int final_result = res1 + res2 + res3;
    global_sink = final_result;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(final_result));
    
    return final_result % 256;  /* Return non-zero to indicate execution */
}
