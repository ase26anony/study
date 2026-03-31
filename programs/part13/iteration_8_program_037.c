/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v) : "memory")

/* Kernel 1: Memory-heavy with pointer aliasing and complex control flow */
static NOOPT void kernel_memory(int* data, int n, int* result) {
    int sum = 0;
    int* p1 = data;
    int* p2 = data + n/2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by complex if-else chain */
        if (i & 1) {
            /* True dependency (RAW) chain */
            int t1 = *p1 + i;
            int t2 = t1 * 2;
            int t3 = t2 - *p2;
            
            /* Anti-dependency (WAR) */
            int read_before_write = *p1;
            *p1 = t3 + read_before_write;
            
            /* Output dependency (WAW) */
            int tmp = t2;
            if (i & 2) {
                tmp = t3 * 3;  /* WAW when i&2 is true */
            }
            sum += tmp;
            
            /* Pointer movement with aliasing possibility */
            p1 = (i % 3 == 0) ? data + (i % 16) : p1 + 1;
        } else {
            /* Different dependency pattern in else branch */
            int a = *p2;
            int b = a / 7;  /* Higher latency operation */
            int c = b % 5;
            
            /* Memory dependency with potential aliasing */
            *p2 = c + i;
            sum += *p1;  /* MEM_DEP: p1 and p2 may alias */
            
            /* Nested loop for additional complexity */
            for (int j = 0; j < 3; ++j) {
                sum += j * (*p1);
            }
            
            p2 = data + ((i * 7) % n);
        }
        
        /* Control-dependent computation */
        int control_var = (i % 8 == 0) ? sum : i;
        if (control_var > 100) {
            /* Another basic block with its own dependencies */
            int x = *data;
            int y = x * x;
            *data = y % 256;  /* Output dependency on *data */
            sum += y;
        }
        
        /* Prevent dead code elimination */
        KEEP_ALIVE(sum);
        KEEP_ALIVE(p1);
        KEEP_ALIVE(p2);
    }
    
    *result = sum;
    global_acc += sum;
}

/* Kernel 2: Arithmetic-heavy with register dependencies and varying latencies */
static NOOPT void kernel_arithmetic(int* data, int n, int* result) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    
    for (int i = 0; i < n; ++i) {
        /* Long dependency chain with varying operation latencies */
        r1 = r1 + data[i];           /* Low latency */
        r2 = r2 * r1;                /* Medium latency */
        r3 = r3 % (r2 + 1);          /* High latency (modulo) */
        r4 = r4 / (r3 | 1);          /* Very high latency (division) */
        
        /* Cross-iteration dependencies (loop-carried) */
        acc1 = acc1 + r1;            /* Distance 1 */
        acc2 = acc2 * r2 + i;        /* Distance 1 with induction var */
        acc3 = acc3 ^ r3;            /* Distance 1 */
        acc4 = (acc4 << 1) | (r4 & 1); /* Distance 1 */
        
        /* Complex conditional with output dependencies */
        int tmp = acc1;
        if (i % 4 == 0) {
            tmp = acc2;  /* WAW on tmp */
            int tmp2 = tmp * 2;
            tmp = tmp2 + acc3;  /* Another WAW */
        } else if (i % 4 == 1) {
            tmp = acc3 * acc4;
        } else {
            tmp = tmp + 1;  /* Uses previous tmp value (RAW) */
        }
        
        /* Anti-dependency pattern */
        int old_acc1 = acc1;
        acc1 = tmp + old_acc1;  /* WAR: old_acc1 read before acc1 written */
        
        /* Memory operation to potentially create MEM_DEP edges */
        data[(i * 13) % n] = acc2;
        int mem_read = data[(i * 17) % n];  /* Potential aliasing */
        acc3 += mem_read;
        
        /* Keep all values live */
        KEEP_ALIVE(r1); KEEP_ALIVE(r2); KEEP_ALIVE(r3); KEEP_ALIVE(r4);
        KEEP_ALIVE(acc1); KEEP_ALIVE(acc2); KEEP_ALIVE(acc3); KEEP_ALIVE(acc4);
    }
    
    /* Combine results with non-trivial computation */
    *result = acc1 + acc2 * 3 + acc3 / 2 + acc4;
    global_acc += *result;
}

/* Kernel 3: Mixed dependencies with nested loops and function calls */
static NOOPT void kernel_mixed(float* fdata, int n, int* result) {
    float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
    int i1 = 0, i2 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Floating point dependency chain */
        f1 = f1 * 1.01f + fdata[i];
        f2 = f2 / 1.02f - f1;
        f3 = f3 + f1 * f2;
        
        /* Integer dependency chain interacting with float chain */
        i1 = (int)f1 + i;
        i2 = i1 * i2 + (int)f2;
        
        /* Nested loop with carried dependencies */
        int inner_sum = 0;
        for (int j = 0; j < 4; ++j) {
            inner_sum += i1 * j + i2;
            /* Output dependency in inner loop */
            int tmp_var = inner_sum;
            tmp_var = tmp_var + j;  /* WAW */
            inner_sum = tmp_var;
        }
        
        /* Complex conditional with multiple basic blocks */
        if (i1 > 1000) {
            fdata[i % n] = f3;
            i2 = i2 + (int)(fdata[(i + 1) % n] * 10);  /* MEM_DEP with distance */
        } else if (i1 > 500) {
            int t = i1;
            i1 = i2;  /* Swap creating WAR */
            i2 = t;
            f1 = f2 + f3;  /* RAW on f2, f3 */
        } else {
            /* Default case with its own dependency pattern */
            f3 = f1 * 2.0f;  /* WAW on f3 */
            i1 = i1 % 17;    /* High latency modulo */
        }
        
        /* Store to memory with potential aliasing */
        if (i % 8 == 0) {
            fdata[(i * 3) % n] = f1;
        }
        
        /* Read from potentially aliased location */
        float mem_val = fdata[(i * 5) % n];
        f2 = f2 + mem_val;
        
        KEEP_ALIVE(f1); KEEP_ALIVE(f2); KEEP_ALIVE(f3);
        KEEP_ALIVE(i1); KEEP_ALIVE(i2);
    }
    
    *result = (int)f1 + i1 * 2 + (int)f3;
    global_acc += *result;
}

/* Simple PRNG to initialize data without library calls */
static uint32_t lcg = 123456789;
static uint32_t rand_lcg(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main(void) {
    /* Initialize data with pseudo-random values */
    int int_data[SIZE];
    float float_data[SIZE];
    
    for (int i = 0; i < SIZE; ++i) {
        int_data[i] = (int)(rand_lcg() % 1000);
        float_data[i] = (float)(rand_lcg() % 1000) / 100.0f;
    }
    
    int result1, result2, result3;
    
    /* Call kernels with different dependency patterns */
    kernel_memory(int_data, ITERATIONS, &result1);
    kernel_arithmetic(int_data + SIZE/4, ITERATIONS, &result2);
    kernel_mixed(float_data, ITERATIONS, &result3);
    
    /* Combine results in a non-trivial way to prevent optimization */
    int final_result = result1 ^ result2 + result3 * 7;
    final_result += global_acc;
    
    /* Volatile sink to prevent dead code elimination */
    sink = final_result;
    
    /* Use result in asm to ensure it's computed */
    asm volatile("" : : "r"(final_result));
    
    return final_result & 255;  /* Return non-zero to indicate execution */
}
