/* test_omp_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered block that generates
 * IFN_GOMP_USE_SIMT and restructures loops for GPU offloading.
 *
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -o test_omp_simt test_omp_simt_lowering.c
 * For coverage: add -fdump-tree-omplower to see the transformation
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void target_teams_distribute_parallel_for_simd(float *arr, int n) {
    /* Explicit simd clause to strongly hint at SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:n]) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        /* Simple vectorizable arithmetic */
        arr[i] = arr[i] * 3.14159f + 1.0f;
    }
}

__attribute__((noinline))
void target_teams_distribute_parallel_for_complex(float *data, int n, float threshold) {
    /* Teams distribute parallel for with conditional inside loop body
     * Creates more complex GIMPLE sequence for copy_gimple_seq_and_replace_locals */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) num_teams(4) num_threads(256)
    for (int i = 0; i < n; ++i) {
        if (data[i] > threshold) {
            /* Function call inside loop to increase complexity */
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_nested_parallelism(float *a, float *b, float *c, int n) {
    /* Multiple map clauses for non-trivial data environment */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        defaultmap(tofrom:scalar)
    for (int i = 0; i < n; ++i) {
        /* Independent computations suitable for SIMD */
        float temp = a[i] + b[i];
        c[i] = temp * temp - 2.0f * a[i] * b[i];
    }
}

/* Helper function to verify results */
float verify_sum(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    int size = N;
    
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    if (argc > 2) {
        size = atoi(argv[2]);
        if (size <= 0 || size > 10000) size = N;
    }
    
    printf("Testing OpenMP SIMT lowering with test_case=%d, size=%d\n", 
           test_case, size);
    
    /* Allocate and initialize test arrays */
    float *arr1 = (float *)malloc(size * sizeof(float));
    float *arr2 = (float *)malloc(size * sizeof(float));
    float *arr3 = (float *)malloc(size * sizeof(float));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < size; ++i) {
        arr1[i] = (float)i;
        arr2[i] = (float)(size - i);
        arr3[i] = 0.0f;
    }
    
    /* Loop over different configurations to increase coverage chance */
    for (int iter = 0; iter < 3; ++iter) {
        printf("\nIteration %d:\n", iter);
        
        switch (test_case) {
            case 1:
                /* Case 1: Explicit SIMD clause */
                printf("Running target_teams_distribute_parallel_for_simd\n");
                target_teams_distribute_parallel_for_simd(arr1, size);
                printf("Checksum: %f\n", verify_sum(arr1, size));
                break;
                
            case 2:
                /* Case 2: Complex control flow in loop */
                printf("Running target_teams_distribute_parallel_for_complex\n");
                target_teams_distribute_parallel_for_complex(arr2, size, THRESHOLD);
                printf("Checksum: %f\n", verify_sum(arr2, size));
                break;
                
            default:
                /* Case 3: Multiple arrays with nested-like computation */
                printf("Running target_nested_parallelism\n");
                target_nested_parallelism(arr1, arr2, arr3, size);
                printf("Checksum: %f\n", verify_sum(arr3, size));
                break;
        }
        
        /* Rotate test cases if multiple iterations */
        test_case = (test_case % 3) + 1;
    }
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("\nTest completed successfully.\n");
    return 0;
}
