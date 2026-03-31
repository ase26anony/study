#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, noinline, optimize("O3")))
static float hot_loop_with_hazards(float* restrict a, float* restrict b, 
                                   float* restrict c, int size) {
    float sum = 0.0f;
    
    /* Mixed RAW, WAR, WAW hazards */
    #pragma GCC unroll 4
    for (int i = 0; i < size; i++) {
        float t1 = a[i] * b[i];      /* RAW: t1 depends on a[i], b[i] */
        float t2 = t1 + c[i];        /* RAW: t2 depends on t1 */
        a[i] = t2 * 0.5f;            /* WAR: a[i] read earlier, now written */
        float t3 = a[i] * t1;        /* RAW: t3 depends on a[i] and t1 */
        b[i] = t3 + t2;              /* WAR: b[i] read earlier, now written */
        sum += b[i];                 /* RAW: sum depends on b[i] */
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
    }
    return sum;
}

__attribute__((cold, noinline, optimize("sched-pressure")))
static int cold_complex_control_flow(int* restrict arr, int size) {
    int result = 0;
    
    /* Complex control flow with multiple exit points */
    for (int i = 0; i < size; i++) {
        if (i % 3 == 0) {
            /* Pointer chasing pattern */
            int* ptr = &arr[i];
            int val = *ptr;
            
            /* Mixed integer operations */
            val = (val * 13 + 7) & 0xFF;
            
            /* Conditional move vs branch */
            result += (val > 128) ? (val >> 1) : (val << 1);
            
            if (result > 1000000) {
                /* Early exit */
                break;
            }
        } else if (i % 5 == 0) {
            /* Different computation path */
            result ^= arr[i];
            continue;  /* Skip rest of loop iteration */
        } else {
            /* Default computation */
            result += arr[i] * 2;
        }
        
        /* Another scheduling barrier */
        asm volatile("" ::: "eax", "ebx", "memory");
    }
    return result;
}

__attribute__((optimize("O3"), noinline))
static double vectorized_mixed_ops(double* restrict x, double* restrict y, 
                                   int* restrict indices, int n) {
    double acc = 0.0;
    
    /* SIMD-friendly loop with mixed operations */
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        /* Integer index calculation */
        int idx = indices[i] & (n - 1);
        
        /* Floating point computation */
        double temp = x[idx] * y[i];
        
        /* Trigonometric operation */
        temp = sin(temp) * cos(x[i]);
        
        /* Power operation */
        temp = pow(temp, 1.5);
        
        /* Conditional update */
        acc += (temp > 0) ? temp : -temp;
        
        /* Memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    return acc;
}

__attribute__((noinline))
static int switch_based_computation(int x) {
    int result;
    
    /* Sparse switch statement */
    switch (x % 17) {
        case 0:
            result = x * 2;
            asm volatile("" ::: "ecx");  /* Register clobber */
            break;
        case 1:
        case 3:
            result = x + (x >> 3);
            /* Fall through */
        case 5:
            result ^= 0x55AA;
            break;
        case 7:
            result = x * x;
            asm volatile("" ::: "edx", "memory");
            break;
        case 11:
            result = ~x;
            break;
        default:
            result = x % 13;
            /* Multiple barriers */
            asm volatile("" ::: "memory");
            asm volatile("" ::: "memory");
            break;
    }
    return result;
}

__attribute__((optimize("O3")))
static void nested_loop_scheduling(int size) {
    /* Triple nested loop with dependencies */
    float matrix[32][32];
    float vector[32];
    float result[32] = {0};
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        vector[i] = i * 0.1f;
        for (int j = 0; j < 32; j++) {
            matrix[i][j] = (i + j) * 0.01f;
        }
    }
    
    /* Matrix-vector multiplication with scheduling challenges */
    for (int iter = 0; iter < size; iter++) {
        for (int i = 0; i < 32; i++) {
            float sum = 0.0f;
            for (int j = 0; j < 32; j++) {
                /* Interleaved load/store with computation */
                float a = matrix[i][j];
                float b = vector[j];
                float prod = a * b;
                
                /* Dependency chain */
                sum += prod;
                
                /* Modify matrix for next iteration */
                matrix[i][j] += 0.001f;
                
                /* Barrier on inner loop */
                if ((j & 3) == 0) {
                    asm volatile("" ::: "memory");
                }
            }
            result[i] = sum;
            
            /* Modify vector with data dependency */
            vector[i] = result[i] * 0.9f + vector[i] * 0.1f;
        }
    }
}

/* Main test driver */
int main(void) {
    /* Allocate and initialize arrays */
    float* fa = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fb = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fc = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    double* dx = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double* dy = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    int* indices = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    int* int_arr = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (i % 100) * 0.1f;
        fb[i] = (i % 50) * 0.2f;
        fc[i] = (i % 25) * 0.3f;
        dx[i] = sin(i * 0.01);
        dy[i] = cos(i * 0.02);
        indices[i] = i * 3;
        int_arr[i] = i * 7;
    }
    
    float total_sum = 0.0f;
    int int_result = 0;
    double double_result = 0.0;
    
    /* Execute all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with hazards */
        total_sum += hot_loop_with_hazards(fa, fb, fc, ARRAY_SIZE);
        
        /* Call cold function with complex control flow */
        int_result += cold_complex_control_flow(int_arr, ARRAY_SIZE);
        
        /* Call vectorized function */
        double_result += vectorized_mixed_ops(dx, dy, indices, ARRAY_SIZE);
        
        /* Switch-based computation */
        for (int i = 0; i < 100; i++) {
            int_result += switch_based_computation(i + iter);
        }
        
        /* Nested loop scheduling test */
        if (iter % 10 == 0) {
            nested_loop_scheduling(10);
        }
        
        /* Modify data to prevent optimization */
        fa[iter % ARRAY_SIZE] += 0.1f;
        int_arr[iter % ARRAY_SIZE] ^= iter;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("  Float sum: %f\n", total_sum);
    printf("  Int result: %d\n", int_result);
    printf("  Double result: %f\n", double_result);
    
    /* Cleanup */
    free(fa);
    free(fb);
    free(fc);
    free(dx);
    free(dy);
    free(indices);
    free(int_arr);
    
    return 0;
}
