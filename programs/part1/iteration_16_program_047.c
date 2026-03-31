#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3"), noinline))
static float hot_loop_scheduler(float* restrict a, float* restrict b, 
                               float* restrict c, int size) {
    volatile float sum = 0.0f;
    
    /* Mixed integer and FP operations with dependencies */
    for (int i = 0; i < size; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: temp is written then read */
        b[i] = temp + 1.0f;
        
        /* WAW hazard: multiple writes to sum */
        sum += b[i];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Another dependent operation */
        c[i] = sum * 0.5f;
        
        /* Complex expression with mixed operations */
        a[i] = (temp > 0.0f) ? sqrtf(fabsf(temp)) : -sqrtf(fabsf(temp));
    }
    
    return sum;
}

__attribute__((cold, optimize("sched-pressure"), noinline))
static int cold_control_flow(int* restrict arr, int size) {
    int result = 0;
    
    /* Complex control flow with switch */
    for (int i = 0; i < size; i++) {
        switch (arr[i] % 7) {
            case 0:
                result += arr[i] * 2;
                /* Memory barrier */
                asm volatile("" ::: "memory");
                break;
            case 1:
                result -= arr[i];
                break;
            case 2:
                result ^= arr[i];
                /* Register clobber */
                asm volatile("" : : : "eax", "ebx", "ecx");
                break;
            case 3:
                result |= arr[i] << 2;
                break;
            case 4:
                result &= ~arr[i];
                break;
            case 5:
                result = (result > 1000) ? result / 2 : result * 2;
                break;
            default:
                result = result + (arr[i] % 3);
                /* Another barrier */
                asm volatile("" ::: "memory");
        }
        
        /* Early exit condition */
        if (result > 1000000) {
            break;
        }
        
        /* Continue condition */
        if (arr[i] < 0) {
            continue;
        }
        
        /* Conditional move style */
        result = (i % 2 == 0) ? result + 1 : result - 1;
    }
    
    return result;
}

__attribute__((optimize("O3"), noinline))
static void vectorized_unrolled_loop(
    double* restrict src, 
    double* restrict dst, 
    int size) {
    
    int i;
    
    /* Manual unrolling hint with SIMD-friendly pattern */
#pragma GCC unroll 4
    for (i = 0; i < size - 3; i += 4) {
        /* SIMD-friendly operations */
        dst[i]     = src[i]     * src[i]     + 1.0;
        dst[i+1]   = src[i+1]   * src[i+1]   + 2.0;
        dst[i+2]   = src[i+2]   * src[i+2]   + 3.0;
        dst[i+3]   = src[i+3]   * src[i+3]   + 4.0;
        
        /* Cross-iteration dependency */
        if (i > 0) {
            dst[i] += dst[i-1] * 0.1;
        }
        
        /* Memory barrier between dependent operations */
        asm volatile("" ::: "memory");
    }
    
    /* Remainder loop */
    for (; i < size; i++) {
        dst[i] = src[i] * src[i] + 5.0;
    }
}

__attribute__((optimize("O3")))
static float pointer_chasing_pattern(
    float** restrict ptr_array, 
    int array_size, 
    int chase_depth) {
    
    float accumulator = 0.0f;
    
    /* Pointer chasing with mixed access patterns */
    for (int i = 0; i < array_size; i++) {
        float* current = ptr_array[i];
        
        for (int j = 0; j < chase_depth; j++) {
            /* Load with potential cache miss */
            float val = *current;
            
            /* Arithmetic chain */
            val = val * 1.5f + (float)j;
            val = sinf(val) * cosf(val);
            
            /* Store with dependency */
            *current = val;
            accumulator += val;
            
            /* Pointer arithmetic with barrier */
            asm volatile("" ::: "memory");
            current = (float*)((uintptr_t)current + sizeof(float) * (j % 8));
        }
        
        /* Conditional early exit */
        if (accumulator > 10000.0f && i > array_size / 2) {
            break;
        }
    }
    
    return accumulator;
}

__attribute__((optimize("O3"), noinline))
static int nested_loop_hazards(int size) {
    int matrix[64][64];
    int sum = 0;
    
    /* Nested loops with various hazards */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* WAW hazard on sum */
            sum = i * j;
            
            /* RAW hazard: matrix depends on sum */
            matrix[i][j] = sum + i - j;
            
            /* Complex condition with branch */
            if ((i + j) % 3 == 0) {
                sum += matrix[i][j] * 2;
                asm volatile("" ::: "memory");
            } else if ((i * j) % 5 == 0) {
                sum -= matrix[i][j] / 2;
            } else {
                sum ^= matrix[i][j];
            }
            
            /* WAR hazard: matrix read after write */
            if (j > 0) {
                matrix[i][j] += matrix[i][j-1];
            }
        }
        
        /* Loop-carried dependency */
        if (i > 0) {
            sum += matrix[i-1][size-1];
        }
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    /* Allocate and initialize arrays */
    float* a = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* b = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* c = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    double* src = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    double* dst = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    int* int_arr = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    /* Array of pointers for chasing */
    float** ptr_array = (float**)malloc(ARRAY_SIZE/16 * sizeof(float*));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)i * 0.2f;
        c[i] = 0.0f;
        src[i] = (double)i * 0.01;
        dst[i] = 0.0;
        int_arr[i] = i * 3 - ARRAY_SIZE/2;
    }
    
    for (int i = 0; i < ARRAY_SIZE/16; i++) {
        ptr_array[i] = &a[i * 16];
    }
    
    float total_sum = 0.0f;
    int int_result = 0;
    double dbl_result = 0.0;
    
    /* Run multiple iterations to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function with scheduling challenges */
        total_sum += hot_loop_scheduler(a, b, c, ARRAY_SIZE);
        
        /* Complex control flow function */
        int_result += cold_control_flow(int_arr, ARRAY_SIZE);
        
        /* Vectorized and unrolled loop */
        vectorized_unrolled_loop(src, dst, ARRAY_SIZE);
        for (int i = 0; i < ARRAY_SIZE; i++) {
            dbl_result += dst[i];
        }
        
        /* Pointer chasing pattern */
        total_sum += pointer_chasing_pattern(ptr_array, ARRAY_SIZE/16, 8);
        
        /* Nested loop hazards */
        int_result += nested_loop_hazards(64);
        
        /* Modify data for next iteration */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            a[i] += 0.01f;
            int_arr[i] ^= iter;
            src[i] *= 1.001;
        }
    }
    
    /* Print results to prevent optimization */
    printf("Results: sum=%f, int=%d, dbl=%f\n", 
           total_sum, int_result, dbl_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(src);
    free(dst);
    free(int_arr);
    free(ptr_array);
    
    return 0;
}
