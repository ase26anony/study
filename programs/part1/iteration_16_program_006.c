/* Selective Scheduling Stress Test for GCC Coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, noinline, optimize("O3")))
static float hot_function(float *data, int n) {
    float sum = 0.0f;
    volatile float temp; /* Prevent optimization */
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write */
        float x = data[i];
        
        /* Complex FP operation chain */
        x = x * x + 1.0f;
        x = sinf(x) * cosf(x);
        
        /* WAR hazard: write after read */
        data[i] = x * 2.0f;
        
        /* WAW hazard: write after write */
        temp = x;
        sum += temp;
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
    }
    return sum;
}

__attribute__((cold, noinline, optimize("sched-pressure")))
static int cold_function(int *arr, int n) {
    int result = 0;
    
    /* Pointer chasing with complex control flow */
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        /* Nested if-else with early exits */
        if (i % 3 == 0) {
            if (ptr[i] > 100) {
                result += ptr[i];
                continue;
            } else {
                result -= ptr[i];
            }
        } else if (i % 3 == 1) {
            /* Conditional move */
            result = (ptr[i] > 50) ? result * 2 : result / 2;
        } else {
            /* Default case with computation */
            result ^= ptr[i];
        }
        
        /* Inline asm with register clobber */
        asm volatile("" : : : "r0", "r1", "r2", "r3");
    }
    return result;
}

__attribute__((optimize("O3")))
static void vectorized_loop(float *a, float *b, float *c, int n) {
    /* SIMD-friendly loop with unrolling directive */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Mixed operations that should vectorize */
        float t1 = a[i] * b[i];
        float t2 = sinf(a[i]) + cosf(b[i]);
        
        /* Cross-iteration dependency */
        c[i] = t1 + t2 + (i > 0 ? c[i-1] * 0.1f : 0.0f);
        
        /* Additional computation to increase pressure */
        a[i] = t1 * 0.5f;
        b[i] = t2 * 2.0f;
    }
}

__attribute__((noinline))
static int switch_pattern(int x) {
    /* Sparse switch cases to challenge scheduler */
    int result = 0;
    switch (x % 13) {
        case 0:  result = x * 2; break;
        case 1:  result = x + x; break;
        case 3:  result = x ^ 0xFF; break;
        case 5:  result = x << 3; break;
        case 7:  result = x >> 2; break;
        case 11: result = ~x; break;
        default: result = x % 7; break;
    }
    
    /* Memory barrier between dependent operations */
    asm volatile("" ::: "memory");
    
    return result * 2;
}

__attribute__((optimize("O3")))
static void nested_loop_scheduling(int *mat, int rows, int cols) {
    /* Complex nested loops with mixed operations */
    for (int i = 0; i < rows; i++) {
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Multiple hazards in tight loop */
            int old = mat[idx];           /* Read */
            mat[idx] = old * 3 + j;       /* Write (WAW potential) */
            int temp = mat[idx] * 2;      /* Read after write (RAW) */
            mat[idx] = temp - old;        /* Write after read (WAR) */
            
            /* Floating point in integer loop */
            if (j % 4 == 0) {
                float fval = (float)mat[idx];
                fval = fval * 1.5f;
                mat[idx] = (int)fval;
            }
        }
        
        /* Scheduling barrier every few iterations */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory", "r4", "r5");
        }
    }
}

/* Main test orchestrator */
int main(void) {
    /* Allocate and initialize test data */
    float *fdata1 = (float*)malloc(SIZE * sizeof(float));
    float *fdata2 = (float*)malloc(SIZE * sizeof(float));
    float *fdata3 = (float*)malloc(SIZE * sizeof(float));
    int *idata = (int*)malloc(SIZE * sizeof(int));
    int *matrix = (int*)malloc(256 * 256 * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        fdata1[i] = (float)rand() / RAND_MAX * 100.0f;
        fdata2[i] = (float)rand() / RAND_MAX * 100.0f;
        idata[i] = rand() % 1000;
    }
    for (int i = 0; i < 256*256; i++) {
        matrix[i] = rand() % 100;
    }
    
    float total_sum = 0.0f;
    int total_int = 0;
    
    /* Execute multiple iterations to ensure coverage */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with FP intensive workload */
        total_sum += hot_function(fdata1, SIZE);
        
        /* Call cold function with integer/control flow */
        total_int += cold_function(idata, SIZE);
        
        /* Vectorized loop with unrolling */
        vectorized_loop(fdata1, fdata2, fdata3, SIZE);
        total_sum += fdata3[SIZE-1];
        
        /* Switch pattern testing */
        for (int i = 0; i < 100; i++) {
            total_int += switch_pattern(idata[i % SIZE]);
        }
        
        /* Nested loop scheduling challenge */
        nested_loop_scheduling(matrix, 256, 256);
        total_int += matrix[0];
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            fdata1[i] += 0.1f;
            idata[i] = (idata[i] + 1) % 1000;
        }
    }
    
    /* Print results to prevent dead code elimination */
    printf("Total sum: %f\n", total_sum);
    printf("Total int: %d\n", total_int);
    printf("Sample values: %f %d\n", fdata1[0], idata[0]);
    
    /* Cleanup */
    free(fdata1);
    free(fdata2);
    free(fdata3);
    free(idata);
    free(matrix);
    
    return 0;
}
