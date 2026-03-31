#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3"))) 
static float hot_loop_scheduler_test(float* restrict a, float* restrict b, 
                                     float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: reusing temp */
        b[i] = temp + 1.0f;
        
        /* WAW hazard: multiple writes to c[i] */
        c[i] = a[i] * b[i];
        c[i] = c[i] + temp;  /* WAW hazard */
        
        /* Pointer chasing pattern */
        float* ptr = &c[i];
        *ptr = *ptr * 0.5f;
        
        sum += c[i];
        
        /* Memory barrier forcing scheduler decisions */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

__attribute__((cold, noinline, optimize("sched-pressure")))
static double cold_complex_control_flow(int* restrict arr, int n) {
    double result = 0.0;
    int* ptr = arr;
    
    /* Complex control flow with scheduling challenges */
    for (int i = 0; i < n; i++) {
        /* Switch statement with sparse cases */
        switch (i % 13) {
            case 0:
                arr[i] = i * 2;
                /* Inline asm with register clobber */
                asm volatile("nop" ::: "%eax");
                break;
            case 1:
            case 3:
                arr[i] = i + ptr[i % 8];  /* Pointer arithmetic */
                break;
            case 5:
                /* Conditional move pattern */
                arr[i] = (i > n/2) ? i * 3 : i / 2;
                break;
            default:
                arr[i] = i ^ 0x55AA55AA;
                /* Another scheduling barrier */
                asm volatile("" ::: "memory");
        }
        
        /* Mixed computations */
        if (i % 7 == 0) {
            result += sqrt(fabs((double)arr[i]));
        } else if (i % 3 == 0) {
            result -= log(fabs((double)arr[i]) + 1.0);
        } else {
            /* Compute goto simulation */
            static void* labels[] = { &&add, &&sub, &&mul };
            goto *labels[i % 3];
            
            add:
                result += arr[i];
                continue;
            sub:
                result -= arr[i];
                continue;
            mul:
                result *= (arr[i] % 10 + 1);
                continue;
        }
        
        /* Early exit condition */
        if (result > 1e6) {
            break;
        }
        
        /* Continue with dependency chain */
        if (i % 11 != 0) {
            result = result / (arr[i % 32] + 1);
        }
    }
    
    return result;
}

#pragma GCC unroll 4
__attribute__((optimize("O3")))
static void vectorized_unrolled_test(float* restrict src, float* restrict dst, 
                                     int n) {
    /* SIMD-friendly loop that should vectorize */
    for (int i = 0; i < n; i++) {
        /* Multiple dependent operations */
        float x = src[i];
        float y = x * x;
        float z = y + x;
        
        /* Trigonometric operations with different latencies */
        dst[i] = sin(z) * cos(x) + tan(y * 0.1f);
        
        /* Interleaved integer operations */
        int idx = (int)(fabs(dst[i]) * 100) % n;
        dst[i] += src[idx] * 0.5f;
    }
}

__attribute__((noinline))
static int pointer_chasing_hazards(int** restrict ptr_array, 
                                   int* restrict data, int n) {
    int sum = 0;
    int* current = data;
    
    /* Pointer chasing with hazards */
    for (int i = 0; i < n; i++) {
        /* Load-store sequences */
        int value = *current;
        
        /* RAW: next_ptr depends on value */
        int** next_ptr = &ptr_array[value % n];
        
        /* WAW: multiple writes to sum */
        sum += value;
        sum ^= value * 2;
        
        /* Complex address calculation */
        current = *next_ptr + (value & 0xF);
        
        /* Memory barrier splitting scheduling regions */
        asm volatile("" ::: "memory");
        
        /* More dependencies */
        if (current) {
            sum += *current;
        }
    }
    
    return sum;
}

#pragma GCC unroll 8
__attribute__((optimize("O3")))
static double nested_loop_scheduler_stress(double* restrict matrix, 
                                           int rows, int cols) {
    double total = 0.0;
    
    /* Nested loops with mixed operations */
    for (int i = 0; i < rows; i++) {
        double row_sum = 0.0;
        
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            /* Matrix operations with dependencies */
            double val = matrix[i * cols + j];
            
            /* Floating point with different latencies */
            double t1 = val * val;
            double t2 = sqrt(fabs(val));
            double t3 = t1 + t2;
            
            /* Conditional update */
            row_sum += (val > 0) ? t3 : -t3;
            
            /* Inline asm with specific constraints */
            asm volatile("" : "=r"(val) : "0"(val) : "cc");
        }
        
        total += row_sum;
        
        /* Early continue with condition */
        if (i % 4 == 0) {
            total *= 0.99;
            continue;
        }
        
        /* Additional computation */
        total = fmod(total, 1000.0);
    }
    
    return total;
}

__attribute__((optimize("O3")))
int main() {
    /* Allocate arrays with different alignments */
    float* fdata1 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* fdata2 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    float* fdata3 = (float*)aligned_alloc(32, SIZE * sizeof(float));
    
    int* idata1 = (int*)malloc(SIZE * sizeof(int));
    int* idata2 = (int*)malloc(SIZE * sizeof(int));
    
    double* dmatrix = (double*)aligned_alloc(64, SIZE * SIZE/4 * sizeof(double));
    int** ptr_array = (int**)malloc(SIZE * sizeof(int*));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        fdata1[i] = (float)(i * 0.1);
        fdata2[i] = (float)(i * 0.2);
        idata1[i] = i;
        idata2[i] = i * 2;
        
        if (i < SIZE/4) {
            for (int j = 0; j < SIZE/4; j++) {
                dmatrix[i * (SIZE/4) + j] = (i + j) * 0.01;
            }
        }
    }
    
    for (int i = 0; i < SIZE; i++) {
        ptr_array[i] = &idata1[(i * 17) % SIZE];
    }
    
    double total_result = 0.0;
    
    /* Execute all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Hot loop with mixed hazards */
        float r1 = hot_loop_scheduler_test(fdata1, fdata2, fdata3, SIZE);
        total_result += r1;
        
        /* Test 2: Cold function with complex control flow */
        double r2 = cold_complex_control_flow(idata1, SIZE);
        total_result += r2;
        
        /* Test 3: Vectorized and unrolled loop */
        vectorized_unrolled_test(fdata1, fdata3, SIZE);
        total_result += fdata3[SIZE-1];
        
        /* Test 4: Pointer chasing with hazards */
        int r4 = pointer_chasing_hazards(ptr_array, idata2, SIZE/8);
        total_result += r4;
        
        /* Test 5: Nested loop stress test */
        double r5 = nested_loop_scheduler_stress(dmatrix, SIZE/4, SIZE/4);
        total_result += r5;
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            fdata1[i] += 0.001f;
            idata1[i] ^= iter;
        }
    }
    
    /* Print result to prevent optimization */
    printf("Total result: %f\n", total_result);
    
    /* Cleanup */
    free(fdata1);
    free(fdata2);
    free(fdata3);
    free(idata1);
    free(idata2);
    free(dmatrix);
    free(ptr_array);
    
    return 0;
}
