/* ddg_edge_trigger.c
 * Program designed to trigger GCC's DDG edge creation logic
 * for uncovered lines in ddg.cc (edge initialization block)
 */

#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000

static volatile int sink;

/* Prevent optimization and inlining */
#define NOOPT __attribute__((noinline, noipa))
#define KEEP(var) asm volatile("" : : "r"(var))
#define MEM_BARRIER() asm volatile("" ::: "memory")

/* Global accumulator to keep values live */
static int global_acc = 0;

/* Kernel 1: Heavy memory aliasing with complex control flow */
NOOPT static int kernel1_memory_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    int tmp1, tmp2, tmp3;
    int *p1, *p2;
    
    /* Create pointer aliasing situation */
    p1 = arr1;
    p2 = arr2;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple basic blocks created by complex if-else chain */
        if (i & 1) {
            /* Block A: Memory RAW and WAR dependencies */
            tmp1 = *p1;                 /* Read from p1 */
            *p1 = tmp1 + i;             /* Write to p1 (WAR) */
            tmp2 = *p2;                 /* Read from p2 */
            
            /* Nested loop for additional complexity */
            for (int j = 0; j < 3; ++j) {
                /* Output dependency (WAW) on tmp3 */
                tmp3 = tmp1 * j;        /* First write to tmp3 */
                tmp3 = tmp2 + tmp3;     /* Second write to tmp3 (WAW) */
                KEEP(tmp3);
            }
            
            /* Memory anti-dependency (WAR) */
            int read_before_write = *p1;
            *p1 = tmp2 * 2;
            sum += read_before_write;
            
        } else {
            /* Block B: Different dependency pattern */
            /* True data dependency chain */
            tmp1 = *p2 + i;
            tmp2 = tmp1 * 3;            /* RAW on tmp1 */
            tmp3 = tmp2 / 2;            /* RAW on tmp2 */
            
            /* Memory output dependency (WAW) through pointers */
            *p2 = tmp3;
            *p2 = *p2 + 1;              /* WAW on *p2 */
            
            /* Complex arithmetic with varying latency ops */
            if (i % 3 == 0) {
                /* High latency operation (%) */
                sum += tmp3 % 7;
            } else {
                /* Lower latency operations */
                sum += tmp1 - tmp2;
            }
        }
        
        /* Inter-block dependencies */
        if (i % 4 == 0) {
            /* Cross-iteration dependency */
            *p1 = sum + *p2;
        }
        
        /* Pointer arithmetic that may alias */
        p1 = &arr1[(i + 1) % SIZE];
        p2 = &arr2[(i * 3) % SIZE];
        
        /* Memory barrier to prevent reordering */
        MEM_BARRIER();
    }
    
    return sum;
}

/* Kernel 2: Arithmetic dependency chains with control flow */
NOOPT static int kernel2_arithmetic_chains(int *arr, int n) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, j = 9, k = 10;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Long true dependency chain (RAW) */
        a = arr[i] + b;
        KEEP(a);
        b = a * c - d;
        KEEP(b);
        c = b / (e + 1);
        KEEP(c);
        d = c % 13 + f;
        KEEP(d);
        e = d * d - g;
        KEEP(e);
        f = e + h;
        KEEP(f);
        g = f ^ j;
        KEEP(g);
        h = g * k;
        KEEP(h);
        j = h >> 2;
        KEEP(j);
        k = j + a;  /* Loop-carried: uses 'a' from this iteration */
        KEEP(k);
        
        /* Anti-dependencies (WAR) */
        int old_a = a;
        a = k + i;
        result += old_a;
        
        /* Output dependencies (WAW) */
        int tmp = b + c;
        tmp = d * e;    /* WAW on tmp */
        tmp = f - g;    /* Another WAW on tmp */
        
        /* Control-dependent computations */
        if (i & 2) {
            /* Branch 1 */
            arr[i % SIZE] = tmp + h;
            result += arr[(i + 1) % SIZE];  /* Potential memory dependency */
        } else if (i & 4) {
            /* Branch 2 */
            arr[i % SIZE] = j - k;
            result *= (arr[i % SIZE] + 1);
        } else {
            /* Branch 3 */
            result ^= tmp;
        }
        
        /* Loop-carried output dependency */
        arr[(i + 2) % SIZE] = result;
    }
    
    /* Force all variables to be live */
    KEEP(a); KEEP(b); KEEP(c); KEEP(d); KEEP(e);
    KEEP(f); KEEP(g); KEEP(h); KEEP(j); KEEP(k);
    
    return result;
}

/* Kernel 3: Mixed dependencies with nested loops */
NOOPT static int kernel3_mixed_nested(int *arr1, int *arr2, int n) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int t1, t2, t3, t4, t5;
    
    for (int i = 0; i < n; ++i) {
        /* Outer loop computations */
        t1 = arr1[i];
        t2 = arr2[i];
        
        /* Nested loop with dependencies */
        for (int j = 0; j < 5; ++j) {
            /* Inner loop carried dependencies */
            t3 = t1 + t2 + j;
            t4 = t3 * t3 - j;
            
            /* Memory WAW in inner loop */
            arr1[(i + j) % SIZE] = t4;
            arr1[(i + j) % SIZE] = t4 + 1;
            
            /* Inner loop anti-dependency */
            int old_t3 = t3;
            t3 = t4 / 2;
            acc1 += old_t3;
        }
        
        /* Cross-iteration memory dependency */
        t5 = arr1[(i + 1) % SIZE] + acc1;
        
        /* Multiple output dependencies */
        acc2 = t5 + i;
        acc2 = acc2 * 2;        /* WAW on acc2 */
        acc2 = acc2 - t2;       /* Another WAW on acc2 */
        
        /* Control flow creating different basic blocks */
        switch (i % 5) {
            case 0:
                acc3 += acc2 % 17;
                arr2[i] = acc3;
                break;
            case 1:
                acc3 ^= acc2;
                arr2[i] = t1 * t2;
                break;
            case 2:
                acc3 = acc2 >> 3;
                arr2[i] = acc3 + 1;
                break;
            case 3:
                acc3 = acc2 * acc2;
                arr2[i] = t5;
                break;
            default:
                acc3 = acc2 - 100;
                arr2[i] = acc3;
                break;
        }
        
        /* Complex pointer arithmetic that may alias */
        int *ptr1 = &arr1[(i * 7) % SIZE];
        int *ptr2 = &arr2[(i * 11) % SIZE];
        
        /* Memory dependencies through potentially aliasing pointers */
        *ptr1 = *ptr2 + acc3;
        acc1 += *ptr1;  /* RAW memory dependency */
        
        /* Another potential WAR */
        int read_ptr2 = *ptr2;
        *ptr2 = acc1;
        acc2 += read_ptr2;
    }
    
    return acc1 + acc2 + acc3;
}

/* Simple PRNG to initialize data without library calls */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main(void) {
    /* Large arrays to work with */
    int data1[SIZE];
    int data2[SIZE];
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; ++i) {
        data1[i] = simple_rand(&seed) % 1000;
        data2[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call kernels with complex dependency patterns */
    int result1 = kernel1_memory_aliasing(data1, data2, ITERATIONS);
    int result2 = kernel2_arithmetic_chains(data1, ITERATIONS);
    int result3 = kernel3_mixed_nested(data1, data2, ITERATIONS);
    
    /* Combine results and use them to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    /* Force the result to be observable */
    sink = final_result;
    
    /* Use inline asm to ensure everything stays */
    asm volatile("" : : "r"(final_result), "m"(data1), "m"(data2));
    
    return final_result & 0xFF;  /* Return non-zero result */
}
