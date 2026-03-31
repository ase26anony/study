/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Global accumulator to keep variables live */
static int global_acc = 0;

/* Prevent optimization */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Kernel 1: Memory aliasing with complex control flow */
__attribute__((noinline, noipa))
static void kernel_memory_aliasing(int* data, int n, int* result) {
    int sum = 0;
    int* p1 = data;
    int* p2 = data + n/2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks start here */
        if (i & 1) {
            /* True dependency (RAW) */
            int temp1 = *p1;
            temp1 = temp1 * 2 + i;  /* WAW on temp1 */
            *p1 = temp1;
            
            /* Anti-dependency (WAR) - read before write to same location */
            int read_before = *p2;
            *p2 = read_before + temp1;  /* WAR: read_before used before p2 written */
            
            /* Complex chain with branching */
            if (temp1 > 1000) {
                int chain1 = temp1 / 3;  /* Higher latency operation */
                int chain2 = chain1 % 7;
                sum += chain2;
                p1++;
            } else {
                int chain1 = temp1 + 7;
                int chain2 = chain1 - 3;
                sum += chain2 * 2;
                p2--;
            }
        } else {
            /* Different dependency pattern in else branch */
            int val = data[i];
            
            /* Output dependency (WAW) */
            int tmp = val * 3;
            if (val & 4) {
                tmp = val / 2;  /* WAW: tmp reassigned */
            }
            
            /* Memory aliasing with potential overlap */
            data[(i + 1) % n] = tmp + data[i];
            sum += tmp;
            
            /* Pointer arithmetic creating aliasing */
            int* p3 = data + (i % (n/4));
            *p3 = *p3 + 1;  /* Could alias with other accesses */
        }
        
        /* Cross-iteration dependency */
        if (i > 0) {
            data[i] += data[i-1];  /* Loop-carried true dependency */
        }
        
        /* Prevent dead code elimination */
        KEEP_ALIVE(sum);
        MEMORY_BARRIER();
    }
    
    *result = sum;
    KEEP_ALIVE(*result);
}

/* Kernel 2: Arithmetic dependency chains with nested loops */
__attribute__((noinline, noipa))
static void kernel_arithmetic_chains(int* data, int n, int* result) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple parallel dependency chains */
        int a = data[i];
        int b = a + i;
        
        /* Chain 1: Integer arithmetic with varying latency */
        int c1 = b * 3;
        int d1 = c1 % 17;      /* Higher latency modulo */
        int e1 = d1 / 2;       /* Higher latency division */
        acc1 += e1;
        
        /* Chain 2: Different operations */
        int c2 = a - i;
        int d2 = c2 & 0xFF;    /* Bitwise ops */
        int e2 = d2 | 0x1;
        acc2 += e2;
        
        /* Chain 3: Mixed operations with conditional */
        int c3 = (a << 2) | (a >> 30);
        int d3;
        if (c3 > 0) {
            d3 = c3 + 5;
        } else {
            d3 = c3 - 5;
        }
        int e3 = d3 * 3;
        acc3 += e3;
        
        /* Nested loop creating inner dependencies */
        for (int j = 0; j < 3; ++j) {
            int inner = e3 + j;
            inner = inner * 2;  /* WAW */
            acc4 += inner;
            
            /* Anti-dependency in inner loop */
            int read_inner = acc4;
            acc4 = read_inner + 1;  /* WAR */
        }
        
        /* Output dependency across chains */
        if (i % 4 == 0) {
            acc1 = acc2 + acc3;  /* WAW on acc1 */
        }
        
        /* Loop-carried dependency */
        static int carry = 0;
        acc2 += carry;
        carry = acc1 % 256;
        
        KEEP_ALIVE(acc1);
        KEEP_ALIVE(acc2);
        KEEP_ALIVE(acc3);
        KEEP_ALIVE(acc4);
    }
    
    *result = acc1 + acc2 + acc3 + acc4;
}

/* Kernel 3: Complex control flow with mixed dependencies */
__attribute__((noinline, noipa))
static void kernel_control_flow(int* data, int n, int* result) {
    int r1 = 0, r2 = 0, r3 = 0;
    int* ptr1 = data;
    int* ptr2 = data + n/3;
    
    for (int i = 0; i < n; ++i) {
        /* Switch-like control flow */
        switch (i % 5) {
            case 0: {
                /* Memory dependencies with potential aliasing */
                int val = *ptr1;
                *ptr2 = val + r1;
                r1 = *ptr1 + 1;  /* RAW through memory */
                ptr1++;
                break;
            }
            case 1: {
                /* Arithmetic chain with output dep */
                int tmp = r2 * 2;
                tmp = tmp + i;      /* WAW on tmp */
                r2 = tmp % 13;
                
                /* Anti-dep through array */
                int read_idx = data[i];
                data[i] = read_idx + tmp;  /* WAR */
                break;
            }
            case 2: {
                /* Nested conditionals */
                if (i & 1) {
                    int x = r3;
                    if (x > 100) {
                        r3 = x / 3;
                    } else {
                        r3 = x * 3;
                    }
                } else {
                    int y = data[i] + data[(i+1)%n];
                    r3 = y - r2;
                }
                break;
            }
            case 3: {
                /* Multiple writes to same location */
                int* p = &data[i % (n/2)];
                *p = r1;
                *p = *p + r2;  /* WAW through pointer */
                r1 = *p;
                break;
            }
            default: {
                /* Complex expression with multiple deps */
                r1 = (r1 + r2) | (r3 & 0xFF);
                r2 = r1 * i;
                r3 = r2 - r1;
                
                /* Loop-carried with distance > 1 */
                if (i >= 2) {
                    data[i] += data[i-2];
                }
                break;
            }
        }
        
        /* Cross-iteration register dependency */
        static int prev = 0;
        int current = r1 + r2 + r3;
        r1 += prev;      /* Loop-carried RAW */
        prev = current;
        
        /* Volatile to prevent optimization */
        sink = r1;
    }
    
    *result = r1 + r2 + r3;
}

/* Simple PRNG to avoid library calls */
static uint32_t lcg = 123456789;
static uint32_t rand_lcg(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

int main(void) {
    /* Initialize data with pseudo-random values */
    int data[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (int)(rand_lcg() % 1000);
    }
    
    int result1, result2, result3;
    
    /* Call kernels with different access patterns */
    kernel_memory_aliasing(data, ITERATIONS, &result1);
    kernel_arithmetic_chains(data + SIZE/4, ITERATIONS/2, &result2);
    kernel_control_flow(data + SIZE/2, ITERATIONS, &result3);
    
    /* Combine results to prevent elimination */
    global_acc = result1 + result2 + result3;
    
    /* Final volatile use */
    KEEP_ALIVE(global_acc);
    asm volatile("" : : "r"(global_acc));
    
    return global_acc & 0xFF;
}
