/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
static void high_pressure_loop(float *restrict a, float *restrict b, 
                               float *restrict c, int size) {
    int i;
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    for (i = 0; i < size; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * 1.1f;
        t2 = b[i] * 2.2f;
        t3 = c[i] * 3.3f;
        t4 = t1 + t2;
        t5 = t2 - t3;
        t6 = t3 / t1;
        
        /* Group 2: More independent operations */
        t7 = sinf(t1);
        t8 = cosf(t2);
        t9 = t7 * t8;
        t10 = t9 + t4;
        
        /* Group 3: Integer operations mixed with float */
        t11 = (float)((int)t1 ^ (int)t2);
        t12 = t11 * 0.5f;
        t13 = t12 + vol_float1;  /* Volatile dependency */
        
        /* Group 4: Long latency operations */
        t14 = sqrtf(fabsf(t5));
        t15 = logf(1.0f + fabsf(t6));
        
        /* Group 5: More operations to increase pressure */
        t16 = t14 * t15;
        t17 = t16 / (t13 + 0.001f);
        t18 = t17 * 2.71828f;
        t19 = t18 - t10;
        t20 = t19 * 0.99f;
        
        /* Store results creating write pressure */
        a[i] = t4 + t20;
        b[i] = t5 * t19;
        c[i] = t6 + t18;
        
        /* Artificial dependency on volatile */
        if (vol_var1 > 0) {
            a[i] += 0.01f;
        }
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
static void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                             int *restrict arr3, int size) {
    int i;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    for (i = 0; i < size; i++) {
        /* Independent chains with different latencies */
        int val1 = arr1[i] + vol_var1;  /* Volatile read creates delay */
        int val2 = arr2[i] * 3;
        int val3 = arr3[i] / 2;         /* Integer division has latency */
        
        /* Create parallel independent operations */
        int tmp1 = val1 ^ val2;
        int tmp2 = val2 & val3;
        int tmp3 = val1 | val3;
        int tmp4 = tmp1 + tmp2;
        int tmp5 = tmp2 - tmp3;
        int tmp6 = tmp3 * tmp4;
        int tmp7 = tmp4 / (tmp5 + 1);
        int tmp8 = tmp5 ^ tmp6;
        
        /* Mix with floating point to create FU conflicts */
        float f1 = (float)tmp1 * 1.5f;
        float f2 = (float)tmp2 * 2.5f;
        float f3 = f1 / (f2 + 0.001f);  /* FP division has high latency */
        
        /* Accumulate results */
        acc1 += tmp1;
        acc2 += tmp2;
        acc3 += tmp3;
        acc4 += (int)(f3 * 100.0f);
        
        /* More accumulators for pressure */
        acc5 ^= tmp4;
        acc6 |= tmp5;
        acc7 &= tmp6;
        acc8 += tmp7 + tmp8;
        
        /* Memory operations with potential aliasing */
        arr1[i] = acc1;
        arr2[i] = acc2 ^ vol_var2;  /* Another volatile dependency */
        arr3[i] = acc3;
    }
    
    /* Prevent dead code elimination */
    vol_var1 = acc1 + acc2;
    vol_var2 = acc3 + acc4;
}

/* Function with inline assembly to clobber registers */
__attribute__((noinline))
static void register_clobber_ops(int *arr, int size) {
    int i;
    for (i = 0; i < size; i++) {
        int val = arr[i];
        int result;
        
        /* Inline assembly that clobbers multiple registers */
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "imul $0x1234, %%eax, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r" (result)
            : "r" (val)
            : "%eax", "%edx"  /* Clobber eax and edx for pressure */
        );
        
        /* More operations to schedule around asm */
        int tmp1 = result * 2;
        int tmp2 = tmp1 + val;
        int tmp3 = tmp2 ^ result;
        
        arr[i] = tmp3;
        
        /* Additional asm with different constraints */
        __asm__ volatile (
            "add $1, %0\n\t"
            : "+r" (arr[i])
            :
            : "cc"  /* Clobber condition codes */
        );
    }
}

/* Main computational kernel */
__attribute__((noinline))
static void compute_kernel(float *fa, float *fb, float *fc,
                          int *ia, int *ib, int *ic, int size) {
    /* Create control flow for priority variations */
    for (int iter = 0; iter < 10; iter++) {
        if (iter % 3 == 0) {
            high_pressure_loop(fa, fb, fc, size);
        } else if (iter % 3 == 1) {
            mixed_dependency(ia, ib, ic, size);
        } else {
            register_clobber_ops(ia, size);
        }
        
        /* Cross-iteration dependency */
        vol_float1 += 0.1f;
        vol_float2 -= 0.05f;
    }
}

int main(void) {
    /* Allocate and initialize data */
    float *fa = malloc(ARRAY_SIZE * sizeof(float));
    float *fb = malloc(ARRAY_SIZE * sizeof(float));
    float *fc = malloc(ARRAY_SIZE * sizeof(float));
    
    int *ia = malloc(ARRAY_SIZE * sizeof(int));
    int *ib = malloc(ARRAY_SIZE * sizeof(int));
    int *ic = malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX;
        fb[i] = (float)rand() / RAND_MAX;
        fc[i] = (float)rand() / RAND_MAX;
        ia[i] = rand();
        ib[i] = rand();
        ic[i] = rand();
    }
    
    /* Perform computation multiple times */
    for (int outer = 0; outer < ITERATIONS / 1000; outer++) {
        compute_kernel(fa, fb, fc, ia, ib, ic, ARRAY_SIZE);
        
        /* Prevent optimization of entire loop */
        if (outer % 100 == 0) {
            vol_var1 = (vol_var1 * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fsum += fa[i] + fb[i] + fc[i];
        isum += ia[i] ^ ib[i] ^ ic[i];
    }
    
    printf("Checksum: fsum=%f, isum=%d\n", fsum, isum);
    printf("Volatile values: v1=%d, v2=%d, f1=%f, f2=%f\n", 
           vol_var1, vol_var2, vol_float1, vol_float2);
    
    free(fa); free(fb); free(fc);
    free(ia); free(ib); free(ic);
    
    return 0;
}
