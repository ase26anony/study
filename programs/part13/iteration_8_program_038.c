/* ddg_edge_coverage.c
 * 
 * This program is designed to trigger the creation of ddg_edge structures
 * in GCC's DDG (Data Dependency Graph) builder, specifically covering
 * the edge initialization code in ddg.cc lines 749-757.
 * 
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-ddg -fdump-rtl-sched1 ddg_edge_coverage.c -o ddg_test
 */

#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
static volatile int global_sink = 0;

/* Prevent inlining and IPA optimizations */
static __attribute__((noinline, noipa))
void kernel_memory_aliasing(int* data, int n, int* result) {
    int i;
    int sum = 0;
    int tmp1, tmp2, tmp3, tmp4;
    int* p1;
    int* p2;
    
    /* Complex loop with multiple basic blocks and dependencies */
    for (i = 0; i < n; ++i) {
        /* Multiple basic blocks created by if/else */
        if (i & 1) {
            /* True dependency (RAW) chain */
            tmp1 = data[i] + i;
            tmp2 = tmp1 * 2;           /* Uses tmp1 */
            tmp3 = tmp2 / 3;           /* Uses tmp2 */
            
            /* Anti-dependency (WAR) pattern */
            p1 = &data[i];
            p2 = &data[(i + 1) % SIZE];
            sum += *p1;                /* Read before write */
            *p1 = tmp3;                /* Write after read - WAR */
            
            /* Memory aliasing with potential dependencies */
            if (i & 2) {
                *p2 = sum + *p1;       /* May alias with p1 */
            }
        } else {
            /* Different dependency pattern in else branch */
            tmp1 = data[i] - i;
            tmp2 = tmp1 % 7;           /* Higher latency operation */
            tmp3 = tmp2 * tmp2;
            
            /* Output dependency (WAW) */
            tmp4 = tmp3 + 1;
            tmp4 = tmp3 * 2;           /* WAW on tmp4 */
            
            /* Memory operation with anti-dependency */
            sum += data[i];
            data[i] = tmp4;            /* WAR on data[i] */
        }
        
        /* Nested loop to create more complex control flow */
        int j;
        for (j = 0; j < 3; ++j) {
            /* Register pressure with many live variables */
            int local_var = tmp1 + tmp2 + tmp3 + j;
            sum += local_var;
            
            /* Complex arithmetic chain with varying latency */
            if (j & 1) {
                local_var = local_var / 5;     /* Higher latency */
            } else {
                local_var = local_var + 7;     /* Lower latency */
            }
            
            /* Force value to be used */
            asm volatile("" : : "r"(local_var));
        }
        
        /* Loop-carried dependency with distance > 0 */
        if (i > 0) {
            data[i] += data[i-1];      /* True dependency across iterations */
        }
    }
    
    /* Prevent optimization */
    asm volatile("" : : "r"(sum));
    *result = sum;
}

static __attribute__((noinline, noipa))
void kernel_arithmetic_chains(int* data, int n, int* result) {
    int i;
    int a, b, c, d, e, f, g, h;
    int acc = 0;
    
    /* Heavy arithmetic dependency chains */
    for (i = 0; i < n; ++i) {
        /* Long true dependency chain */
        a = data[i] + 1;
        b = a * 2;              /* RAW on a */
        c = b - 3;              /* RAW on b */
        d = c / 4;              /* RAW on c, higher latency */
        e = d % 5;              /* RAW on d, higher latency */
        f = e * e;              /* RAW on e */
        g = f + a;              /* RAW on f, also uses a from earlier */
        h = g - b;              /* RAW on g, also uses b */
        
        /* Multiple output dependencies (WAW) */
        int tmp = a + b;
        tmp = c + d;            /* WAW on tmp */
        tmp = e + f;            /* WAW on tmp */
        tmp = g + h;            /* WAW on tmp */
        
        /* Anti-dependencies (WAR) with registers */
        int x = tmp;
        tmp = x + i;            /* WAR on tmp */
        x = tmp * 2;            /* WAR on x */
        
        /* Complex conditional with control dependencies */
        if (x > 100) {
            acc += x / 3;
        } else if (x < -100) {
            acc += x * 2;
        } else {
            acc += x;
        }
        
        /* Multiple interacting loops */
        int k;
        for (k = 0; k < 2; ++k) {
            int inner = acc + k;
            acc = inner - (k * 2);
            
            /* More dependencies inside nested loop */
            if (k == 0) {
                data[(i + k) % SIZE] = acc;
            } else {
                acc += data[(i + k) % SIZE];  /* Potential WAR */
            }
        }
    }
    
    /* Use volatile to prevent elimination */
    asm volatile("" : : "r"(acc), "r"(a), "r"(b), "r"(c), "r"(d));
    *result = acc;
}

static __attribute__((noinline, noipa))
void kernel_control_flow(int* data, int n, int* result) {
    int i;
    int r1, r2, r3, r4, r5;
    int total = 0;
    
    /* Complex control flow with many basic blocks */
    for (i = 0; i < n; ++i) {
        /* Initial computation */
        r1 = data[i] ^ i;
        
        /* Multi-way branch creating many basic blocks */
        switch (i % 5) {
            case 0:
                r2 = r1 + 10;
                r3 = r2 * 2;        /* RAW on r2 */
                data[i] = r3;       /* Memory write */
                break;
            case 1:
                r2 = r1 - 5;
                r3 = r2 / 2;        /* RAW on r2, higher latency */
                total += data[i];   /* Memory read - potential WAR with later writes */
                data[i] = r3;       /* Memory write - WAR if i repeats */
                break;
            case 2:
                r2 = r1 * 3;
                r3 = r2 % 7;        /* RAW on r2, higher latency */
                r4 = r3 + r1;       /* RAW on r3, also uses r1 */
                r5 = r4 - r2;       /* RAW on r4, also uses r2 */
                total += r5;
                break;
            case 3:
                /* Output dependencies */
                r2 = r1 + 1;
                r2 = r1 * 2;        /* WAW on r2 */
                r2 = r1 - 3;        /* WAW on r2 */
                total += r2;
                break;
            case 4:
                /* Anti-dependency pattern */
                r2 = data[i];       /* Read */
                data[i] = r1;       /* Write - WAR on data[i] */
                r3 = r2 + r1;       /* Uses both */
                total += r3;
                break;
        }
        
        /* Post-switch computations with loop-carried dependencies */
        if (i > 10) {
            total += data[i-10];    /* Loop-carried true dependency */
        }
        
        /* Another conditional block */
        if (total & 1) {
            int temp = total;
            total = temp * 3;       /* WAW on total */
            data[(i + 7) % SIZE] = temp;  /* Memory write */
        } else {
            total += data[(i + 3) % SIZE];  /* Memory read */
        }
    }
    
    /* Force value to be live */
    asm volatile("" : : "r"(total), "r"(r1), "r"(r2), "r"(r3));
    *result = total;
}

/* Simple PRNG to avoid library dependencies */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main() {
    int data[SIZE];
    int i, seed = 123456;
    int result1, result2, result3;
    
    /* Initialize with pseudo-random data */
    for (i = 0; i < SIZE; ++i) {
        data[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call kernels with complex dependency patterns */
    kernel_memory_aliasing(data, ITERATIONS, &result1);
    kernel_arithmetic_chains(data, ITERATIONS, &result2);
    kernel_control_flow(data, ITERATIONS, &result3);
    
    /* Combine results and use them to prevent dead code elimination */
    int final_result = result1 + result2 + result3;
    
    /* Volatile sink to ensure all computations are kept */
    global_sink = final_result;
    
    /* Also use asm to prevent optimization */
    asm volatile("" : : "r"(final_result), "m"(data[0]));
    
    return final_result & 0xFF;  /* Return non-zero to indicate execution */
}
