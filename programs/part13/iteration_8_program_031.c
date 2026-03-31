/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Prevent optimization */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" : : : "memory")

/* Kernel 1: Memory aliasing and pointer arithmetic with complex control flow */
__attribute__((noinline, noipa))
static void kernel_mem_aliasing(int* data, int n, int* result) {
    int* p1 = data;
    int* p2 = data + n/2;
    int* p3 = data + n/4;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < n; ++i) {
        /* Basic block 1: RAW dependencies with memory */
        tmp1 = *p1;
        *p1 = tmp1 + i;
        
        /* Anti-dependency (WAR) */
        tmp2 = *p2;
        *p2 = tmp2 * 2;
        *p2 = tmp2 + i;  /* WAW on *p2 */
        
        /* Complex conditional creating control dependencies */
        if (i & 1) {
            /* Nested loop inside conditional block */
            for (int j = 0; j < 3; ++j) {
                tmp3 = *p3 + j;
                *p3 = tmp3 - i;  /* WAR inside nested loop */
                acc1 += tmp3;
            }
            
            /* Output dependency chain (WAW) */
            tmp4 = acc1;
            tmp4 = acc2;  /* WAW on tmp4 */
            tmp4 = acc3;
            
            /* Pointer aliasing - p3 may alias p1 or p2 */
            *p3 = *p1 + *p2;
        } else {
            /* Alternative path with different dependencies */
            tmp3 = *p3;
            *p3 = tmp3 / (i + 1);  /* Higher latency division */
            acc2 += tmp3 * 3;
            
            /* Register pressure: many live variables */
            int live1 = acc1, live2 = acc2, live3 = acc3;
            int live4 = tmp1, live5 = tmp2, live6 = tmp3;
            KEEP_ALIVE(live1); KEEP_ALIVE(live2); KEEP_ALIVE(live3);
            KEEP_ALIVE(live4); KEEP_ALIVE(live5); KEEP_ALIVE(live6);
        }
        
        /* Loop-carried dependencies */
        acc3 = acc1 + acc2;  /* RAW from previous iteration's acc1/acc2 */
        
        /* Pointer updates with modulo to create aliasing */
        p1 = data + ((i + 1) % (n/2));
        p2 = data + ((i * 2) % (n/2));
        p3 = data + ((i * 3) % (n/4));
        
        /* Memory barrier to prevent reordering */
        MEM_BARRIER();
    }
    
    *result = acc1 + acc2 + acc3;
    KEEP_ALIVE(*result);
}

/* Kernel 2: Arithmetic dependency chains with varying latencies */
__attribute__((noinline, noipa))
static void kernel_arithmetic_chains(int* data, int n, int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, j = 9, k = 10;
    
    int sum = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long RAW dependency chain */
        a = b + data[i];      /* Cycle 1 */
        b = c * a;            /* Cycle 2: depends on a */
        c = d - b;            /* Cycle 3: depends on b */
        d = e / (c + 1);      /* Cycle 4: higher latency division, depends on c */
        e = f % (d + 2);      /* Cycle 5: higher latency modulo, depends on d */
        
        /* Parallel chain with anti-dependencies */
        f = g + i;            /* WAR: g was read earlier in prev iteration */
        g = h * f;            /* RAW */
        h = j - g;            /* RAW */
        j = k / (h + 1);      /* RAW with division */
        k = a + j;            /* Cross-chain dependency */
        
        /* Output dependencies (WAW) in conditional blocks */
        int tmp = a;
        if (i % 3 == 0) {
            tmp = b * c;      /* WAW on tmp */
        } else if (i % 3 == 1) {
            tmp = d + e;      /* WAW on tmp */
        } else {
            tmp = f - g;      /* WAW on tmp */
        }
        
        /* Complex expression with many operands */
        sum += ((a * b) / (c + 1)) + ((d % e) * (f - g)) - (h / (j + 1)) + (k * tmp);
        
        /* Loop-carried output dependency */
        data[i % 16] = sum;   /* WAW on array elements across iterations */
        
        /* Volatile to prevent elimination */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e),
                        "+r"(f), "+r"(g), "+r"(h), "+r"(j), "+r"(k));
    }
    
    *result = sum;
    global_acc += sum;
}

/* Kernel 3: Nested loops with mixed dependencies */
__attribute__((noinline, noipa))
static void kernel_nested_loops(int* data, int n, int* result) {
    int matrix[4][4] = {{0}};
    int vec[4] = {0};
    int acc = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Initialize vector with data-dependent values */
        for (int v = 0; v < 4; ++v) {
            vec[v] = data[(i + v) % n] + v;
        }
        
        /* Matrix-vector multiplication with dependencies */
        for (int row = 0; row < 4; ++row) {
            int row_sum = 0;
            for (int col = 0; col < 4; ++col) {
                /* RAW: matrix value used then updated */
                int mat_val = matrix[row][col];
                matrix[row][col] = mat_val + vec[col] * i;
                row_sum += mat_val;
                
                /* Anti-dependency across inner loop iterations */
                vec[col] = vec[col] + row_sum;  /* WAR on vec[col] */
            }
            
            /* Output dependency in outer loop */
            acc = row_sum;  /* WAW on acc */
            
            /* Conditional with loop-variant condition */
            if ((i + row) & 1) {
                acc = acc * 2 - row_sum;
            } else {
                acc = acc / 2 + row_sum;
            }
        }
        
        /* Reduction with loop-carried dependency */
        static int static_acc = 0;  /* Force memory dependency */
        static_acc += acc;
        acc = static_acc;
        
        /* Complex pointer arithmetic with potential aliasing */
        int* ptr1 = &matrix[i % 4][0];
        int* ptr2 = &vec[0];
        *ptr1 = *ptr2 + *(ptr2 + 1);  /* May alias */
        
        /* Keep many values live */
        KEEP_ALIVE(matrix[0][0]); KEEP_ALIVE(matrix[1][1]);
        KEEP_ALIVE(vec[0]); KEEP_ALIVE(vec[3]);
        KEEP_ALIVE(row_sum);
    }
    
    *result = acc;
    global_acc += acc;
}

/* Simple PRNG to avoid library dependencies */
static uint32_t lcg = 123456789;
static uint32_t rand_lcg(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main(void) {
    int data[SIZE];
    int result1, result2, result3;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (int)(rand_lcg() % 1000);
    }
    
    /* Run kernels with complex dependency patterns */
    kernel_mem_aliasing(data, ITERS, &result1);
    kernel_arithmetic_chains(data, ITERS, &result2);
    kernel_nested_loops(data, ITERS / 2, &result3);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + global_acc;
    
    /* Volatile sink */
    sink = final_result;
    
    return final_result != 0;
}
