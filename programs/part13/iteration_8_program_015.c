/* ddg_edge_coverage.c
 * 
 * This program is designed to trigger the DDG edge creation logic in GCC's
 * scheduler, specifically the initialization of ddg_edge structures.
 * It creates complex loop-carried dependencies across multiple basic blocks
 * to ensure the compiler's DDG construction pass executes the uncovered
 * edge initialization code.
 */

#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
static volatile int global_sink = 0;

/* Prevent inlining and IPA optimizations */
#define NOOPT __attribute__((noinline, noipa))

/* Memory-intensive kernel with pointer aliasing */
NOOPT static void kernel_mem(int *arr, int n, int *result) {
    int sum = 0;
    int *p1, *p2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency (RAW) with memory */
            p1 = &arr[i];
            p2 = &arr[(i + 1) % n];
            int val = *p1;          /* Read */
            *p2 = val * 2;          /* Write - creates memory dependency */
            sum += *p1;             /* Read again - anti-dependency (WAR) */
        } else {
            /* Different pointer arithmetic to confuse alias analysis */
            p1 = arr + (i % 16);
            p2 = arr + ((i * 7) % 16);
            *p1 = i;                /* Write */
            sum += *p2;             /* Read - potential memory dependency */
        }
        
        /* Nested loop to create more complex control flow */
        for (int j = 0; j < 3; ++j) {
            /* Output dependency (WAW) on local variable */
            int tmp = arr[i] + j;
            tmp = tmp * tmp;        /* WAW on tmp */
            sum += tmp;
        }
        
        /* Control-dependent computation */
        if (sum > 1000) {
            sum = sum / 2;          /* Higher latency division */
        } else {
            sum = sum + i;          /* Lower latency addition */
        }
    }
    
    /* Use result and volatile asm to keep everything live */
    *result = sum;
    asm volatile("" : : "r"(sum));
}

/* Arithmetic-intensive kernel with long dependency chains */
NOOPT static void kernel_arith(int *arr, int n, int *result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    int g = 7, h = 8, j = 9, k = 10;
    
    for (int i = 0; i < n; ++i) {
        /* Long chain of true dependencies (RAW) */
        a = arr[i] + b;
        b = a * c;
        c = b - d;
        d = c / (arr[i] + 1);       /* Higher latency division */
        e = d % 7;                  /* Higher latency modulo */
        f = e << 2;
        g = f | h;
        h = g ^ j;
        j = h + k;
        k = j * 2;
        
        /* Anti-dependencies (WAR) with the same variables */
        int old_a = a;
        a = b + c;                  /* WAR: a is written after old_a read */
        int old_b = b;
        b = old_a * old_b;
        
        /* Output dependencies (WAW) */
        int tmp = a + b;
        tmp = c + d;                /* WAW on tmp */
        tmp = tmp * 2;
        
        /* Complex condition creating control flow */
        if ((i % 8) == 0) {
            a = a + tmp;
            b = b - tmp;
        } else if ((i % 8) == 1) {
            c = c * tmp;
            d = d / (tmp + 1);
        } else {
            e = e ^ tmp;
            f = f | tmp;
        }
        
        /* Loop-carried dependency with distance > 0 */
        arr[(i + 2) % n] = arr[i] + a;
    }
    
    /* Combine results and force them to be live */
    int total = a + b + c + d + e + f + g + h + j + k;
    *result = total;
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
}

/* Kernel with complex control flow and mixed dependencies */
NOOPT static void kernel_control(int *arr, int n, int *result) {
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Switch-like structure for multiple basic blocks */
        switch (i % 4) {
            case 0:
                /* Memory and arithmetic mix */
                acc1 = arr[i] * 3;
                arr[(i + 1) % n] = acc1;  /* Memory dep with distance 1 */
                sum += acc1;
                break;
            case 1:
                /* Anti-dependency pattern */
                int temp = arr[i];
                arr[i] = acc2;            /* WAR: arr[i] written after read */
                acc2 = temp + acc2;
                sum += acc2;
                break;
            case 2:
                /* Output dependency chain */
                acc3 = i * i;
                acc3 = acc3 + arr[i];     /* WAW on acc3 */
                acc3 = acc3 % 17;         /* WAW on acc3 */
                sum += acc3;
                break;
            case 3:
                /* Nested loop with dependencies */
                for (int k = 0; k < 2; ++k) {
                    acc1 = acc1 + acc2;
                    acc2 = acc2 - acc3;
                    acc3 = acc1 * acc2;
                }
                sum += acc1 + acc2 + acc3;
                break;
        }
        
        /* Loop-carried dependency with variable distance */
        if (i > 10) {
            arr[i % 10] = sum;
        }
        
        /* Volatile asm to prevent elimination of any computation */
        asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3));
    }
    
    *result = sum;
}

/* Simple PRNG to initialize data without library calls */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    const int N = 1024;
    const int ITERS = 1000;
    int data[N];
    int result1, result2, result3;
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < N; ++i) {
        data[i] = simple_rand(&seed) % 100;
    }
    
    /* Call all kernels to maximize coverage chances */
    kernel_mem(data, ITERS, &result1);
    kernel_arith(data, ITERS, &result2);
    kernel_control(data, ITERS, &result3);
    
    /* Combine results and use them */
    int final_result = result1 + result2 + result3;
    global_sink = final_result;
    
    /* Prevent dead code elimination of entire program */
    asm volatile("" : : "r"(final_result));
    
    return final_result > 0 ? 0 : 1;
}
