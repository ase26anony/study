/* ddg_edge_trigger.c
 * Program designed to trigger DDG edge creation with complex dependencies
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-ddg -fdump-rtl-sched1 ddg_edge_trigger.c -o ddg_test
 */

#include <stdint.h>

#define N 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent inlining and IPA optimizations */
static void __attribute__((noinline,noipa)) 
kernel_memory_heavy(int* restrict data1, int* restrict data2, int count) {
    int i;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    
    for (i = 0; i < count; ++i) {
        /* Multiple basic blocks created by if-else chain */
        if (i & 1) {
            /* True dependency chain with memory operations */
            tmp1 = data1[i] + data2[i];          /* RAW: tmp1 depends on data1/data2 */
            tmp2 = tmp1 * 3;                     /* RAW: tmp2 depends on tmp1 */
            data1[i] = tmp2;                     /* WAR: data1[i] read earlier in next iteration? */
            
            /* Anti-dependency pattern */
            tmp3 = data2[i];                     /* WAR: tmp3 reads data2[i] */
            data2[i] = tmp2 + i;                 /* WAR: then writes data2[i] */
            
            /* Output dependency */
            tmp4 = tmp3;                         /* WAW: tmp4 assigned here... */
            if (tmp4 > 100) {
                tmp4 = tmp2;                     /* WAW: ...and potentially reassigned here */
            }
            acc1 += tmp4;
        } else {
            /* Different dependency pattern in else branch */
            tmp5 = data1[i] - data2[i];
            tmp6 = tmp5 / 2;                     /* Higher latency division */
            
            /* Memory aliasing pattern - pointers may alias */
            int* p1 = &data1[i];
            int* p2 = &data2[(i + 1) % count];
            *p1 = tmp6;
            tmp7 = *p2;                          /* MEM_DEP: possible true dependency */
            
            /* Nested loop for additional complexity */
            for (int j = 0; j < 3; ++j) {
                tmp7 += j;
            }
            
            acc2 += tmp7;
        }
        
        /* Cross-iteration dependencies */
        if (i > 0) {
            /* Loop-carried true dependency */
            tmp8 = data1[i-1] + acc3;            /* Distance = 1 */
            acc3 = tmp8;
            
            /* Loop-carried anti-dependency */
            int old_val = data2[i];              /* Read */
            data2[i-1] = old_val + i;            /* Write to previous iteration's location */
        }
        
        /* Mix results to keep everything live */
        acc4 = acc1 + acc2 + acc3;
        
        /* Prevent dead code elimination */
        __asm__ volatile("" : "+r"(acc1), "+r"(acc2), "+r"(acc3), "+r"(acc4));
    }
    
    /* Volatile sink to prevent optimization */
    sink = acc1 + acc2 + acc3 + acc4;
}

static void __attribute__((noinline,noipa))
kernel_arithmetic_chains(int* data, int count) {
    int i;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int r1, r2, r3, r4, r5, r6, r7, r8;
    
    for (i = 0; i < count; ++i) {
        /* Long arithmetic dependency chain */
        r1 = data[i] + a;
        r2 = r1 * b;                    /* RAW on r1 */
        r3 = r2 - c;                    /* RAW on r2 */
        r4 = r3 / (d + 1);              /* RAW on r3, higher latency */
        r5 = r4 % (e + 1);              /* RAW on r4, even higher latency */
        
        /* Parallel chain with cross dependencies */
        r6 = data[count - i - 1] + f;
        r7 = r6 + r2;                   /* RAW on r6, also RAW on r2 from other chain */
        r8 = r7 * r5;                   /* RAW on r7 and r5 - joins both chains */
        
        /* Control-dependent assignments creating output dependencies */
        if (r8 > 1000) {
            a = r8;                     /* WAW: a reassigned */
            b = a + 1;
        } else if (r8 < 100) {
            a = r8 / 2;                 /* WAW: a reassigned differently */
            b = a - 1;
        } else {
            /* Anti-dependency pattern */
            int temp = b;               /* WAR: read b */
            b = r8;                     /* WAR: then write b */
            a = temp;                   /* Use the old value */
        }
        
        /* Complex condition with side effects */
        switch (i % 4) {
            case 0:
                c = r1 + r6;
                data[i] = r8;           /* Memory write */
                break;
            case 1:
                c = r2 - r7;
                data[(i + 1) % count] = r8;  /* Different memory location */
                break;
            case 2:
                /* Create memory dependency with potential aliasing */
                int* ptr1 = &data[i];
                int* ptr2 = &data[(i + 3) % count];
                *ptr1 = r3;
                c = *ptr2 + r4;         /* Possible MEM_DEP */
                break;
            default:
                c = r5;
                /* Output dependency in memory */
                data[i] = r6;           /* WAW: data[i] might be written multiple times */
                data[i] = r7;           /* WAW: overwrite */
                break;
        }
        
        /* Keep variables live across iterations */
        d = c + i;
        e = d * 2;
        f = e - r8;
        g = f / (data[i] + 1);
        h = g % 256;
        
        /* Force all values to be used */
        __asm__ volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d),
                                 "+r"(e), "+r"(f), "+r"(g), "+r"(h));
    }
    
    /* Combine results */
    int result = a + b + c + d + e + f + g + h;
    sink = result;
}

static void __attribute__((noinline,noipa))
kernel_control_flow(int* data1, int* data2, int count) {
    int i, j;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    
    for (i = 0; i < count; ++i) {
        /* Deeply nested control flow */
        t1 = data1[i];
        
        if (t1 > 500) {
            t2 = t1 * 2;
            if (i % 3 == 0) {
                t3 = t2 + data2[i];
                for (j = 0; j < 2; ++j) {
                    t3 += j * data1[(i + j) % count];
                }
                t4 = t3 / 3;
            } else if (i % 3 == 1) {
                t3 = t2 - data2[i];
                t4 = t3 * 2;
            } else {
                t3 = t2;
                t4 = t3 % 17;
            }
            t5 = t4 + i;
        } else if (t1 > 250) {
            t2 = t1 + 100;
            t3 = t2 * t2;
            t4 = t3 - data2[i];
            t5 = t4 / 2;
        } else {
            t2 = t1;
            t3 = t2;
            
            /* Loop with carried dependency */
            for (j = 0; j < 4; ++j) {
                t3 += data1[(i + j) % count];
                data1[(i + j) % count] = t3 + j;  /* WAR in inner loop */
            }
            t4 = t3;
            t5 = t4;
        }
        
        /* More branches creating different dependency distances */
        switch (i % 5) {
            case 0:
                t6 = t5 + sum1;
                sum1 = t6;                      /* Loop-carried: distance 1 */
                break;
            case 1:
                t6 = t5 - sum2;
                sum2 = t6;                      /* Loop-carried: distance 1 */
                break;
            case 2:
                t6 = t5 * 2;
                /* Anti-dependency across iterations */
                int old = data2[i];             /* Read */
                data2[i] = t6;                  /* Write - WAR for next iteration? */
                t7 = old;
                break;
            case 3:
                t6 = t5 / 3;
                /* Output dependency */
                t7 = t6;                        /* Assignment */
                t7 = t6 + data1[i];             /* Reassignment - WAW */
                break;
            default:
                t6 = t5;
                /* Memory dependency chain */
                t7 = data1[i] + data2[i];
                data1[(i + 1) % count] = t7;    /* Store with distance 1 */
                t8 = data1[i];                  /* Load - creates flow dep */
                t6 = t8;
                break;
        }
        
        /* Final accumulation with complex expression */
        t9 = t6 + (i % 10);
        t10 = t9 * t9;
        sum3 += t10;
        
        /* Mix values to create cross-dependencies */
        data1[i] = (data1[i] + t10) % 1000;
        data2[i] = (data2[i] + sum3) % 1000;
        
        /* Prevent optimization */
        __asm__ volatile("" : "+r"(sum1), "+r"(sum2), "+r"(sum3));
    }
    
    sink = sum1 + sum2 + sum3;
}

/* Simple PRNG to avoid library dependencies */
static uint32_t lcg_parkmiller(uint32_t* state) {
    return *state = (uint64_t)*state * 48271 % 0x7fffffff;
}

int main(void) {
    int data1[N], data2[N];
    uint32_t seed = 123456789;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; ++i) {
        data1[i] = lcg_parkmiller(&seed) % 1000;
        data2[i] = lcg_parkmiller(&seed) % 1000;
    }
    
    /* Call kernels with different dependency patterns */
    kernel_memory_heavy(data1, data2, ITERATIONS);
    kernel_arithmetic_chains(data1, ITERATIONS);
    kernel_control_flow(data1, data2, ITERATIONS);
    
    /* Combine results to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < N; ++i) {
        total += data1[i] + data2[i];
    }
    
    /* Volatile asm to ensure computation isn't optimized away */
    __asm__ volatile("" : : "r"(total));
    
    return total % 256;
}
