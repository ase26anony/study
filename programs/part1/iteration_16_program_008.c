/* sel-sched-test.c - Comprehensive test for GCC selective scheduler dump logic */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function with hot attribute and scheduling optimization */
__attribute__((hot, optimize("O3", "sched-pressure")))
static float hot_function(float* data, int size, int* indices) {
    float sum = 0.0f;
    float temp[ARRAY_SIZE];
    
    /* Complex loop with mixed dependencies */
    #pragma GCC unroll 4
    for (int i = 0; i < size; i++) {
        /* RAW hazard: read after write to temp */
        temp[i] = data[i] * 2.5f;
        
        /* WAR hazard: write after read to data */
        data[i] = sinf(temp[i]) + 1.0f;
        
        /* WAW hazard: write after write to sum */
        sum += data[i];
        
        /* Pointer chasing with varying latency */
        int idx = indices[i];
        if (idx >= 0 && idx < size) {
            /* Memory barrier to force scheduling decisions */
            asm volatile("" ::: "memory");
            
            /* WAW hazard on sum */
            sum += temp[idx] * 0.5f;
        }
        
        /* Mixed integer/float operations */
        int int_val = (int)data[i];
        float float_val = int_val * 0.333f;
        
        /* Conditional move vs branch */
        sum += (i % 2 == 0) ? float_val : -float_val;
    }
    
    return sum;
}

/* Cold function with noinline to create scheduling boundaries */
__attribute__((cold, noinline))
static double cold_function(double* array, int n) {
    double result = 0.0;
    
    /* Switch statement with sparse cases */
    for (int i = 0; i < n; i++) {
        switch (i % 7) {
            case 0:
                result += array[i] * 1.1;
                /* Fall through */
            case 1:
                result += sqrt(array[i]);
                break;
            case 2:
            case 3:
                result += array[i] * array[i];
                /* Inline asm with register clobber */
                asm volatile("" : : : "r0", "r1", "r2", "r3");
                break;
            case 4:
                /* Complex expression with multiple dependencies */
                result += (array[i] > 0.5) ? 
                         array[i] * log(array[i] + 1.0) : 
                         array[i] * array[i] * 0.5;
                break;
            default:
                result += 1.0 / (array[i] + 1.0);
        }
        
        /* Memory barrier between dependent operations */
        if (i % 3 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return result;
}

/* Function with vectorization-friendly pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vectorized_loop(float* a, float* b, float* c, int size) {
    /* SIMD-friendly loop with auto-vectorization */
    #pragma GCC unroll 8
    for (int i = 0; i < size; i++) {
        /* Independent operations that can be vectorized */
        float t1 = a[i] * b[i];
        float t2 = sinf(a[i]) + cosf(b[i]);
        
        /* RAW hazard */
        c[i] = t1 + t2;
        
        /* Cross-iteration dependency to challenge scheduler */
        if (i > 0) {
            c[i] += c[i-1] * 0.1f;
        }
        
        /* Mixed precision operations */
        double dbl_val = (double)c[i] * 1.5;
        c[i] = (float)dbl_val;
    }
}

/* Function with nested loops and complex control flow */
__attribute__((noinline))
static int complex_control_flow(int* matrix, int rows, int cols) {
    int total = 0;
    int early_exit_count = 0;
    
    for (int i = 0; i < rows; i++) {
        /* Multiple early exit points */
        if (early_exit_count > 10) {
            break;
        }
        
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Complex condition with continue */
            if (matrix[idx] % 13 == 0) {
                early_exit_count++;
                continue;
            }
            
            /* Computed goto-like pattern using switch */
            switch (matrix[idx] % 5) {
                case 0:
                    total += matrix[idx] * 2;
                    /* Inline asm barrier */
                    asm volatile("" ::: "memory");
                    break;
                case 1:
                    total -= matrix[idx];
                    break;
                case 2:
                    total *= (matrix[idx] % 3) + 1;
                    /* Register clobber to force spills */
                    asm volatile("" : : : "r4", "r5", "r6", "r7");
                    break;
                case 3:
                    /* Nested ternary operators */
                    total += (matrix[idx] > 100) ? 
                            matrix[idx] / 2 : 
                            (matrix[idx] < 50) ? matrix[idx] * 3 : matrix[idx];
                    break;
                default:
                    total ^= matrix[idx];
            }
            
            /* Another memory barrier */
            if ((i + j) % 4 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Loop-carried dependency */
        matrix[i * cols] = total % 1000;
    }
    
    return total;
}

/* Main test driver */
int main(void) {
    /* Initialize data arrays */
    float float_data[ARRAY_SIZE];
    double double_data[ARRAY_SIZE];
    float vec_a[ARRAY_SIZE], vec_b[ARRAY_SIZE], vec_c[ARRAY_SIZE];
    int indices[ARRAY_SIZE];
    int matrix[64][64];  /* 64x64 matrix */
    
    srand(time(NULL));
    
    /* Initialize arrays with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data[i] = (double)rand() / RAND_MAX * 100.0;
        vec_a[i] = (float)rand() / RAND_MAX;
        vec_b[i] = (float)rand() / RAND_MAX;
        indices[i] = rand() % ARRAY_SIZE;
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = rand() % 256;
        }
    }
    
    float hot_sum = 0.0f;
    double cold_sum = 0.0;
    int control_flow_result = 0;
    
    /* Execute multiple iterations to ensure coverage */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function - should trigger selective scheduling */
        hot_sum += hot_function(float_data, ARRAY_SIZE, indices);
        
        /* Call cold function - creates scheduling boundaries */
        cold_sum += cold_function(double_data, ARRAY_SIZE / 2);
        
        /* Vectorized loop - interacts with unrolling and scheduling */
        vectorized_loop(vec_a, vec_b, vec_c, ARRAY_SIZE);
        
        /* Complex control flow - challenges region formation */
        if (iter % 10 == 0) {
            control_flow_result += complex_control_flow(&matrix[0][0], 64, 64);
        }
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i += 17) {
            float_data[i] *= 1.01f;
            double_data[i] *= 0.99;
            indices[i] = (indices[i] + 1) % ARRAY_SIZE;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results:\n");
    printf("  Hot function sum: %f\n", hot_sum);
    printf("  Cold function sum: %f\n", cold_sum);
    printf("  Control flow result: %d\n", control_flow_result);
    
    /* Final computation using all results */
    float final_result = (float)hot_sum + (float)cold_sum + (float)control_flow_result;
    printf("  Final combined result: %f\n", final_result);
    
    /* Additional pointer-chasing pattern */
    float* ptr = float_data;
    float ptr_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        ptr_sum += *ptr;
        /* Pointer arithmetic with dependency */
        ptr = &float_data[(int)(*ptr) % ARRAY_SIZE];
        
        /* Memory barrier in pointer chase */
        asm volatile("" ::: "memory");
    }
    printf("  Pointer chase sum: %f\n", ptr_sum);
    
    return (final_result > 0) ? 0 : 1;
}
