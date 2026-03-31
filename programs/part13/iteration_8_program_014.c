/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory-heavy with pointer aliasing and complex control flow */
NOINLINE static void kernel_memory_aliasing(int* data, int n, int* result) {
    int sum = 0;
    int* p1 = data;
    int* p2 = data + n/2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by complex if-else chain */
        if (i & 1) {
            /* True dependency (RAW) chain */
            int t1 = *p1 * 3;
            int t2 = t1 + i;
            int t3 = t2 / 2;
            
            /* Anti-dependency (WAR) */
            int read_before = *p2;
            *p2 = t3 + read_before;  /* WAR: read_before used before p2 written */
            
            /* Output dependency (WAW) */
            int tmp = t1;
            tmp = t2;  /* WAW: tmp reassigned */
            tmp = t3;  /* WAW: tmp reassigned again */
            
            sum += tmp + read_before;
            
            /* Pointer arithmetic that may alias */
            p1 += (i % 3) - 1;
            p2 += (i % 5) - 2;
        } else {
            /* Different dependency pattern in else branch */
            int a = data[i % SIZE];
            int b = a + *p2;
            
            /* Memory dependency with potential aliasing */
            *p1 = b * 2;
            int c = *p2 + i;  /* MEM_DEP: p2 may alias with p1 */
            
            /* Nested loop for additional basic blocks */
            for (int j = 0; j < 3; ++j) {
                c += j * data[(i + j) % SIZE];
            }
            
            sum += c;
            
            /* More pointer movement */
            p1 = data + ((i * 7) % SIZE);
            p2 = data + ((i * 11) % SIZE);
        }
        
        /* Control-dependent computation */
        if ((i % 7) == 0) {
            int x = sum % 13;
            int y = x * x;
            sum = y - (i & 0xFF);
        }
        
        /* Volatile to prevent elimination */
        KEEP_ALIVE(sum);
        KEEP_ALIVE(p1);
        KEEP_ALIVE(p2);
    }
    
    *result = sum;
    global_acc += sum;
}

/* Kernel 2: Arithmetic-heavy with register dependencies and varying latencies */
NOINLINE static void kernel_arithmetic_chains(int* data, int n, int* result) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple parallel dependency chains */
        
        /* Chain 1: High-latency operations (division/modulo) */
        int a = data[i % SIZE];
        int b = a % 17;        /* Potentially high latency */
        int c = b / 3;         /* Potentially high latency */
        int d = c * 7;
        acc1 += d;
        
        /* Chain 2: Medium-latency operations */
        int e = data[(i + 1) % SIZE];
        int f = e * e;         /* Multiplication */
        int g = f - a;
        int h = g >> 2;
        acc2 += h;
        
        /* Chain 3: Output dependencies (WAW) */
        int tmp = r1;
        tmp = r2 + i;      /* WAW */
        tmp = tmp * r3;    /* WAW */
        acc3 += tmp;
        
        /* Chain 4: Anti-dependencies (WAR) */
        int old_r4 = r4;   /* Read before write */
        r4 = old_r4 * 2 + i;  /* WAR */
        acc4 += r4;
        
        /* Cross-chain dependencies */
        if ((i % 5) == 0) {
            r1 = acc1 % 256;
            r2 = acc2 % 256;
            r3 = acc3 % 256;
        } else {
            r1 = (r1 + 1) % 256;
            r2 = (r2 + 2) % 256;
            r3 = (r3 + 3) % 256;
        }
        
        /* Loop-carried dependency with distance > 0 */
        static int carry = 0;
        int new_carry = carry + acc1;
        acc1 = new_carry - carry;
        carry = new_carry % 1000;
        
        /* Keep all accumulators live */
        KEEP_ALIVE(acc1); KEEP_ALIVE(acc2);
        KEEP_ALIVE(acc3); KEEP_ALIVE(acc4);
        KEEP_ALIVE(r1); KEEP_ALIVE(r2);
        KEEP_ALIVE(r3); KEEP_ALIVE(r4);
    }
    
    *result = acc1 + acc2 + acc3 + acc4;
    global_acc += *result;
}

/* Kernel 3: Complex control flow with nested loops and mixed dependencies */
NOINLINE static void kernel_control_flow(int* data, int n, int* result) {
    int sum = 0;
    int* ptrs[4] = {data, data + 100, data + 200, data + 300};
    
    for (int i = 0; i < n; ++i) {
        /* Switch-like control flow creating multiple basic blocks */
        switch (i % 4) {
            case 0: {
                /* Memory dependencies with aliasing */
                int* p = ptrs[0];
                int val = *p;
                *ptrs[1] = val + i;  /* May alias with p */
                sum += val;
                
                /* Nested loop */
                int inner_sum = 0;
                for (int j = 0; j < 4; ++j) {
                    inner_sum += data[(i + j) % SIZE];
                }
                sum += inner_sum;
                break;
            }
            case 1: {
                /* Register pressure with many live variables */
                int v1 = data[i % SIZE];
                int v2 = v1 * 2;
                int v3 = v2 + ptrs[2][0];
                int v4 = v3 - v1;
                int v5 = v4 % 19;
                int v6 = v5 * v2;
                int v7 = v6 >> 1;
                int v8 = v7 + v3;
                int v9 = v8 - v4;
                int v10 = v9 * v5;
                
                /* Use all variables to keep them live */
                sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
                break;
            }
            case 2: {
                /* True dependencies across iterations */
                static int prev = 0;
                int curr = data[i % SIZE] * 3;
                int diff = curr - prev;  /* Loop-carried dependency */
                sum += diff;
                prev = curr;
                
                /* Pointer chasing */
                ptrs[0] = data + (i % SIZE);
                ptrs[1] = data + ((i * 2) % SIZE);
                break;
            }
            case 3: {
                /* Mixed dependencies */
                int x = *ptrs[3];
                int y = x + sum;      /* RAW */
                *ptrs[3] = y;         /* WAR (x read above) and WAW (ptrs[3] written) */
                int z = *ptrs[3] + i; /* RAW from previous store */
                sum += z;
                break;
            }
        }
        
        /* Rotate pointers */
        int* temp = ptrs[0];
        ptrs[0] = ptrs[1];
        ptrs[1] = ptrs[2];
        ptrs[2] = ptrs[3];
        ptrs[3] = temp;
        
        KEEP_ALIVE(sum);
        for (int k = 0; k < 4; ++k) {
            KEEP_ALIVE(ptrs[k]);
        }
    }
    
    *result = sum;
    global_acc += sum;
}

/* Simple PRNG to initialize data without library calls */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    /* Initialize data with pseudo-random values */
    int data[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (int)(lcg_rand() % 1000);
    }
    
    int res1, res2, res3;
    
    /* Call kernels with different dependency patterns */
    kernel_memory_aliasing(data, ITERS, &res1);
    kernel_arithmetic_chains(data, ITERS, &res2);
    kernel_control_flow(data, ITERS, &res3);
    
    /* Combine results and use volatile sink */
    int final_result = res1 + res2 + res3 + global_acc;
    sink = final_result;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(final_result));
    
    return final_result % 256;
}
