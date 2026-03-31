#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with scheduling pressure attribute */
__attribute__((hot, optimize("O3", "sched-pressure")))
static float hot_function(float *data, int n) {
    float sum = 0.0f;
    float temp[SIZE];
    
    /* Mixed integer and FP operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: read after write */
        float x = data[i] * 2.0f;
        
        /* WAR hazard: write after read */
        data[i] = x + 1.0f;
        
        /* WAW hazard: write after write */
        temp[i] = data[i];
        temp[i] = sinf(temp[i]);  // WAW
        
        /* Complex dependency chain */
        sum += temp[i] * (float)i;
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Cold function with different optimization */
__attribute__((cold, noinline, optimize("O2")))
static double cold_function(double *arr, int start, int end) {
    double result = 0.0;
    
    /* Nested loops with mixed control flow */
    for (int i = start; i < end; i++) {
        /* Pointer chasing pattern */
        double *ptr = &arr[i];
        for (int j = 0; j < 4; j++) {
            if (ptr < &arr[end - 1]) {
                *ptr = *ptr * 1.5 + (double)j;
                ptr++;
                
                /* Assembly with register clobber */
                asm volatile("" : "=r"(result) : "0"(result) : "rax");
            }
        }
        
        /* Conditional move vs branch */
        result = (i % 2 == 0) ? result + *ptr : result - *ptr;
    }
    
    return result;
}

/* SIMD-friendly function with unrolling */
__attribute__((optimize("O3")))
static void vectorized_loop(int *restrict a, int *restrict b, 
                           int *restrict c, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* SIMD-friendly operations */
        a[i] = b[i] * 2 + c[i];
        c[i] = a[i] - b[i] / 3;
        
        /* Create scheduling challenges */
        if (i > 0) {
            b[i] = a[i-1] + c[i];  // Cross-iteration dependency
        }
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
    }
}

/* Function with complex control flow */
__attribute__((noinline))
static int control_flow_test(int x) {
    int result = 0;
    
    /* Switch with sparse cases */
    switch (x % 13) {
        case 0:
            result = x * 2;
            /* Fall through */
        case 1:
        case 2:
            result += x / 3;
            break;
        case 5:
            result = x << 2;
            /* Fall through */
        case 7:
            result ^= 0xAAAA;
            break;
        case 11:
            result = ~x;
            break;
        default:
            result = x + 1;
            
            /* Inline assembly barrier */
            asm volatile("" ::: "rax", "rbx", "memory");
            
            /* Computed goto-like pattern */
            static void *labels[] = { &&L1, &&L2, &&L3 };
            goto *labels[x % 3];
            
        L1:
            result *= 3;
            goto end;
        L2:
            result /= 2;
            goto end;
        L3:
            result += 100;
            goto end;
    }
    
end:
    /* Mixed operations to create scheduling pressure */
    float fresult = (float)result;
    fresult = sinf(fresult) * cosf(fresult);
    result = (int)fresult + result;
    
    return result;
}

/* Outer loop with pipelining opportunities */
__attribute__((optimize("O3", "unroll-loops")))
static double outer_loop_pipelining(double *matrix, int rows, int cols) {
    double total = 0.0;
    
    /* Outer loop for pipelining */
    for (int i = 0; i < rows; i++) {
        double row_sum = 0.0;
        
        /* Inner loop with unrolling */
        #pragma GCC unroll 8
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Multiple dependent operations */
            double val = matrix[idx];
            val = val * val + sqrt(fabs(val));
            
            /* Conditional with side effects */
            row_sum += (val > 0) ? val : -val;
            
            /* Write back with WAW hazard */
            matrix[idx] = val;
            matrix[idx] = log1p(fabs(matrix[idx]));  // WAW
            
            /* Occasional barrier */
            if (j % 16 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        total += row_sum;
        
        /* Early exit condition */
        if (total > 1e6) {
            break;
        }
    }
    
    return total;
}

/* Main test driver */
int main(void) {
    /* Initialize data */
    float float_data[SIZE];
    double double_data[SIZE];
    int int_data_a[SIZE], int_data_b[SIZE], int_data_c[SIZE];
    double matrix[64][64];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        float_data[i] = (float)rand() / RAND_MAX;
        double_data[i] = (double)rand() / RAND_MAX;
        int_data_a[i] = rand() % 1000;
        int_data_b[i] = rand() % 1000;
        int_data_c[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = (double)rand() / RAND_MAX;
        }
    }
    
    float hot_sum = 0.0f;
    double cold_sum = 0.0;
    double pipelined_sum = 0.0;
    int control_flow_result = 0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function */
        hot_sum += hot_function(float_data, SIZE);
        
        /* Call cold function */
        cold_sum += cold_function(double_data, 0, SIZE);
        
        /* Vectorized operations */
        vectorized_loop(int_data_a, int_data_b, int_data_c, SIZE);
        
        /* Control flow test */
        control_flow_result += control_flow_test(iter);
        
        /* Outer loop pipelining */
        pipelined_sum += outer_loop_pipelining((double*)matrix, 64, 64);
        
        /* Modify data between iterations to prevent optimization */
        for (int i = 0; i < SIZE; i++) {
            float_data[i] += 0.001f;
            double_data[i] += 0.001;
            int_data_a[i] ^= iter;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results:\n");
    printf("Hot function sum: %f\n", hot_sum);
    printf("Cold function sum: %f\n", cold_sum);
    printf("Control flow result: %d\n", control_flow_result);
    printf("Pipelined sum: %f\n", pipelined_sum);
    
    /* Verify some computations */
    float final_check = float_data[SIZE-1] + (float)double_data[SIZE-1];
    printf("Final check: %f\n", final_check);
    
    return 0;
}
