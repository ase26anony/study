/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" ::: "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
static NOOPT void kernel_mem_aliasing(int* arr1, int* arr2, int n) {
    int i;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (i = 0; i < n; ++i) {
        /* Create multiple basic blocks with if/else */
        if (i & 1) {
            /* True dependency chain (RAW) */
            tmp1 = arr1[i] + arr2[i];      /* Node A */
            tmp2 = tmp1 * 2;               /* Node B depends on A */
            tmp3 = tmp2 / 3;               /* Node C depends on B */
            
            /* Anti-dependency (WAR) */
            int read_before = arr1[i];     /* Read arr1[i] */
            arr1[i] = tmp3;                /* Write arr1[i] - anti-dep on read_before */
            
            /* Output dependency (WAW) */
            tmp4 = read_before;            /* First write to tmp4 */
            if (tmp3 > 100) {
                tmp4 = tmp3 * 2;           /* Second write to tmp4 - WAW */
            }
            
            acc1 += tmp4;
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = arr2[i] - arr1[i];
            tmp2 = tmp1 % 17;              /* Higher latency operation */
            tmp3 = arr1[i] ^ tmp2;
            
            /* Memory aliasing with pointer arithmetic */
            int* p1 = &arr1[i];
            int* p2 = &arr2[(i * 7) % n];  /* May alias with p1 */
            
            *p1 = tmp3;                    /* Store through p1 */
            tmp4 = *p2;                    /* Load through p2 - possible MEM_DEP */
            
            acc2 += tmp4;
        }
        
        /* Loop-carried dependency with distance 1 */
        static int lc_var = 0;
        int old_lc = lc_var;
        lc_var = (lc_var + arr1[i]) % 256;
        acc3 += old_lc;
        
        /* Complex nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            if ((i + j) & 1) {
                tmp1 = acc1 + j;
                acc2 += tmp1;
            } else {
                tmp2 = acc2 - j;
                acc1 += tmp2;
            }
        }
    }
    
    /* Force values to be live */
    KEEP(acc1); KEEP(acc2); KEEP(acc3);
    global_acc += acc1 + acc2 + acc3;
}

/* Kernel 2: Arithmetic chains with varying latencies */
static NOOPT void kernel_arithmetic_chains(float* fa, float* fb, int n) {
    float f1, f2, f3, f4, f5;
    int i1, i2, i3, i4;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple parallel dependency chains */
        
        /* Chain 1: Floating point operations */
        f1 = fa[i] * 1.5f;
        f2 = f1 + fb[i];           /* RAW on f1 */
        f3 = f2 / 3.14f;           /* RAW on f2 */
        f4 = f3 - f1;              /* RAW on f3 and f1 */
        
        /* Chain 2: Integer operations with modulo (higher latency) */
        i1 = (int)fa[i];
        i2 = i1 % 13;              /* Higher latency */
        i3 = i2 * i1;              /* RAW on i2 and i1 */
        i4 = i3 / (i2 + 1);        /* RAW on i3 and i2 */
        
        /* Control dependencies */
        if (i4 > 100) {
            f5 = f4 * 2.0f;        /* Control-dependent on i4 */
            fa[i] = f5;            /* Store */
        } else {
            f5 = f4 / 2.0f;
            fb[i] = f5;            /* Store to different array */
        }
        
        /* Output dependencies (WAW) */
        float tmp_result = f5;
        if (i & 3) {
            tmp_result = f4;       /* WAW on tmp_result */
        }
        
        /* Loop-carried anti-dependency (WAR) */
        float prev = fb[(i + n - 1) % n];  /* Read previous iteration's write */
        fb[i] = tmp_result + prev;         /* WAR with previous iteration */
        
        /* Memory barrier to prevent reordering */
        MEM_BARRIER();
    }
    
    /* Use results */
    KEEP(f1); KEEP(i4);
}

/* Kernel 3: Complex control flow with nested loops */
static NOOPT int kernel_control_flow(int* data, int n) {
    int result = 0;
    int a = 0, b = 0, c = 0, d = 0, e = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Outer loop with switch-like control flow */
        switch (i % 4) {
            case 0:
                a = data[i] + i;
                b = a * 2;          /* RAW on a */
                c = b - data[i];    /* RAW on b */
                break;
            case 1:
                a = data[i] ^ 0xFF;
                d = a + c;          /* RAW on a, loop-carried on c */
                e = d % 7;          /* RAW on d, higher latency */
                break;
            case 2:
                /* WAW pattern */
                b = data[i];
                if (e > 10) {
                    b = e * 2;      /* WAW on b */
                }
                c = b + a;          /* RAW on b, loop-carried on a */
                break;
            case 3:
                /* WAR pattern with memory */
                int read_val = data[i];
                data[i] = c + d;    /* WAR on read_val */
                a = read_val * 3;   /* Use read value */
                break;
        }
        
        /* Inner loop with its own dependencies */
        for (int j = 0; j < 2; ++j) {
            int inner_tmp = a + j;
            if (j == 0) {
                b += inner_tmp;     /* RAW on inner_tmp */
            } else {
                c -= inner_tmp;     /* RAW on inner_tmp */
            }
            
            /* Small dependency chain inside inner loop */
            int chain1 = b + 1;
            int chain2 = chain1 * 2;    /* RAW on chain1 */
            d ^= chain2;                /* RAW on chain2 */
        }
        
        /* Loop-carried output dependency */
        static int counter = 0;
        int old_counter = counter;
        counter = (counter + a) & 0xFF;
        e = old_counter;
        
        result += a + b + c + d + e;
    }
    
    return result;
}

/* Simple PRNG to initialize data without library calls */
static int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main(void) {
    int data_int[SIZE];
    float data_float[SIZE];
    int seed = 42;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data_int[i] = simple_rand(&seed) % 1000;
        data_float[i] = (simple_rand(&seed) % 1000) / 10.0f;
    }
    
    /* Call kernels with complex dependency patterns */
    kernel_mem_aliasing(data_int, &data_int[SIZE/2], ITERATIONS);
    
    kernel_arithmetic_chains(data_float, &data_float[SIZE/2], ITERATIONS);
    
    int final_result = kernel_control_flow(data_int, ITERATIONS);
    
    /* Combine results and force output */
    final_result += global_acc;
    KEEP(final_result);
    
    /* Use result to prevent dead code elimination */
    sink = final_result;
    
    return final_result & 0xFF;
}
