/* ddg_edge_coverage.c
 * 
 * This program is designed to trigger the creation of ddg_edge structures
 * during GCC's DDG (Data Dependency Graph) construction, specifically
 * covering the edge initialization code in ddg.cc lines 749-757.
 * 
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-ddg -fdump-rtl-sched1 ddg_edge_coverage.c -o ddg_test
 */

#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
static volatile int global_sink = 0;

/* Prevent inlining and inter-procedural analysis */
#define NOINLINE __attribute__((noinline, noipa))

/* Memory-intensive kernel with pointer aliasing */
NOINLINE static int kernel_mem(int* arr, int n) {
    int sum = 0;
    int* p1 = arr;
    int* p2 = arr + (n / 2);
    
    /* Complex loop with multiple dependency types */
    for (int i = 0; i < n; ++i) {
        /* True dependency (RAW) chain */
        int t1 = arr[i];
        int t2 = t1 * 2;           /* RAW on t1 */
        int t3 = t2 + i;           /* RAW on t2 */
        
        /* Anti-dependency (WAR) */
        int old_val = *p1;         /* Read before write */
        *p1 = t3;                  /* WAR: p1 written after read */
        
        /* Output dependency (WAW) */
        int tmp = old_val;         /* First assignment to tmp */
        if (i & 1) {
            tmp = t2;              /* WAW: tmp reassigned in branch */
        } else {
            tmp = t3;              /* WAW: alternative reassignment */
        }
        
        /* Memory aliasing with potential dependencies */
        int val_from_p2 = *p2;     /* May alias with p1 */
        *p2 = tmp + val_from_p2;   /* MEM_DEP possible */
        
        /* Control-dependent computation */
        int ctrl_val;
        if (i % 3 == 0) {
            ctrl_val = val_from_p2 * 3;
        } else if (i % 3 == 1) {
            ctrl_val = val_from_p2 / 2;  /* Higher latency divide */
        } else {
            ctrl_val = val_from_p2 + 7;
        }
        
        /* Another RAW chain with varying latencies */
        int chain1 = ctrl_val % 13;      /* Higher latency modulo */
        int chain2 = chain1 + tmp;
        int chain3 = chain2 * 2;
        
        /* Update pointers with wrap-around */
        p1 = &arr[(i + 1) % n];
        p2 = &arr[(i + n/3) % n];
        
        /* Accumulate results to keep values live */
        sum += chain3 + *p1 + *p2;
    }
    
    /* Force value to be used */
    asm volatile("" : : "r"(sum));
    return sum;
}

/* Arithmetic-intensive kernel with nested loops */
NOINLINE static int kernel_arith(int* arr, int n) {
    int acc = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple parallel dependency chains */
        int a = arr[i];
        int b = a + i;
        int c = b * 2;          /* RAW on b */
        
        /* Nested loop to create more basic blocks */
        int inner_sum = 0;
        for (int j = 0; j < 4; ++j) {
            /* WAR in inner loop */
            int x = a + j;      /* Read a */
            a = x * j;          /* WAR: a written after read */
            
            /* WAW in inner loop */
            int y = b + j;
            if (j % 2) {
                y = c + j;      /* WAW on y */
            }
            inner_sum += y;
        }
        
        /* Complex conditional with dependencies */
        int d;
        if (c > 100) {
            d = c / 3;          /* Higher latency divide */
        } else {
            d = c * 5;
        }
        
        /* Interleaved dependencies across branches */
        int e = d + inner_sum;
        int f;
        if (i % 4 == 0) {
            f = e % 17;         /* Modulo has higher latency */
        } else {
            f = e + 11;
        }
        
        /* Output dependency across iterations (loop-carried) */
        static int persistent = 0;
        int old_persistent = persistent;  /* Read */
        persistent = f + old_persistent;  /* Write - creates loop-carried dep */
        
        /* Multiple simultaneous live variables */
        int g = a + b;
        int h = c + d;
        int k = e + f;
        int m = g + h + k;
        
        acc += m + persistent;
        
        /* Anti-dependency with array */
        arr[i] = acc;           /* WAR: arr[i] was read at start of iteration */
    }
    
    asm volatile("" : : "r"(acc));
    return acc;
}

/* Control-flow intensive kernel */
NOINLINE static int kernel_control(int* arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Base value with RAW */
        int base = arr[i] + i;
        
        /* Multiple conditional blocks creating different basic blocks */
        int branch_val;
        if (i & 1) {
            /* Branch 1: memory operations */
            int* ptr = &arr[(i * 3) % n];
            int mem_val = *ptr;          /* Read */
            *ptr = base + mem_val;       /* Write - creates MEM_DEP */
            branch_val = mem_val * 2;
        } else if (i & 2) {
            /* Branch 2: arithmetic chain */
            int x = base % 7;            /* Modulo */
            int y = x * x;
            int z = y / 2;               /* Divide */
            branch_val = z + i;
        } else {
            /* Branch 3: mixed operations */
            int t1 = base;
            int t2 = t1 + arr[(i + 1) % n];  /* RAW on t1 + memory */
            int t3 = t2 * 3;
            branch_val = t3;
            
            /* WAW within same block */
            t1 = branch_val;             /* WAW on t1 */
            t3 = t1 + 5;                 /* RAW on t1, WAW on t3 */
            branch_val = t3;
        }
        
        /* Loop-carried output dependency */
        static int loop_carried = 0;
        int old_lc = loop_carried;       /* Read from previous iteration */
        loop_carried = branch_val;       /* Write for next iteration */
        
        /* Another layer of conditionals */
        int final;
        switch (i % 5) {
            case 0: final = old_lc + branch_val; break;
            case 1: final = old_lc - branch_val; break;
            case 2: final = old_lc * branch_val; break;
            case 3: final = old_lc / (branch_val ? branch_val : 1); break;
            default: final = old_lc % (branch_val ? branch_val : 1); break;
        }
        
        result += final;
        
        /* Anti-dependency with array update */
        arr[(i + 7) % n] = result;       /* WAR: different index */
    }
    
    /* Use inline asm to prevent optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* Simple PRNG to initialize data without library calls */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    int data[SIZE];
    int seed = 42;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call each kernel to trigger different DDG construction patterns */
    int result1 = kernel_mem(data, ITERATIONS);
    int result2 = kernel_arith(data + SIZE/4, ITERATIONS/2);
    int result3 = kernel_control(data + SIZE/2, ITERATIONS/4);
    
    /* Combine results and force output */
    int total = result1 + result2 + result3;
    global_sink = total;
    
    /* Prevent dead code elimination of entire computation */
    asm volatile("" : : "r"(total));
    
    return total & 0xff;  /* Return non-zero to indicate execution */
}
