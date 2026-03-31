/* ddg_test.c - Complex loop-carried dependency generator for DDG edge coverage */
#include <stdint.h>

#define SIZE 1024
#define ITERS 1000

/* Global accumulator to prevent dead code elimination */
static volatile int global_sink = 0;

/* Prevent inlining and IPA optimizations */
static __attribute__((noinline,noipa)) 
void kernel_memory_aliasing(int* restrict data1, int* restrict data2, int n) {
    int sum = 0;
    int tmp1, tmp2, tmp3;
    
    /* Complex loop with memory aliasing dependencies */
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency (RAW) chain */
            tmp1 = data1[i] * 3;
            tmp2 = tmp1 + data2[i];  /* Uses tmp1 */
            tmp3 = tmp2 / 2;         /* Uses tmp2 */
            
            /* Anti-dependency (WAR) */
            int read_before_write = data1[i];  /* Read */
            data1[i] = tmp3 + i;               /* Write to same location */
            
            /* Output dependency (WAW) */
            sum = read_before_write + tmp2;    /* First assignment to sum */
            if (tmp3 > 100) {
                sum = tmp1 - tmp3;             /* Second assignment to sum (WAW) */
            }
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = data2[i] - data1[i];
            tmp2 = tmp1 * tmp1;                /* Uses tmp1 */
            
            /* Memory dependency with potential aliasing */
            int* p1 = &data1[i];
            int* p2 = &data2[(i + 1) % n];
            *p1 = tmp2 + i;
            sum += *p2;                        /* MEM_DEP possible */
            
            /* Nested loop for additional complexity */
            for (int j = 0; j < 3; ++j) {
                tmp3 = (tmp2 + j) % 7;         /* Higher latency modulo */
                sum += tmp3;
            }
        }
        
        /* Control-dependent computation */
        if ((i % 3) == 0) {
            /* Another RAW chain with different operations */
            int chain1 = sum * 2;
            int chain2 = chain1 / 3;           /* Integer division has latency */
            int chain3 = chain2 + data1[i];
            sum = chain3;
        } else if ((i % 3) == 1) {
            /* Different latency operations */
            sum = (sum * 5) / 2;               /* Mix of multiply and divide */
        }
        
        /* Force value to be live across iteration */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    /* Sink result */
    global_sink += sum;
}

static __attribute__((noinline,noipa))
void kernel_arithmetic_chains(int* data, int n) {
    int a, b, c, d, e, f, g, h;
    int acc = 0;
    
    /* Loop with long arithmetic dependency chains */
    for (int i = 1; i < n - 1; ++i) {
        /* Long RAW chain spanning multiple statements */
        a = data[i-1] + 1;
        b = a * data[i];           /* Uses a */
        c = b / 3;                 /* Uses b - division has latency */
        d = c - a;                 /* Uses c and a */
        e = d * d;                 /* Uses d */
        f = e % 17;                /* Uses e - modulo has latency */
        g = f + b;                 /* Uses f and b */
        h = g << 2;                /* Uses g */
        
        /* Interleaved WAR dependencies */
        int old_a = a;             /* Read a */
        a = h + i;                 /* Write a - WAR with above read */
        
        int old_b = b;             /* Read b */
        b = old_a * 2;             /* Write b - WAR */
        
        /* WAW on multiple variables */
        if (h > 1000) {
            c = old_b + 5;         /* Second write to c (WAW) */
            d = c * 2;             /* Second write to d (WAW) */
        } else {
            c = old_b - 5;
            d = c / 2;
        }
        
        /* Complex conditional with dependencies crossing basic blocks */
        switch (i % 4) {
            case 0:
                e = a + b + c;
                acc += e * 2;
                break;
            case 1:
                e = b - c;
                acc += e / 2;
                break;
            case 2:
                e = c * d;
                acc += e % 13;
                break;
            case 3:
                e = d << 1;
                acc += e + a;
                break;
        }
        
        /* Loop-carried dependency */
        data[i] = acc + data[i-1];  /* RAW on data[i-1] from previous iteration */
        
        /* Anti-dependency on next iteration's read */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), 
                          "+r"(e), "+r"(f), "+r"(g), "+r"(h) : : "memory");
    }
    
    global_sink += acc;
}

static __attribute__((noinline,noipa))
void kernel_control_flow(int* data, int n) {
    int x = 0, y = 0, z = 0;
    int result = 0;
    
    /* Loop with complex control flow creating many basic blocks */
    for (int i = 0; i < n; ++i) {
        /* Deeply nested conditionals */
        if (data[i] > 0) {
            if ((i % 2) == 0) {
                x = data[i] * 2;
                y = x + i;          /* RAW on x */
                
                /* Inner loop with dependencies */
                for (int j = 0; j < 2; ++j) {
                    z = y + j;      /* RAW on y, loop-carried on z for inner loop */
                    result += z;
                }
            } else {
                x = data[i] / 3;    /* Division latency */
                y = x - i;
                
                /* Different dependency pattern */
                int tmp = y;
                y = x * 2;          /* WAR on y */
                result += tmp;      /* Use old y */
            }
        } else {
            if ((i % 3) == 0) {
                x = -data[i];
                y = x << 1;
                z = y | 0xFF;
            } else {
                x = data[i] + 100;
                y = x % 7;          /* Modulo latency */
                z = y * y;
            }
            
            /* Cross-iteration dependency */
            result = result + z;    /* Loop-carried on result */
        }
        
        /* More conditionals */
        switch (i % 5) {
            case 0:
                data[i] = x + y;
                break;
            case 1:
                data[i] = y - z;
                break;
            case 2:
                data[i] = x * z;
                break;
            case 3:
                data[i] = (x + y + z) / 3;
                break;
            case 4:
                data[i] = (x ^ y) & z;
                break;
        }
        
        /* Keep variables live */
        asm volatile("" : "+r"(x), "+r"(y), "+r"(z) : : "memory");
    }
    
    global_sink += result;
}

/* Simple PRNG to avoid library calls */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    int data1[SIZE];
    int data2[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = simple_rand(&seed) % 1000;
        data2[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call kernels with complex dependency patterns */
    kernel_memory_aliasing(data1, data2, ITERS);
    kernel_arithmetic_chains(data1, ITERS);
    kernel_control_flow(data2, ITERS);
    
    /* Use result to prevent elimination */
    asm volatile("" : : "r"(global_sink) : "memory");
    
    return global_sink & 0xFF;
}
