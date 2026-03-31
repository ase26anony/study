#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with hot attribute and scheduling pressure */
__attribute__((hot, optimize("O3", "sched-pressure"))) 
static float hot_function(float *data, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write to same location */
        float temp = data[i] * 2.0f;
        sum += temp;
        
        /* WAR hazard: write after read */
        data[i] = sum / (i + 1);
        
        /* WAW hazard through pointer aliasing */
        if (i > 0) {
            data[i-1] = data[i] * 0.5f;
        }
    }
    
    /* Memory barrier to split scheduling region */
    asm volatile("" ::: "memory");
    
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static double cold_function(double *arr, int *indices, int n) {
    double result = 0.0;
    
    /* Pointer chasing with varying latencies */
    for (int i = 0; i < n; i++) {
        int idx = indices[i];
        /* Complex addressing mode */
        result += arr[idx * 2] * arr[idx * 2 + 1];
        
        /* Conditional move vs branch */
        result = (idx % 2 == 0) ? result * 1.1 : result * 0.9;
    }
    
    return result;
}

/* Function with explicit unrolling and vectorization hints */
__attribute__((optimize("O3")))
static void vectorized_loop(int *a, int *b, int *c, int n) {
    int i;
    
    /* SIMD-friendly loop with pragma unroll */
    #pragma GCC unroll 4
    for (i = 0; i < n; i++) {
        /* Mixed operations to create instruction diversity */
        a[i] = b[i] * 3 + c[i];
        c[i] = a[i] / 2 - b[i];
        b[i] = (a[i] + c[i]) * 2;
        
        /* Inline assembly with register clobber */
        if (i % 8 == 0) {
            asm volatile("" : : : "r0", "r1", "r2", "r3");
        }
    }
}

/* Complex control flow with switch statement */
__attribute__((noinline))
static int switch_pattern(int x, int y) {
    int result = 0;
    
    /* Sparse switch cases to challenge scheduler */
    switch (x % 13) {
        case 0:
            result = y * 2;
            /* Fall through */
        case 1:
        case 2:
            result += y / 3;
            break;
        case 5:
            result = y << 2;
            /* Multiple early exits */
            if (y > 100) return result;
            /* Fall through */
        case 7:
            result ^= 0xFF;
            break;
        case 10:
            result = ~y;
            /* Another early exit */
            if (result < 0) return -result;
            break;
        default:
            result = y + x;
            /* Complex expression with side effects */
            result = (result > 0) ? result : (result * -1 + x);
    }
    
    /* Nested conditional */
    return (result % 2 == 0) ? result : result * 3 + 1;
}

/* Function with mixed FP and integer hazards */
__attribute__((optimize("O3")))
static double mixed_hazards(float *farr, int *iarr, int n) {
    double acc = 0.0;
    float f_acc = 0.0f;
    
    /* Loop with multiple dependency chains */
    for (int i = 0; i < n; i++) {
        /* FP operation */
        float f_val = farr[i] * 1.5f;
        
        /* Integer operation with dependency on FP result */
        int i_val = (int)(f_val) + iarr[i];
        
        /* WAW hazard on accumulator */
        f_acc = f_val * 0.8f;
        
        /* RAW hazard using f_acc */
        acc += (double)f_acc + i_val;
        
        /* WAR hazard - overwrite input */
        farr[i] = (float)acc;
        
        /* Memory barrier every 16 iterations */
        if (i % 16 == 15) {
            asm volatile("" ::: "memory");
        }
    }
    
    return acc;
}

/* Outer loop with pipelining opportunities */
__attribute__((optimize("O3")))
static void nested_loop_scheduler(int *out, const int *in, int n, int m) {
    /* Nested loops with mixed strides */
    for (int i = 0; i < n; i++) {
        #pragma GCC unroll 2
        for (int j = 0; j < m; j++) {
            /* Complex addressing with multiple uses */
            int idx = i * m + j;
            int val = in[idx];
            
            /* Multiple dependent operations */
            val = val * 3 + 7;
            val = (val >> 2) | (val << 30);  /* Rotation */
            val ^= idx;
            
            out[idx] = val;
            
            /* Conditional continue */
            if (val % 7 == 0) {
                continue;
            }
            
            /* Additional operation for some elements */
            out[idx] += i - j;
        }
        
        /* Early exit condition */
        if (i > n/2 && out[i*m] < 0) {
            break;
        }
    }
}

/* Computed goto pattern for complex control flow */
__attribute__((noinline))
static int computed_goto_pattern(int x) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    if (x < 0 || x > 4) return -1;
    
    goto *labels[x];
    
label0:
    return x * 10;
label1:
    return x * 20 + 5;
label2:
    /* Inline assembly barrier */
    asm volatile("" ::: "memory");
    return x * 30 - 3;
label3:
    return x * 40 ^ 0xAA;
label4:
    return x * 50 | 0x55;
}

/* Main test driver */
int main() {
    /* Allocate and initialize test data */
    float *fdata = (float*)aligned_alloc(16, SIZE * sizeof(float));
    double *ddata = (double*)aligned_alloc(16, SIZE * sizeof(double));
    int *idata1 = (int*)aligned_alloc(16, SIZE * sizeof(int));
    int *idata2 = (int*)aligned_alloc(16, SIZE * sizeof(int));
    int *idata3 = (int*)aligned_alloc(16, SIZE * sizeof(int));
    int *indices = (int*)aligned_alloc(16, SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        fdata[i] = (float)rand() / RAND_MAX * 100.0f;
        ddata[i] = (double)rand() / RAND_MAX * 200.0;
        idata1[i] = rand() % 1000;
        idata2[i] = rand() % 1000;
        idata3[i] = rand() % 1000;
        indices[i] = rand() % (SIZE / 2);
    }
    
    double total_result = 0.0;
    
    /* Execute all test patterns multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with scheduling pressure */
        total_result += hot_function(fdata, SIZE);
        
        /* Call cold function with pointer chasing */
        total_result += cold_function(ddata, indices, SIZE / 4);
        
        /* Vectorized loop with unrolling */
        vectorized_loop(idata1, idata2, idata3, SIZE);
        total_result += idata1[SIZE-1];
        
        /* Complex switch pattern */
        for (int i = 0; i < 100; i++) {
            total_result += switch_pattern(i, idata2[i % SIZE]);
        }
        
        /* Mixed hazards function */
        total_result += mixed_hazards(fdata, idata1, SIZE);
        
        /* Nested loop scheduler */
        nested_loop_scheduler(idata2, idata1, 32, 32);
        total_result += idata2[0];
        
        /* Computed goto */
        for (int i = 0; i < 5; i++) {
            total_result += computed_goto_pattern(i);
        }
        
        /* Memory barrier between iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total_result);
    
    /* Cleanup */
    free(fdata);
    free(ddata);
    free(idata1);
    free(idata2);
    free(idata3);
    free(indices);
    
    return 0;
}
