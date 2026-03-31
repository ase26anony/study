/* sel-sched-test.c - Stress test for GCC selective scheduler dump logic */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, noinline, optimize("O3", "unroll-loops")))
static float hot_loop_scheduler_test(float *restrict a, float *restrict b, 
                                     float *restrict c, int n) {
    volatile float sum = 0.0f;
    
    /* Mixed data dependencies with RAW, WAR, WAW hazards */
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on a */
        float t1 = a[i] * 2.0f;
        asm volatile("" ::: "memory");  /* Scheduling barrier */
        
        /* WAR hazard: t2 overwrites t1's register */
        float t2 = t1 + b[i];
        float t3 = t2 * 0.5f;
        
        /* WAW hazard: multiple writes to c[i] */
        c[i] = t3;
        c[i] = c[i] + sinf(t3);  /* FP operation mixed with integer indexing */
        
        /* Pointer chasing simulation */
        float *ptr = &c[i];
        *ptr = *ptr * (*ptr);
        
        sum += c[i];
    }
    
    return sum;
}

__attribute__((cold, noinline, optimize("sched-pressure")))
static int cold_control_flow_test(int *restrict arr, int n) {
    int result = 0;
    
    /* Complex control flow with switch and computed gotos */
    for (int i = 0; i < n; i++) {
        int val = arr[i] % 7;
        
        switch (val) {
            case 0:
                result += arr[i] * 2;
                /* Fall through */
            case 1:
                result += arr[i] >> 1;
                break;
            case 2:
                /* Conditional move mixed with branch */
                result += (val > 1) ? arr[i] : -arr[i];
                break;
            case 3:
                result += arr[i] & 0xFF;
                break;
            case 4:
                /* Early exit point */
                if (result > 1000000) return result;
                continue;
            case 5:
                result ^= arr[i];
                break;
            default:
                result = result * 2 - arr[i];
        }
        
        /* Multiple continue conditions */
        if (arr[i] < 0) continue;
        if ((arr[i] & 1) == 0) {
            result += 1;
            continue;
        }
    }
    
    return result;
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vectorization_unroll_test(double *restrict a, double *restrict b,
                                      double *restrict c, int n) {
    /* SIMD-friendly loop with varying unroll factors */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i += 4) {
        /* Vectorizable operations */
        double t0 = a[i] * b[i];
        double t1 = a[i+1] * b[i+1];
        double t2 = a[i+2] * b[i+2];
        double t3 = a[i+3] * b[i+3];
        
        /* Mixed FP operations */
        c[i] = t0 + sin(t0);
        c[i+1] = t1 + cos(t1);
        c[i+2] = t2 + exp(t2 * 0.1);
        c[i+3] = t3 + log(fabs(t3) + 1.0);
        
        /* Inline asm with register clobbers */
        asm volatile("" : "+r"(c[i]), "+r"(c[i+1]) :: "xmm0", "xmm1", "memory");
    }
}

__attribute__((noinline, optimize("O2")))
static int pointer_chasing_hazards(int **restrict ptr_array, int n) {
    int sum = 0;
    int *current = ptr_array[0];
    
    /* Pointer chasing with memory dependencies */
    for (int i = 0; i < n; i++) {
        /* Load-store sequences with varying latencies */
        int val = *current;
        asm volatile("" ::: "memory");  /* Memory barrier */
        
        /* WAW hazard on sum */
        sum = sum + val;
        sum = sum ^ (val * 2);
        
        /* RAW hazard: next depends on current computation */
        int next_idx = (val + i) % n;
        current = ptr_array[next_idx];
        
        /* Mixed operations to create scheduling pressure */
        sum = (sum << 3) | (sum >> 29);  /* Rotation */
        sum += (val % 2 == 0) ? val : -val;
    }
    
    return sum;
}

__attribute__((optimize("O3", "unroll-all-loops")))
static double nested_loop_scheduler_test(double *restrict matrix, int size) {
    double total = 0.0;
    
    /* Nested loops with complex dependencies */
    for (int i = 0; i < size; i++) {
        double row_sum = 0.0;
        
        #pragma GCC unroll 2
        for (int j = 0; j < size; j++) {
            /* Matrix operations with cross-iteration dependencies */
            double elem = matrix[i * size + j];
            
            /* FP and integer mix */
            double transformed = elem * (i + 1) / (j + 1);
            transformed = sin(transformed) * cos(transformed);
            
            /* Conditional store */
            matrix[i * size + j] = (transformed > 0) ? transformed : -transformed;
            
            row_sum += transformed;
            
            /* Scheduling barrier with clobber */
            if (j % 8 == 0) {
                asm volatile("" : "+r"(row_sum) :: "xmm2", "xmm3", "memory");
            }
        }
        
        total += row_sum;
        
        /* Early exit with complex condition */
        if (total > 1e6 && i > size / 2) {
            break;
        }
    }
    
    return total;
}

/* Main test driver */
int main(void) {
    /* Allocate and initialize test arrays */
    float *fa = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *fb = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *fc = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    
    double *da = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double *db = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double *dc = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    
    int *int_arr = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    int **ptr_arr = (int**)malloc(ARRAY_SIZE * sizeof(int*));
    
    double *matrix = (double*)aligned_alloc(32, 64 * 64 * sizeof(double));
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX;
        fb[i] = (float)rand() / RAND_MAX;
        da[i] = (double)rand() / RAND_MAX;
        db[i] = (double)rand() / RAND_MAX;
        int_arr[i] = rand() % 1000;
        ptr_arr[i] = &int_arr[(i * 13) % ARRAY_SIZE];
    }
    
    for (int i = 0; i < 64 * 64; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
    
    /* Accumulator to prevent dead code elimination */
    volatile float total_f = 0.0f;
    volatile double total_d = 0.0;
    volatile int total_i = 0;
    
    /* Run multiple iterations to ensure coverage */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call each test function with different scheduling characteristics */
        total_f += hot_loop_scheduler_test(fa, fb, fc, ARRAY_SIZE);
        total_i += cold_control_flow_test(int_arr, ARRAY_SIZE);
        vectorization_unroll_test(da, db, dc, ARRAY_SIZE);
        total_i += pointer_chasing_hazards(ptr_arr, ARRAY_SIZE / 16);
        total_d += nested_loop_scheduler_test(matrix, 64);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i += 17) {
            fa[i] += 0.1f;
            int_arr[i] ^= iter;
        }
    }
    
    /* Print results to ensure execution */
    printf("Results: f=%f, i=%d, d=%f\n", 
           (double)total_f, total_i, total_d);
    
    /* Cleanup */
    free(fa); free(fb); free(fc);
    free(da); free(db); free(dc);
    free(int_arr);
    free(ptr_arr);
    free(matrix);
    
    return 0;
}
