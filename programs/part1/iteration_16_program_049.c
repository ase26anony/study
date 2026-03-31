#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

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
        float t1 = a[i] * 2.0f;
        b[i] = t1 + 1.0f;
        
        /* WAR hazard: reusing t1 */
        t1 = b[i] * 3.0f;
        
        /* WAW hazard: multiple writes to c[i] */
        c[i] = t1;
        c[i] = c[i] + a[i];  /* This creates WAW */
        
        /* Pointer chasing pattern */
        float* ptr = &c[i];
        *ptr = *ptr * (*ptr);
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" ::: "memory");
        
        sum += c[i];
    }
    
    return sum;
}

__attribute__((cold, noinline))
static float cold_control_flow_test(float* restrict arr, int n) {
    float result = 0.0f;
    
    /* Complex control flow with switch */
    for (int i = 0; i < n; i++) {
        int mod = i % 7;
        
        switch (mod) {
            case 0:
                arr[i] = arr[i] * 2.0f;
                break;
            case 1:
                arr[i] = sqrtf(arr[i]);
                break;
            case 2:
                arr[i] = arr[i] + arr[i-1 < 0 ? 0 : i-1];
                break;
            case 3:
                /* Conditional move pattern */
                arr[i] = (i > n/2) ? arr[i] * 3.0f : arr[i] / 3.0f;
                break;
            case 4:
                arr[i] = arr[i] * arr[i];
                break;
            case 5:
                /* Early continue */
                if (arr[i] < 0) continue;
                arr[i] = -arr[i];
                break;
            default:
                arr[i] = 0.0f;
                break;
        }
        
        result += arr[i];
    }
    
    return result;
}

__attribute__((optimize("O3")))
#pragma GCC unroll 4
static void vectorized_unrolled_test(double* restrict a, double* restrict b, 
                                     double* restrict c, int n) {
    /* SIMD-friendly loop that should vectorize */
    for (int i = 0; i < n; i++) {
        /* Mixed operations to create scheduling pressure */
        double t1 = a[i] * b[i];
        double t2 = sin(t1);
        double t3 = cos(a[i]);
        
        /* Assembly with register clobbering */
        asm volatile(
            "movq %0, %%rax\n\t"
            "addq $1, %%rax\n\t"
            : 
            : "r" ((uint64_t)i)
            : "rax", "cc"
        );
        
        c[i] = t2 + t3 * t1;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
    }
}

__attribute__((optimize("sched-pressure")))
static int integer_pointer_chasing(int* restrict base, int n) {
    int sum = 0;
    int* current = base;
    
    /* Pointer chasing with mixed operations */
    for (int i = 0; i < n; i++) {
        /* Load with potential cache miss pattern */
        int val = *current;
        
        /* Arithmetic chain with dependencies */
        val = val * 3 + 1;
        val = val ^ (val >> 1);
        val = val * 7 - 5;
        
        /* Store and update pointer */
        *current = val;
        sum += val;
        
        /* Next pointer location with wrap-around */
        current = base + ((val % (n-1)) + 1);
        
        /* Multiple exit points */
        if (sum > 1000000) break;
        if (val < 0) continue;
    }
    
    return sum;
}

__attribute__((noinline))
static float nested_loop_hazard_test(float* restrict mat, int rows, int cols) {
    float total = 0.0f;
    
    /* Nested loops with data hazards */
    for (int i = 1; i < rows - 1; i++) {
        #pragma GCC unroll 2
        for (int j = 1; j < cols - 1; j++) {
            /* Stencil computation with multiple dependencies */
            int idx = i * cols + j;
            
            /* RAW hazards on neighboring elements */
            float up = mat[idx - cols];
            float down = mat[idx + cols];
            float left = mat[idx - 1];
            float right = mat[idx + 1];
            
            /* Computation with mixed latencies */
            float temp1 = up + down;
            float temp2 = left * right;
            
            /* Memory barrier between dependent operations */
            asm volatile("" ::: "memory");
            
            float new_val = (temp1 + temp2) * 0.25f;
            
            /* WAW hazard - multiple potential writes */
            float old_val = mat[idx];
            mat[idx] = (new_val > old_val) ? new_val : old_val;
            
            total += mat[idx];
        }
    }
    
    return total;
}

/* Main test driver */
int main(void) {
    float* a = aligned_alloc(64, SIZE * sizeof(float));
    float* b = aligned_alloc(64, SIZE * sizeof(float));
    float* c = aligned_alloc(64, SIZE * sizeof(float));
    double* da = aligned_alloc(64, SIZE * sizeof(double));
    double* db = aligned_alloc(64, SIZE * sizeof(double));
    double* dc = aligned_alloc(64, SIZE * sizeof(double));
    int* int_arr = aligned_alloc(64, SIZE * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)(i % 50) * 0.2f;
        c[i] = (float)(i % 25) * 0.3f;
        da[i] = (double)(i % 100) * 0.01;
        db[i] = (double)(i % 50) * 0.02;
        dc[i] = (double)(i % 25) * 0.03;
        int_arr[i] = i * 3 + 1;
    }
    
    float total = 0.0f;
    
    /* Run all test functions multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += hot_loop_scheduler_test(a, b, c, SIZE);
        total += cold_control_flow_test(a, SIZE);
        vectorized_unrolled_test(da, db, dc, SIZE);
        total += integer_pointer_chasing(int_arr, SIZE);
        
        /* Matrix test */
        float* matrix = aligned_alloc(64, 64 * 64 * sizeof(float));
        for (int i = 0; i < 64 * 64; i++) {
            matrix[i] = (float)(i % 255) * 0.01f;
        }
        total += nested_loop_hazard_test(matrix, 64, 64);
        free(matrix);
        
        /* Modify inputs slightly each iteration */
        for (int i = 0; i < SIZE; i++) {
            a[i] += 0.001f;
            da[i] += 0.0001;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(da);
    free(db);
    free(dc);
    free(int_arr);
    
    return 0;
}
