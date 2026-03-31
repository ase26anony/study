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
#define ITERATIONS 1000

/* Global accumulator to prevent dead code elimination */
static volatile int global_sink = 0;

/* Prevent inlining and IPA optimizations */
static __attribute__((noinline, noipa))
void kernel_memory_aliasing(int* restrict data, int n, int* restrict result) {
    int i;
    int sum = 0;
    int tmp1, tmp2, tmp3;
    
    /* Multiple interacting pointers with potential aliasing */
    int* p1 = data;
    int* p2 = data + 1;
    int* p3 = data + 2;
    
    for (i = 0; i < n; ++i) {
        /* Complex memory dependency pattern across basic blocks */
        if (i & 1) {
            /* True dependency (RAW) with memory */
            tmp1 = *p1 + i;
            *p2 = tmp1 * 2;  /* WAR: p2 written after p1 read in next iteration */
            
            /* Anti-dependency (WAR) */
            tmp2 = *p3;
            *p1 = tmp2 - i;  /* WAW on p1 across iterations */
            
            /* Output dependency (WAW) */
            tmp3 = tmp1;
            tmp3 = tmp2 + tmp3;  /* WAW on tmp3 */
            
            /* Memory chain with pointer arithmetic */
            p1 = data + ((i * 3) % (SIZE - 10));
            p2 = data + ((i * 5) % (SIZE - 10));
            p3 = data + ((i * 7) % (SIZE - 10));
        } else {
            /* Different path with register dependencies */
            int r1 = i * 3;
            int r2 = r1 / 2;    /* Higher latency division */
            int r3 = r2 % 17;   /* Modulo operation */
            int r4 = r3 + r1;
            int r5 = r4 - r2;
            
            /* WAW on variables */
            r1 = r5 * 2;
            r1 = r3 + r4;       /* Another WAW */
            
            /* Cross-iteration dependency */
            *p1 = r1 + *p2;     /* Memory WAW */
            tmp1 = *p3;         /* Memory RAW from previous iteration */
            
            /* Force register pressure */
            asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4), "+r"(r5));
        }
        
        /* Loop-carried dependency with distance > 0 */
        if (i > 2) {
            data[i] = data[i-2] + data[i-1];  /* Distance 2 true dependency */
        }
        
        /* Nested loop to create more basic blocks */
        for (int j = 0; j < 3; ++j) {
            /* Anti-dependency in nested loop */
            int anti_tmp = data[j];
            data[j] = (anti_tmp + j) * i;
            
            /* Output dependency */
            int out_tmp = i + j;
            out_tmp = out_tmp * 2;
            out_tmp = out_tmp - j;  /* Multiple WAWs */
        }
        
        /* Volatile asm to prevent elimination */
        asm volatile("" : : "r"(tmp1), "r"(tmp2), "r"(tmp3));
    }
    
    *result = sum;
    global_sink += sum;
}

static __attribute__((noinline, noipa))
void kernel_arithmetic_chains(int* data, int n, int* result) {
    int i;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int chain1, chain2, chain3, chain4, chain5;
    
    for (i = 0; i < n; ++i) {
        /* Long arithmetic dependency chain */
        chain1 = data[i] + i;
        chain2 = chain1 * 3;
        chain3 = chain2 / 2;        /* Higher latency */
        chain4 = chain3 % 13;       /* Modulo - high latency */
        chain5 = chain4 - chain1;
        
        /* Branch with different dependency patterns */
        if (chain5 > 0) {
            /* True dependencies with control flow */
            int t1 = chain5 * 2;
            int t2 = t1 + chain3;
            int t3 = t2 / 7;        /* Another division */
            
            /* Anti-dependency */
            int anti = acc1;
            acc1 = t3 + anti;       /* WAR on acc1 */
            
            /* Output dependencies */
            chain1 = t2;
            chain1 = t3;            /* WAW on chain1 */
        } else {
            /* Alternative path with memory dependencies */
            int idx = i % (SIZE - 5);
            
            /* Memory RAW */
            int mem_val = data[idx];
            data[idx + 1] = mem_val + chain2;
            
            /* Memory WAR */
            int read_before = data[idx + 2];
            data[idx] = read_before * 2;
            
            /* Memory WAW */
            data[idx + 3] = chain4;
            data[idx + 3] = chain5;  /* WAW on memory */
        }
        
        /* Multiple accumulators with loop-carried dependencies */
        acc1 = acc1 + chain1;      /* Distance 1 true dep */
        acc2 = acc2 + chain3;      /* Another distance 1 */
        if (i > 1) {
            acc3 = acc1 + acc2;    /* Distance 2 true dep */
        }
        
        /* Complex condition for control dependencies */
        switch (i % 4) {
            case 0:
                chain1 = acc1 * 2;
                break;
            case 1:
                chain1 = acc2 / 3;
                break;
            case 2:
                chain1 = acc3 % 11;
                break;
            default:
                chain1 = (acc1 + acc2 + acc3) & 0xFF;
                break;
        }
        
        /* Prevent optimization */
        asm volatile("" : "+r"(chain1), "+r"(chain2), "+r"(chain3),
                          "+r"(chain4), "+r"(chain5));
    }
    
    *result = acc1 + acc2 + acc3;
    global_sink += *result;
}

static __attribute__((noinline, noipa))
void kernel_control_flow(int* data, int n, int* result) {
    int i, j;
    int sum = 0;
    int var1, var2, var3, var4, var5, var6, var7, var8;
    
    for (i = 0; i < n; ++i) {
        /* Start with multiple live variables */
        var1 = i;
        var2 = data[i];
        var3 = var1 + var2;
        
        /* Deeply nested control flow */
        if (var3 > 100) {
            var4 = var3 * 2;
            
            if (var4 < 500) {
                var5 = var4 / 3;
                var6 = var5 % 7;
                
                /* Inner loop with dependencies */
                for (j = 0; j < 2; ++j) {
                    int inner = var6 + j;
                    var7 = inner * data[j];
                    var8 = var7 - var5;
                    
                    /* Anti-dependency in inner loop */
                    int anti = var4;
                    var4 = var8 + anti;  /* WAR */
                }
            } else {
                var5 = var4 + 100;
                var6 = var5 - 50;
                
                /* Output dependencies */
                var7 = var6;
                var7 = var5;      /* WAW */
                var7 = var4;      /* Another WAW */
            }
            
            /* True dependency chain */
            var8 = var7 + var6;
            var1 = var8 * var5;
            var2 = var1 / 2;
        } else {
            var4 = var3 - 50;
            
            /* Different arithmetic chain */
            var5 = var4 * 3;
            var6 = var5 + 17;
            var7 = var6 % 19;
            var8 = var7 - var4;
            
            /* Memory operations with aliasing */
            int idx = (i * 7) % (SIZE - 3);
            int tmp = data[idx];
            data[idx + 1] = tmp + var8;  /* RAW then WAR */
            data[idx] = var7;            /* WAW on data[idx] */
        }
        
        /* Loop-carried output dependency */
        static int persistent = 0;
        int old_persistent = persistent;
        persistent = var8 + i;           /* WAW across iterations */
        
        /* Use all variables to keep them live */
        sum += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + old_persistent;
        
        /* Volatile asm to prevent dead code elimination */
        asm volatile("" : : "r"(var1), "r"(var2), "r"(var3), "r"(var4),
                          "r"(var5), "r"(var6), "r"(var7), "r"(var8));
    }
    
    *result = sum;
    global_sink += sum;
}

/* Simple PRNG to avoid library dependencies */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

int main(void) {
    int data[SIZE];
    int result1, result2, result3;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Call each kernel to exercise different DDG patterns */
    kernel_memory_aliasing(data, ITERATIONS, &result1);
    kernel_arithmetic_chains(data, ITERATIONS, &result2);
    kernel_control_flow(data, ITERATIONS, &result3);
    
    /* Combine results and use them to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    /* Volatile sink to ensure all computations are kept */
    asm volatile("" : : "r"(final_result));
    
    return final_result & 0xFF;  /* Return non-zero to indicate execution */
}
