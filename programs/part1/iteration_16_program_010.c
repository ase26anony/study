/* Selective Scheduling Stress Test for GCC sel-sched-dump.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3"))) 
static float hot_function(float *data, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        float x = data[i];
        int idx = i & 0xFF;
        
        /* RAW hazard: x used multiple times */
        float t1 = x * x;
        float t2 = t1 + x;      /* Depends on t1 */
        float t3 = t2 * 2.0f;   /* Depends on t2 */
        data[idx] = t3;         /* WAW hazard with other writes */
        
        /* Pointer chasing pattern */
        float *ptr = &data[idx];
        float val = *ptr;       /* Load after store - WAR hazard */
        sum += val * (i % 8);
    }
    
    return sum;
}

__attribute__((cold, noinline))
static double cold_function(double *arr, int size) {
    double result = 0.0;
    
    /* Complex control flow with scheduling barriers */
    for (int i = 0; i < size; i++) {
        /* Inline assembly as scheduling barrier */
        asm volatile("" ::: "memory");
        
        /* Conditional moves mixed with branches */
        double x = arr[i];
        if (x > 0.5) {
            result += sqrt(x);
        } else {
            result += x * x;
        }
        
        /* Another assembly barrier with clobbers */
        asm volatile("" ::: "r0", "r1", "r2", "r3", "memory");
    }
    
    return result;
}

__attribute__((optimize("sched-pressure")))
static void vectorized_loop(int *restrict a, int *restrict b, 
                           int *restrict c, int n) {
    /* SIMD-friendly loop that should vectorize */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Multiple dependent operations */
        int t1 = a[i] * 3;
        int t2 = b[i] + t1;      /* RAW on t1 */
        int t3 = t2 << 2;        /* RAW on t2 */
        c[i] = t3 | 0x1;         /* RAW on t3 */
        
        /* Create anti-dependency */
        a[i] = t3 + i;           /* WAR on t3 */
    }
}

__attribute__((noinline))
static int switch_control_flow(int x) {
    /* Switch with sparse cases to challenge scheduler */
    int result = 0;
    
    switch (x & 0xF) {
        case 0:  result = x * 2; break;
        case 1:  result = x + x; break;
        case 3:  result = x >> 1; break;
        case 7:  result = x & 0xFF; break;
        case 15: result = ~x; break;
        default: result = x ^ 0xAAAA; break;
    }
    
    /* Mixed operations after switch */
    asm volatile("" ::: "memory");
    return result * 3;
}

__attribute__((optimize("O3")))
static void nested_loop_hazards(float *matrix, int rows, int cols) {
    /* Nested loops with complex dependencies */
    for (int i = 0; i < rows; i++) {
        float row_sum = 0.0f;
        
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            /* Multiple interleaved dependencies */
            float a = matrix[i * cols + j];
            float b = a * 0.5f;           /* RAW on a */
            float c = sinf(b);            /* RAW on b */
            matrix[i * cols + j] = c;     /* WAW hazard */
            
            /* Pointer chasing within loop */
            float *elem = &matrix[i * cols + j];
            row_sum += *elem + j;         /* WAR on previous store */
        }
        
        /* Store with dependency on inner loop */
        matrix[i * cols] = row_sum;
    }
}

__attribute__((optimize("O2")))
static void mixed_precision_ops(short *src, int *dst, int n) {
    /* Mixed data types and operations */
    for (int i = 0; i < n; i++) {
        /* Type conversions create scheduling constraints */
        short s = src[i];
        int t1 = (int)s * 17;
        int t2 = t1 + (s >> 2);   /* RAW on t1, RAW on s */
        dst[i] = t2 ^ (i * 3);    /* RAW on t2 */
        
        /* Assembly with specific register constraints */
        if (i % 16 == 0) {
            asm volatile(
                "mov %0, %0\n\t"
                : "+r" (dst[i])
                :
                : "cc"
            );
        }
    }
}

/* Main test driver */
int main(void) {
    /* Allocate and initialize test data */
    float *float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int *int_data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_data3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    short *short_data = (short*)malloc(ARRAY_SIZE * sizeof(short));
    float *matrix = (float*)malloc(64 * 64 * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        float_data[i] = (i % 255) * 0.01f;
        double_data[i] = (i % 127) * 0.02;
        int_data1[i] = i * 3;
        int_data2[i] = i * 5;
        short_data[i] = (short)(i * 7);
    }
    
    for (int i = 0; i < 64 * 64; i++) {
        matrix[i] = (i % 64) * 0.1f;
    }
    
    /* Accumulator for results to prevent dead code elimination */
    float total = 0.0f;
    
    /* Execute test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call each test function with dependencies between calls */
        float r1 = hot_function(float_data, ARRAY_SIZE);
        double r2 = cold_function(double_data, ARRAY_SIZE);
        
        vectorized_loop(int_data1, int_data2, int_data3, ARRAY_SIZE);
        
        int cf_result = 0;
        for (int i = 0; i < 32; i++) {
            cf_result += switch_control_flow(int_data1[i]);
        }
        
        nested_loop_hazards(matrix, 64, 64);
        
        mixed_precision_ops(short_data, int_data1, ARRAY_SIZE);
        
        /* Accumulate results with mixing */
        total += r1 + (float)r2 + cf_result * 0.01f;
        
        /* Modify data between iterations to create new scheduling patterns */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            float_data[i] += 0.001f;
            double_data[i] += 0.0005;
            int_data1[i] ^= iter;
        }
    }
    
    /* Print final result to ensure all code executes */
    printf("Result: %f\n", total);
    
    /* Cleanup */
    free(float_data);
    free(double_data);
    free(int_data1);
    free(int_data2);
    free(int_data3);
    free(short_data);
    free(matrix);
    
    return (int)(total * 0.001) & 0xFF;
}
