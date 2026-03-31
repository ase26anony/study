#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow to generate interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = data[i] * 2.0f - threshold;
        } else {
            data[i] = data[i] * 0.5f + threshold;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        /* Multiple conditions to create complex GIMPLE sequence */
        float val = a[i] + b[i];
        if (val > 100.0f) {
            c[i] = val * 2.0f;
        } else if (val < 50.0f) {
            c[i] = val * 0.5f;
        } else {
            c[i] = val;
        }
    }
}

__attribute__((noinline))
void target_multi_clause(float *arr, int size, int offset) {
    /* Multiple clauses to test clause processing */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) \
        num_teams(2) thread_limit(128) \
        private(offset) firstprivate(size)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] + (i % 32) * 0.1f + offset;
    }
}

int main(int argc, char *argv[]) {
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    
    /* Initialize arrays with test data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)i;
        array2[i] = (float)(N - i);
        array3[i] = 0.0f;
    }
    
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Loop over different configurations to increase coverage */
    for (int config = 0; config < 3; config++) {
        int size = N / (config + 1);
        
        switch (test_case) {
            case 1:
                /* Basic SIMD clause with vector scaling */
                target_simt_vector_scale(array1, size, 3.14159f);
                break;
                
            case 2:
                /* Conditional update with teams-distribute */
                target_simt_conditional_update(array1, size, THRESHOLD);
                break;
                
            case 3:
                /* Nested control flow */
                target_simt_nested_control(array1, array2, array3, size);
                break;
                
            default:
                /* Multiple clauses and private variables */
                target_multi_clause(array1, size, config * 10);
                break;
        }
        
        /* Force synchronization and verification */
        #pragma omp target update from(array1[0:size])
        
        /* Compute checksum to ensure computation isn't optimized away */
        float checksum = 0.0f;
        for (int i = 0; i < size; ++i) {
            checksum += array1[i];
        }
        printf("Config %d, Size %d: Checksum = %f\n", config, size, checksum);
    }
    
    /* Additional test with dynamic scheduling hint */
    #pragma omp target teams distribute parallel for \
        map(tofrom: array2[0:N]) schedule(static, 64)
    for (int i = 0; i < N; ++i) {
        array2[i] = array2[i] * array2[i];
    }
    
    /* Final verification */
    float final_sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        final_sum += array1[i] + array2[i] + array3[i];
    }
    printf("Final sum: %f\n", final_sum);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
