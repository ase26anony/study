/* ddg_test.c - Complex dependency patterns to trigger DDG edge creation */
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define KEEP_ALIVE(v) asm volatile("" : : "r"(v) : "memory")

/* Kernel 1: Memory-heavy with pointer aliasing and complex control flow */
NOINLINE static int kernel1(int *arr1, int *arr2, int n) {
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4;
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
    
    /* Multiple interacting pointers that may alias */
    int *p1 = arr1;
    int *p2 = arr2;
    int *p3 = arr1 + (n / 2);
    
    for (int i = 0; i < n; ++i) {
        /* Basic block 1: True dependencies (RAW) */
        tmp1 = *p1 + i;          /* Read from memory */
        tmp2 = tmp1 * 2;         /* RAW: depends on tmp1 */
        tmp3 = tmp2 / 3;         /* RAW: depends on tmp2 (higher latency) */
        
        /* Basic block 2: Anti-dependencies (WAR) with branching */
        if (i & 1) {
            a = tmp3;            /* Use tmp3 */
            *p1 = a + 1;         /* WAR: p1 written after being read earlier */
        } else {
            b = tmp3 % 7;        /* Different computation */
            *p1 = b * 3;         /* WAR with different computation */
        }
        
        /* Basic block 3: Output dependencies (WAW) and memory aliasing */
        tmp4 = *p2;              /* Read from potentially aliased memory */
        if (i % 3 == 0) {
            tmp4 = *p3 + i;      /* WAW: tmp4 reassigned (different source) */
            *p3 = tmp4 - 1;      /* Memory write */
        } else if (i % 3 == 1) {
            tmp4 = tmp4 * 5;     /* WAW: tmp4 reassigned */
            *p2 = tmp4 + 2;      /* Memory write to potentially aliased location */
        } else {
            tmp4 = tmp4 / 2;     /* WAW: tmp4 reassigned again */
        }
        
        /* Basic block 4: Complex arithmetic chain with loop-carried dependency */
        c = d + i;               /* Loop-carried: d from previous iteration */
        d = c * 2 - 1;           /* Loop-carried RAW */
        
        /* Mix computations to create cross-iteration dependencies */
        e = (e + tmp4) % 11;     /* Loop-carried through e */
        f = (f ^ (tmp3 & 0xFF)) + 1; /* Loop-carried through f */
        
        /* Pointer updates with modulo to create aliasing patterns */
        p1 = arr1 + ((i + 1) % (n / 4));
        p2 = arr2 + ((i * 2) % (n / 4));
        p3 = arr1 + ((i * 3) % (n / 2));
        
        /* Keep variables live */
        sum += tmp1 + tmp2 + tmp3 + tmp4 + a + b + c + d + e + f;
    }
    
    KEEP_ALIVE(sum);
    return sum;
}

/* Kernel 2: Arithmetic-heavy with nested loops and varying latencies */
NOINLINE static int kernel2(int *arr, int n) {
    int acc = 0;
    int x = 1, y = 2, z = 3;
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Outer loop with multiple dependency chains */
        
        /* Chain 1: High latency operations (division/modulo) */
        r1 = (arr[i] % 13) + x;
        r2 = r1 / 7;             /* RAW: division has higher latency */
        r3 = r2 % 5;             /* RAW: modulo */
        
        /* Chain 2: Medium latency with branching */
        if (arr[i] > 0) {
            r4 = r3 * 3 + y;     /* RAW from r3 */
            y = r4 & 0xFF;       /* WAR: y written after being read */
        } else {
            r4 = r3 * 5 - y;     /* Different RAW chain */
            y = r4 | 0x7F;       /* WAR with different computation */
        }
        
        /* Chain 3: Low latency arithmetic */
        r5 = z + i;
        z = r5 * 2;              /* WAR: z written */
        
        /* Nested loop to create more basic blocks */
        int inner_sum = 0;
        for (int j = 0; j < 3; ++j) {
            /* Inner loop with its own dependencies */
            int t = r5 + j;
            inner_sum += t * (j + 1);
            
            /* Output dependency in inner loop */
            t = inner_sum - j;   /* WAW: t reassigned */
            
            /* Anti-dependency */
            int u = arr[j % 8];  /* Read */
            arr[j % 8] = u + t;  /* WAR: write to same location */
        }
        
        /* Loop-carried dependencies across outer iterations */
        x = (x * 3 + r4) % 17;   /* x carries to next iteration */
        
        /* Complex expression mixing all chains */
        acc += r1 + r2 + r3 + r4 + r5 + inner_sum + x + y + z;
    }
    
    KEEP_ALIVE(acc);
    return acc;
}

/* Kernel 3: Control-flow heavy with multi-way branching */
NOINLINE static int kernel3(int *arr1, int *arr2, int n) {
    int total = 0;
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int w1 = 0, w2 = 0, w3 = 0, w4 = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Multi-way switch-like branching */
        switch (i % 5) {
            case 0:
                w1 = v1 + arr1[i];
                w2 = w1 * v2;        /* RAW */
                v1 = w2 % 11;        /* WAR: v1 written */
                arr2[i] = w2;        /* Memory write */
                break;
            case 1:
                w1 = v2 - arr1[i];
                w3 = w1 / 3;         /* RAW with division */
                v2 = w3 + 7;         /* WAR: v2 written */
                total += *arr2;      /* Memory read (may alias with arr2[i]) */
                break;
            case 2:
                w1 = v3 * arr1[i];
                w4 = w1 & 0xFF;      /* RAW */
                v3 = w4 ^ 0x55;      /* WAR: v3 written */
                arr1[i] = v3;        /* Memory write */
                break;
            case 3:
                w1 = v4 ^ i;
                w2 = w1 | 0xAA;      /* RAW */
                v4 = w2 << 1;        /* WAR: v4 written */
                w3 = w2;             /* WAW: w3 reassigned from earlier case */
                break;
            default:
                w1 = (v1 + v2 + v3 + v4) / 4;
                w2 = w1 * i;         /* RAW */
                w3 = w2 + 5;         /* RAW */
                w4 = w3 - 2;         /* RAW */
                v1 = v2 = v3 = v4 = w4; /* Multiple WARs */
                break;
        }
        
        /* Loop-carried dependency chain through multiple variables */
        static int persistent = 0;
        persistent = (persistent + w1 + w2 + w3 + w4) % 100;
        
        /* Complex conditional with nested dependencies */
        if (i % 7 == 0) {
            int t1 = v1 + persistent;
            int t2 = t1 * v2;        /* RAW */
            int t3 = t2 / v3;        /* RAW with division */
            v4 = t3 % 9;             /* WAR */
        } else if (i % 7 == 3) {
            int t1 = v2 - persistent;
            int t2 = v3 * t1;        /* RAW */
            int t3 = t2 + v4;        /* RAW */
            v1 = t3 & 0x7F;          /* WAR */
        }
        
        /* Update total with all live variables */
        total += v1 + v2 + v3 + v4 + w1 + w2 + w3 + w4 + persistent;
    }
    
    KEEP_ALIVE(total);
    return total;
}

/* Simple PRNG to initialize data without library calls */
static int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main(void) {
    int data1[SIZE];
    int data2[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = simple_rand(&seed) % 100;
        data2[i] = simple_rand(&seed) % 100;
    }
    
    /* Call kernels with complex dependency patterns */
    int result1 = kernel1(data1, data2, ITERATIONS);
    int result2 = kernel2(data1, ITERATIONS);
    int result3 = kernel3(data1, data2, ITERATIONS);
    
    /* Combine results and force output */
    int final_result = result1 + result2 + result3;
    
    /* Use volatile asm to prevent dead code elimination */
    asm volatile("" : : "r"(final_result));
    
    /* Also use global accumulator */
    global_acc += final_result;
    
    return global_acc != 0 ? 0 : 1;
}
