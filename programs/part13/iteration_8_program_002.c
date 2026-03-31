/* ddg_edge_trigger.c
 * Program designed to trigger GCC's DDG edge creation with complex dependencies
 */

#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))
#define NOINLINE __attribute__((noinline, noipa))

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Memory aliasing and pointer arithmetic dependencies */
NOINLINE static void kernel_memory_aliasing(int* data, int n, int* result) {
    int sum = 0;
    int tmp1, tmp2, tmp3;
    
    for (int i = 1; i < n; ++i) {
        /* Create multiple basic blocks with if/else */
        if (i & 1) {
            /* True dependency chain (RAW) */
            tmp1 = data[i] + data[i-1];      /* RAW: data[i] read */
            tmp2 = tmp1 * 3;                 /* RAW: tmp1 -> tmp2 */
            data[i] = tmp2 + i;              /* WAR: data[i] written after read above */
            
            /* Anti-dependency (WAR) */
            int read_val = data[i-1];        /* Read */
            data[i-1] = tmp2;                /* Write to same location - WAR */
            sum += read_val;
            
            /* Output dependency (WAW) */
            tmp3 = tmp1 + tmp2;
            tmp3 = tmp2 * 2;                 /* WAW: tmp3 reassigned */
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = data[i] - data[i-1];
            tmp2 = tmp1 / 2;                 /* Higher latency division */
            if (tmp2 > 0) {
                data[i] = tmp2;              /* WAR */
                tmp3 = data[i] * data[i-1];
            } else {
                data[i] = -tmp2;
                tmp3 = data[i-1] % 7;        /* Higher latency modulo */
            }
            sum += tmp3;
        }
        
        /* Pointer aliasing to create memory dependencies */
        int* p1 = &data[i];
        int* p2 = &data[(i * 7) % n];        /* May alias with p1 */
        
        *p1 = sum + i;                       /* Memory write */
        sum += *p2;                          /* Memory read - potential MEM_DEP */
        
        /* Keep variables live */
        KEEP_ALIVE(tmp1);
        KEEP_ALIVE(tmp2);
        KEEP_ALIVE(tmp3);
    }
    
    *result = sum;
    global_acc += sum;
}

/* Kernel 2: Complex arithmetic chains with control flow */
NOINLINE static void kernel_arithmetic_chains(int* data, int n, int* result) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int acc = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long true dependency chain */
        a = data[i] + b;                     /* RAW: b -> a */
        b = a * c;                           /* RAW: a, c -> b */
        c = b - d;                           /* RAW: b, d -> c */
        d = c % (e + 1);                     /* RAW: c, e -> d (higher latency %) */
        e = d ^ a;                           /* RAW: d, a -> e */
        
        /* Nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            /* Output dependencies within nested loop */
            int tmp = a + j;                 /* First assignment */
            if (j & 1) {
                tmp = b * j;                 /* WAW: tmp reassigned */
                acc += tmp;
            } else {
                tmp = c - j;                 /* WAW: tmp reassigned */
                acc -= tmp;
            }
            
            /* Anti-dependency with array */
            int read_tmp = data[j];          /* Read */
            data[j] = tmp;                   /* Write - WAR */
            acc += read_tmp;
        }
        
        /* Control-dependent computation */
        int control_var;
        if ((i % 7) == 0) {
            control_var = a * b * c;         /* Multi-cycle operation */
        } else if ((i % 7) == 1) {
            control_var = d / (e + 1);       /* Higher latency division */
        } else {
            control_var = a + b + c + d + e;
        }
        
        /* Loop-carried dependency with distance > 0 */
        static int carry = 0;
        int new_carry = control_var + carry; /* RAW with distance=1 */
        carry = new_carry;
        acc += new_carry;
        
        /* Volatile to prevent elimination */
        KEEP_ALIVE(a);
        KEEP_ALIVE(b);
        KEEP_ALIVE(c);
        KEEP_ALIVE(d);
        KEEP_ALIVE(e);
    }
    
    *result = acc;
    global_acc += acc;
}

/* Kernel 3: Mixed dependencies with unpredictable branches */
NOINLINE static void kernel_mixed_branches(int* data, int n, int* result) {
    int x = 0, y = 0, z = 0;
    int total = 0;
    
    for (int i = 2; i < n; ++i) {
        /* Multiple interleaved dependency chains */
        int chain1 = data[i] + x;
        int chain2 = data[i-1] + y;
        int chain3 = data[i-2] + z;
        
        /* Complex conditional with dependencies across branches */
        switch (i % 5) {
            case 0:
                x = chain1 * chain2;         /* RAW: chain1, chain2 -> x */
                y = chain3 - x;              /* RAW: chain3, x -> y */
                data[i] = x + y;             /* WAR: data[i] */
                break;
            case 1:
                z = chain2 / (chain1 + 1);   /* Higher latency, RAW */
                x = z + chain3;
                data[i-1] = x;               /* WAR: data[i-1] */
                break;
            case 2:
                y = chain1 % 11;             /* Higher latency modulo */
                z = y ^ chain2;
                data[i-2] = z;               /* WAR: data[i-2] */
                break;
            case 3:
                /* Output dependencies */
                int tmp = chain1;
                tmp = chain2;                /* WAW */
                tmp = chain3;                /* WAW */
                x = tmp;
                y = tmp + 1;
                z = tmp - 1;
                break;
            default:
                /* All chains together */
                x = chain1 + chain2 + chain3;
                y = x * 2;
                z = y % 13;                  /* Higher latency */
                break;
        }
        
        /* Memory aliasing with pointer arithmetic */
        int* ptr1 = &data[i];
        int* ptr2 = &data[(i * 3 + 1) % n];  /* Potentially aliases */
        
        int val1 = *ptr1;                    /* Read */
        *ptr2 = val1 + x;                    /* Write - may create MEM_DEP */
        total += *ptr1;                      /* Read again - WAR with line 124? */
        
        /* Loop-carried with varying distances */
        static int history[3] = {0, 0, 0};
        int idx = i % 3;
        history[idx] = total + history[(idx + 1) % 3];  /* RAW with distance */
        
        KEEP_ALIVE(x);
        KEEP_ALIVE(y);
        KEEP_ALIVE(z);
        KEEP_ALIVE(chain1);
        KEEP_ALIVE(chain2);
        KEEP_ALIVE(chain3);
    }
    
    *result = total;
    global_acc += total;
}

/* Simple PRNG to initialize data without library calls */
static void init_data(int* data, int n) {
    uint32_t seed = 123456789;
    for (int i = 0; i < n; ++i) {
        seed = seed * 1103515245 + 12345;
        data[i] = (int)(seed % 1000);
    }
}

int main() {
    int data[SIZE];
    int result1, result2, result3;
    
    /* Initialize with pseudo-random data */
    init_data(data, SIZE);
    
    /* Call kernels with complex dependency patterns */
    kernel_memory_aliasing(data, ITERATIONS < SIZE ? ITERATIONS : SIZE, &result1);
    kernel_arithmetic_chains(data, ITERATIONS < SIZE ? ITERATIONS : SIZE, &result2);
    kernel_mixed_branches(data, ITERATIONS < SIZE ? ITERATIONS : SIZE, &result3);
    
    /* Combine results and use them to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + global_acc;
    
    /* Volatile sink to ensure all computations are kept */
    sink = final_result;
    
    /* Return value based on computation to ensure execution */
    return (final_result > 0) ? 0 : 1;
}
