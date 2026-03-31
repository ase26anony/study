#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3"))) 
__attribute__((optimize("sched-pressure")))
static float hot_function(float* restrict a, float* restrict b, int n) {
    float sum = 0.0f;
    
    /* Mixed data dependencies and pointer chasing */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on previous a calculation */
        float temp = a[i] * 2.5f;
        
        /* WAR hazard: reusing same variable */
        temp = temp + b[i];
        
        /* WAW hazard: multiple writes to sum */
        sum += temp;
        
        /* Inline assembly as scheduling barrier */
        asm volatile("" ::: "memory");
        
        /* Complex expression with mixed operations */
        b[i] = temp * 0.5f + sinf(temp) * cosf(temp);
        
        /* Another memory barrier */
        asm volatile("" : "+r"(sum) :: "memory");
    }
    
    return sum;
}

__attribute__((cold, noinline))
static int cold_function(int* data, int n) {
    int result = 0;
    
    /* Complex control flow with switch statement */
    for (int i = 0; i < n; i++) {
        switch (data[i] % 7) {
            case 0:
                result += data[i] * 2;
                /* Fall through */
            case 1:
                result -= data[i] / 3;
                break;
            case 2:
                result ^= data[i];
                /* Conditional move */
                result = (data[i] > 0) ? result : -result;
                break;
            case 3:
                /* Nested if-else */
                if (data[i] % 2 == 0) {
                    result |= 0xFF;
                } else if (data[i] % 3 == 0) {
                    result &= 0x0F;
                } else {
                    result ^= 0xAA;
                }
                break;
            case 4:
                /* Pointer arithmetic with dependency */
                int* ptr = &data[i];
                result += *ptr + *(ptr + 1);
                break;
            default:
                result = result * 3 - data[i];
        }
        
        /* Early exit condition */
        if (result > 1000000) {
            break;
        }
        
        /* Continue condition */
        if (data[i] < 0) {
            continue;
        }
        
        /* Additional computation */
        result += i % 5;
    }
    
    return result;
}

__attribute__((optimize("O3")))
#pragma GCC unroll 4
static void vectorized_loop(double* restrict arr1, double* restrict arr2, 
                           double* restrict out, int n) {
    /* SIMD-friendly loop that should trigger vectorization */
    for (int i = 0; i < n; i++) {
        /* Mixed FP and integer operations */
        double val1 = arr1[i];
        double val2 = arr2[i];
        
        /* Complex dependency chain */
        double temp = val1 * val2 + sin(val1) * cos(val2);
        
        /* Conditional computation */
        temp = (temp > 0.0) ? sqrt(temp) : -sqrt(-temp);
        
        /* Store with dependency */
        out[i] = temp + (double)i * 0.01;
        
        /* Assembly with register clobber */
        asm volatile("" : "+r"(temp) :: "r8", "r9", "r10");
    }
}

__attribute__((noinline))
static int pointer_chasing_test(int* base, int steps) {
    int* current = base;
    int sum = 0;
    
    /* Pointer chasing with data dependencies */
    for (int i = 0; i < steps; i++) {
        /* Load with potential cache miss pattern */
        int value = *current;
        
        /* Computation with dependency */
        sum += value * i;
        
        /* Update pointer based on computation */
        current = base + (value % 256);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Additional dependent operation */
        sum ^= (int)(current - base);
    }
    
    return sum;
}

__attribute__((optimize("O3")))
static void nested_loop_scheduler_stress(float* mat, int rows, int cols) {
    /* Nested loops with mixed dependencies */
    for (int i = 0; i < rows; i++) {
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            /* Matrix operations with RAW hazards */
            float prev = (i > 0) ? mat[(i-1)*cols + j] : 0.0f;
            float curr = mat[i*cols + j];
            
            /* Mixed operations */
            float result = prev * 1.5f + curr * 2.0f;
            
            /* Trigonometric operations */
            result = sinf(result) * cosf(result);
            
            /* Store with WAW hazard potential */
            mat[i*cols + j] = result;
            
            /* Conditional store creating WAR hazard */
            if (j % 3 == 0) {
                mat[i*cols + j] += 1.0f;
            }
        }
        
        /* Outer loop computation */
        asm volatile("" ::: "r12", "r13", "r14", "r15");
    }
}

/* Main test driver */
int main(void) {
    /* Allocate and initialize test data */
    float* fdata1 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* fdata2 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    double* ddata1 = (double*)aligned_alloc(32, SIZE * sizeof(double));
    double* ddata2 = (double*)aligned_alloc(32, SIZE * sizeof(double));
    double* dout = (double*)aligned_alloc(32, SIZE * sizeof(double));
    int* idata = (int*)malloc(SIZE * sizeof(int));
    float* matrix = (float*)aligned_alloc(32, 64 * 64 * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        fdata1[i] = (float)i * 0.1f;
        fdata2[i] = (float)i * 0.2f;
        ddata1[i] = (double)i * 0.01;
        ddata2[i] = (double)i * 0.02;
        idata[i] = i * 3 - SIZE/2;
    }
    
    for (int i = 0; i < 64*64; i++) {
        matrix[i] = (float)(i % 100) * 0.1f;
    }
    
    float total_float = 0.0f;
    int total_int = 0;
    double total_double = 0.0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with scheduling pressure */
        total_float += hot_function(fdata1, fdata2, SIZE);
        
        /* Call cold function with complex control flow */
        total_int += cold_function(idata, SIZE);
        
        /* Vectorized loop */
        vectorized_loop(ddata1, ddata2, dout, SIZE);
        for (int i = 0; i < SIZE; i++) {
            total_double += dout[i];
        }
        
        /* Pointer chasing test */
        total_int += pointer_chasing_test(idata, 1000);
        
        /* Nested loop stress test */
        nested_loop_scheduler_stress(matrix, 64, 64);
        for (int i = 0; i < 64*64; i++) {
            total_float += matrix[i];
        }
        
        /* Modify data for next iteration */
        for (int i = 0; i < SIZE; i++) {
            fdata1[i] += 0.01f;
            idata[i] ^= iter;
        }
    }
    
    /* Print results to prevent optimization */
    printf("Results: float=%f, int=%d, double=%f\n", 
           total_float, total_int, total_double);
    
    /* Cleanup */
    free(fdata1);
    free(fdata2);
    free(ddata1);
    free(ddata2);
    free(dout);
    free(idata);
    free(matrix);
    
    return 0;
}
