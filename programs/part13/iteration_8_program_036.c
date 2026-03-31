/* ddg_test.c - Complex loop-carried dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v) : "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing with complex control flow */
NOINLINE static int kernel1_memory_aliasing(int* arr, int n) {
    int sum = 0;
    int* p1 = arr;
    int* p2 = arr + n/2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks start here */
        if (i & 1) {
            /* True dependency (RAW) with memory */
            int val1 = *p1;
            int val2 = val1 * 2;           /* RAW: val1 -> val2 */
            *p2 = val2;                    /* Memory write */
            
            /* Anti-dependency (WAR) */
            int read_before_write = *p1;   /* Read before write below */
            *p1 = i;                       /* WAR: read_before_write -> *p1 */
            
            /* Output dependency (WAW) */
            int tmp = val2;                /* First write to tmp */
            if (i & 2) {
                tmp = read_before_write;   /* WAW: tmp redefined */
            }
            sum += tmp + read_before_write;
            
            /* Pointer arithmetic that may alias */
            p1 += (i % 3) - 1;
            p2 += (i % 5) - 2;
        } else {
            /* Different path with arithmetic chain */
            int a = i * 3;
            int b = a / 7;                 /* Higher latency division */
            int c = b % 11;                /* Higher latency modulo */
            int d = c + a;                 /* RAW: a,c -> d */
            int e = d - b;                 /* RAW: d,b -> e */
            
            /* WAW on local variable */
            e = e * 2;                     /* WAW: e redefined */
            
            /* Memory dependency with potential aliasing */
            arr[i % 16] = e;
            sum += arr[(i + 1) % 16];      /* MEM_DEP possible */
        }
        
        /* Nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            /* Register pressure with many live variables */
            int x = sum + j;
            int y = x * (i + 1);
            int z = y - (j * 2);
            
            /* Control-dependent computation */
            if (j & 1) {
                z = z / 3;                 /* Higher latency in branch */
            }
            sum = z;                       /* WAW: sum redefined */
        }
        
        /* Prevent dead code elimination */
        KEEP_ALIVE(sum);
    }
    
    return sum;
}

/* Kernel 2: Arithmetic dependency chains with varying latencies */
NOINLINE static int kernel2_arithmetic_chains(int* arr, int n) {
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Four parallel dependency chains */
        
        /* Chain 1: Integer arithmetic */
        int a1 = arr[i];
        int b1 = a1 * 3;                   /* RAW: a1 -> b1 */
        int c1 = b1 / 7;                   /* High latency: b1 -> c1 */
        int d1 = c1 % 13;                  /* High latency: c1 -> d1 */
        acc1 += d1;
        
        /* Chain 2: Mixed operations with branching */
        int a2 = arr[(i + 1) % n];
        int b2;
        if (a2 > 0) {
            b2 = a2 * 2;                   /* RAW in branch */
        } else {
            b2 = -a2;                      /* Different RAW in else */
        }
        int c2 = b2 + i;                   /* RAW: b2 -> c2 */
        int d2 = c2 << 2;                  /* RAW: c2 -> d2 */
        acc2 ^= d2;                        /* WAR: acc2 read, then written */
        
        /* Chain 3: Loop-carried dependency */
        static int persistent = 0;
        int a3 = persistent;               /* Loop-carried RAW */
        int b3 = a3 + arr[i % 16];
        int c3 = b3 * 3;
        persistent = c3;                   /* Loop-carried write */
        acc3 += c3;
        
        /* Chain 4: Complex expression with many temporaries */
        int t1 = i * 5;
        int t2 = t1 + 7;                   /* RAW: t1 -> t2 */
        int t3 = t2 / 3;                   /* RAW: t2 -> t3 */
        int t4 = t3 % 17;                  /* RAW: t3 -> t4 */
        int t5 = t4 - t1;                  /* RAW: t4,t1 -> t5 */
        int t6 = t5 * 2;                   /* RAW: t5 -> t6 */
        acc4 = acc4 + t6;                  /* WAR: acc4 read/write */
        
        /* Cross-chain dependencies */
        if (i % 8 == 0) {
            acc1 += acc2;                  /* MEM_DEP/REG_DEP between chains */
            acc3 ^= acc4;
        }
        
        /* Output dependencies (WAW) */
        if (i % 16 == 0) {
            acc1 = acc3;                   /* WAW: acc1 redefined */
            acc2 = acc4;
        }
        
        /* Keep all accumulators live */
        KEEP_ALIVE(acc1);
        KEEP_ALIVE(acc2);
        KEEP_ALIVE(acc3);
        KEEP_ALIVE(acc4);
    }
    
    return acc1 + acc2 + acc3 + acc4;
}

/* Kernel 3: Control flow intensive with nested loops */
NOINLINE static int kernel3_control_flow(int* arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Outer loop with switch-like control flow */
        switch (i % 4) {
            case 0: {
                /* Memory-intensive case */
                int* ptr1 = &arr[i % 32];
                int* ptr2 = &arr[(i + 16) % 32];
                
                int read1 = *ptr1;         /* Memory read */
                *ptr2 = read1 * 2;         /* Memory write, potential MEM_DEP */
                result += *ptr1;           /* Memory read, WAR with line 7? */
                
                /* Small nested loop */
                for (int k = 0; k < 4; ++k) {
                    int temp = result + k;
                    result = temp - (k * 2); /* WAW: result redefined */
                }
                break;
            }
            case 1: {
                /* Arithmetic chain case */
                int x = i;
                int y = x * x;             /* RAW: x -> y */
                int z = y % 97;            /* High latency: y -> z */
                int w = z + x;             /* RAW: z,x -> w */
                result ^= w;               /* WAR: result read/write */
                break;
            }
            case 2: {
                /* Pointer chasing with aliasing */
                int* a = arr + (i % 8);
                int* b = arr + ((i * 7) % 8);
                
                /* May-alias accesses */
                int val_a = *a;
                *b = val_a + i;            /* Potential MEM_DEP */
                result += *a;              /* Potential MEM_DEP or WAR */
                break;
            }
            case 3: {
                /* Complex expression tree */
                int a = arr[i % 16];
                int b = arr[(i + 1) % 16];
                int c = arr[(i + 2) % 16];
                
                int t1 = a + b;            /* RAW: a,b -> t1 */
                int t2 = b * c;            /* RAW: b,c -> t2 */
                int t3 = t1 - t2;          /* RAW: t1,t2 -> t3 */
                int t4 = t3 / (c + 1);     /* RAW: t3,c -> t4, high latency */
                result = result + t4;      /* WAR: result read/write */
                break;
            }
        }
        
        /* Post-switch computation */
        if (result > 1000) {
            result = result % 1000;        /* WAW: result redefined */
        } else {
            result = result * 3;           /* WAW: alternative redefinition */
        }
        
        /* Another level of nesting */
        for (int j = 0; j < 2; ++j) {
            int mod = (i + j) % 3;
            if (mod == 0) {
                result += arr[j];
            } else if (mod == 1) {
                result -= arr[j];
            } else {
                result ^= arr[j];
            }
        }
        
        KEEP_ALIVE(result);
    }
    
    return result;
}

/* Simple PRNG to initialize data without library calls */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    /* Initialize data with pseudo-random values */
    int data[SIZE];
    int seed = 42;
    
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call all kernels to maximize DDG edge creation opportunities */
    int result1 = kernel1_memory_aliasing(data, ITERATIONS);
    int result2 = kernel2_arithmetic_chains(data, ITERATIONS);
    int result3 = kernel3_control_flow(data, ITERATIONS);
    
    /* Combine results and use them to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    /* Force the result to be observable */
    sink = final_result;
    global_acc += final_result;
    
    /* Use inline asm to ensure result is used */
    asm volatile("" : : "r"(final_result));
    
    return global_acc != 0 ? 0 : 1;
}
