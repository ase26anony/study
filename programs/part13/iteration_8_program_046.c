/* ddg_edge_coverage.c
 * 
 * This program is designed to trigger the DDG (Data Dependency Graph) edge
 * creation logic in GCC's ddg.cc, specifically the initialization block
 * for ddg_edge structures (lines 749-757 in the target file).
 *
 * It creates complex loop-carried dependencies across multiple basic blocks
 * to force the scheduler's DDG construction to allocate and initialize
 * many dependency edges between instructions.
 */

#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
static volatile int global_sink = 0;

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Memory-intensive kernel with pointer aliasing */
NOINLINE static void kernel_mem(int* arr, int n, int* result) {
    int* p1 = arr;
    int* p2 = arr + (n / 2);
    int* p3 = arr + 1;
    
    int sum = 0;
    int tmp1, tmp2, tmp3;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else */
        if (i & 1) {
            /* True dependency (RAW) with memory */
            tmp1 = *p1 + i;
            *p3 = tmp1 * 2;  /* WAR: p3 may alias with p1/p2 */
            
            /* Anti-dependency (WAR) */
            tmp2 = *p2;
            *p1 = tmp2 + tmp1;  /* WAW on *p1 across iterations */
            
            /* Complex chain */
            tmp3 = tmp1 % 7;  /* Higher latency operation */
            sum += tmp3;
            
            /* Pointer arithmetic that may cause aliasing */
            p1 = arr + ((i * 3) % n);
            p3 = arr + ((i * 5) % n);
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = *p2 - i;
            *p1 = tmp1 / 3;  /* Division has higher latency */
            
            /* Output dependency (WAW) */
            tmp2 = tmp1;
            tmp2 = *p3 + tmp2;  /* Overwrite tmp2 */
            
            /* Memory dependency with potential aliasing */
            *p2 = tmp2;
            sum += *p1;  /* Read after write - may create MEM_DEP */
            
            p2 = arr + ((i * 7) % n);
        }
        
        /* Nested loop to create more complex CFG */
        for (int j = 0; j < 3; ++j) {
            /* Register pressure */
            int local_var = tmp1 + j;
            if (local_var & 1) {
                sum += local_var;
            } else {
                sum -= local_var;
            }
        }
        
        /* Force memory barrier */
        __asm__ volatile("" : : "r"(sum) : "memory");
    }
    
    *result = sum;
    global_sink += sum;
}

/* Arithmetic-intensive kernel with long dependency chains */
NOINLINE static void kernel_arith(int* arr, int n, int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long true dependency chain */
        a = b + arr[i];
        b = c * a;
        c = d - b;
        d = e % (c + 1);  /* Modulo - higher latency */
        e = f / (d + 1);  /* Division - higher latency */
        f = a + e;
        
        /* Parallel chains that converge */
        x = y + arr[n - i - 1];
        y = z * x;
        z = x + y;
        
        /* Control-dependent computations */
        if (i % 4 == 0) {
            a = b + c;
            x = y - z;
        } else if (i % 4 == 1) {
            b = c * d;
            y = z + x;
        } else if (i % 4 == 2) {
            c = d - e;
            z = x * y;
        } else {
            d = e % f;
            x = y / (z + 1);
        }
        
        /* Output dependencies (WAW) */
        int tmp = a + b;
        tmp = c + d;      /* Overwrite tmp */
        tmp = e + f;      /* Overwrite again */
        
        /* Anti-dependencies (WAR) */
        int old_x = x;
        x = tmp + old_x;
        int old_y = y;
        y = old_x - old_y;
        
        /* Use all variables to keep them live */
        __asm__ volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), 
                         "r"(e), "r"(f), "r"(x), "r"(y), "r"(z));
    }
    
    *result = a + b + c + d + e + f + x + y + z;
    global_sink += *result;
}

/* Kernel with complex control flow and mixed dependencies */
NOINLINE static void kernel_mixed(double* arr, int n, double* result) {
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    double t1, t2, t3, t4;
    
    for (int i = 0; i < n; ++i) {
        /* Switch-like control flow */
        switch (i % 5) {
            case 0:
                t1 = arr[i] * 2.0;
                t2 = arr[i + 1] + 1.0;
                acc1 += t1 - t2;
                break;
            case 1:
                t3 = arr[i] / 3.0;
                t4 = arr[i - 1] * 4.0;
                acc2 += t3 + t4;
                break;
            case 2:
                t1 = t3 * t4;  /* Cross-iteration dependency */
                acc3 += t1;
                break;
            case 3:
                /* Memory dependencies with type conversion */
                int int_val = (int)arr[i];
                double dbl_val = (double)int_val;
                acc1 += dbl_val;
                break;
            default:
                /* Complex expression with many operations */
                acc2 += (arr[i] * arr[i]) / (arr[i] + 1.0);
                break;
        }
        
        /* Loop-carried output dependency */
        static double persistent = 0.0;
        double old_persistent = persistent;
        persistent = acc1 + acc2 + acc3;
        
        /* Loop-carried true dependency */
        acc3 = old_persistent * 0.5;
        
        /* Nested loop with its own dependencies */
        for (int j = 0; j < 2; ++j) {
            double inner = acc1 + j;
            if (inner > 0.0) {
                acc2 += inner;
            } else {
                acc1 -= inner;
            }
        }
        
        /* Prevent optimization */
        __asm__ volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3));
    }
    
    *result = acc1 + acc2 + acc3;
    global_sink += (int)*result;
}

/* Simple PRNG to avoid library calls */
static int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    const int N = 1024;
    const int ITERS = 1000;
    
    /* Initialize data with pseudo-random values */
    int int_data[N];
    double double_data[N];
    
    int seed = 42;
    for (int i = 0; i < N; ++i) {
        int_data[i] = simple_rand(&seed) % 100;
        double_data[i] = (double)(simple_rand(&seed) % 100) / 10.0;
    }
    
    int result1, result2;
    double result3;
    
    /* Call kernels sequentially */
    kernel_mem(int_data, ITERS, &result1);
    kernel_arith(int_data, ITERS, &result2);
    kernel_mixed(double_data, ITERS, &result3);
    
    /* Combine results and force output */
    int final_result = result1 + result2 + (int)result3;
    
    /* Volatile assembly to prevent dead code elimination */
    __asm__ volatile("" : : "r"(final_result));
    
    return final_result % 256;
}
