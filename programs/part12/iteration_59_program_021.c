/* Test program to cover all partitioning states in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning(int *result) {
    int i, j, k;
    
    /* Variables that should get different partitioning states */
    int scalar_private;           /* Likely gang redundant (0) */
    int scalar_firstprivate = 42; /* Likely gang redundant (0) */
    int scalar_reduction = 0;     /* Reduction variable */
    
    /* 1D arrays with different mappings */
    int arr1[N];                  /* copy - may be gang partitioned (1) */
    int arr2[N];                  /* copyin - may be worker partitioned (2) */
    int arr3[N];                  /* copyout - may be gang+worker partitioned (3) */
    int arr4[N];                  /* create - may be vector partitioned (4) */
    
    /* Pattern B: Multi-dimensional arrays */
    int arr_multi[M][N];          /* May trigger complex partitioning */
    
    /* Pattern C: Pointer-based dynamic memory */
    int *dyn_arr = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        arr4[i] = i * 3;
        dyn_arr[i] = i * 4;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            arr_multi[i][j] = i * N + j;
        }
    }
    
    /* OpenACC parallel region with complex data clauses and nested loops */
    #pragma acc parallel loop copy(arr1[0:N]) copyin(arr2[0:N]) \
                copyout(arr3[0:N]) create(arr4[0:N]) \
                copyin(arr_multi[0:M][0:N]) copyin(dyn_arr[0:N]) \
                private(scalar_private) firstprivate(scalar_firstprivate) \
                reduction(+:scalar_reduction)
    for (i = 0; i < N; i++) {
        /* Access scalar with different scopes */
        scalar_private = i;
        
        /* Pattern D: Struct-like access through separate arrays */
        int temp = arr1[i] + arr2[i];
        
        /* Nested loops to create complex data flow */
        for (j = 0; j < 8; j++) {
            /* Conditional operations */
            if (j % 2 == 0) {
                arr3[i] += temp * j;
            } else {
                arr3[i] -= temp * j;
            }
            
            /* Access multi-dimensional array with varying indices */
            for (k = 0; k < 4; k++) {
                arr4[i] += arr_multi[j % M][(i + k) % N] / (k + 1);
            }
        }
        
        /* Access dynamic memory */
        arr3[i] += dyn_arr[i];
        
        /* Reduction operation */
        scalar_reduction += arr3[i] % 100;
    }
    
    /* Second kernel with different partitioning */
    #pragma acc parallel loop gang worker vector \
                copy(arr1[0:N]) copy(arr3[0:N]) \
                private(scalar_private)
    for (i = 0; i < N; i++) {
        /* Vector-level operations */
        int vec_temp = 0;
        #pragma acc loop vector
        for (j = 0; j < 16; j++) {
            vec_temp += arr1[(i + j) % N];
        }
        arr3[i] = vec_temp / 16;
    }
    
    /* Third kernel with explicit gang/worker/vector partitioning hints */
    #pragma acc parallel loop gang(N/256) worker(4) vector(32) \
                copy(arr1[0:N]) copy(arr2[0:N]) copy(arr3[0:N]) \
                copy(arr4[0:N])
    for (i = 0; i < N; i++) {
        /* Complex nested loops accessing all arrays */
        int gang_part = 0, worker_part = 0, vector_part = 0;
        
        #pragma acc loop worker
        for (j = 0; j < 4; j++) {
            worker_part += arr2[(i + j) % N];
            
            #pragma acc loop vector
            for (k = 0; k < 8; k++) {
                vector_part += arr4[(i + j + k) % N];
            }
        }
        
        arr1[i] = worker_part + vector_part;
    }
    
    /* Compute final result */
    result[0] = scalar_reduction;
    for (i = 0; i < N; i++) {
        result[0] += arr3[i];
    }
    
    free(dyn_arr);
}

/* Additional test with OpenMP to trigger different code paths */
void test_openmp_partitioning(int *result) {
    int i, j;
    int arr_a[N], arr_b[N], arr_c[N];
    int scalar_local = 5;
    int reduction_var = 0;
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        arr_a[i] = i;
        arr_b[i] = i * 2;
        arr_c[i] = 0;
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
                map(to: arr_a[0:N], arr_b[0:N]) \
                map(from: arr_c[0:N]) \
                private(scalar_local) \
                reduction(+:reduction_var) \
                num_teams(4) num_threads(128)
    for (i = 0; i < N; i++) {
        scalar_local = i % 10;
        
        /* Nested loop with conditional */
        for (j = 0; j < scalar_local; j++) {
            if (j % 3 == 0) {
                arr_c[i] += arr_a[i] * arr_b[(i + j) % N];
            } else {
                arr_c[i] -= arr_a[i] / (arr_b[(i + j) % N] + 1);
            }
        }
        
        reduction_var += arr_c[i] % 100;
    }
    
    result[1] = reduction_var;
    
    /* Another OpenMP region with collapse */
    #pragma omp target teams distribute parallel for collapse(2) \
                map(tofrom: arr_a[0:N], arr_b[0:N]) \
                map(to: arr_c[0:N])
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            int idx = i * 32 + j;
            if (idx < N) {
                arr_a[idx] = arr_b[idx] + arr_c[idx];
                arr_b[idx] = arr_a[idx] * 2;
            }
        }
    }
    
    /* Final reduction */
    int final_sum = 0;
    #pragma omp target teams distribute parallel for \
                map(to: arr_a[0:N]) \
                reduction(+:final_sum)
    for (i = 0; i < N; i++) {
        final_sum += arr_a[i];
    }
    
    result[2] = final_sum;
}

/* Test with struct/class-like data (C version) */
struct DataPoint {
    int x;
    int y;
    int z;
    double value;
};

void test_struct_partitioning(int *result) {
    int i;
    struct DataPoint points[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
        points[i].value = i * 0.5;
    }
    
    int sum = 0;
    
    /* OpenACC with struct array */
    #pragma acc parallel loop copy(points[0:N]) reduction(+:sum)
    for (i = 0; i < N; i++) {
        /* Access different struct members */
        points[i].value = points[i].x + points[i].y + points[i].z;
        
        /* Conditional based on struct values */
        if (points[i].x % 2 == 0) {
            points[i].z = points[i].y * 3;
        } else {
            points[i].z = points[i].y / 2;
        }
        
        sum += points[i].z;
    }
    
    result[3] = sum;
}

/* Main function that runs all tests */
int main() {
    int results[4] = {0};
    int expected[4] = {0};
    int i;
    
    /* Calculate expected values for verification */
    int test_sum = 0;
    for (i = 0; i < N; i++) {
        test_sum += i % 100;
    }
    expected[0] = test_sum * 2;  /* Approximate expected */
    expected[1] = test_sum;      /* Approximate expected */
    expected[2] = N * (N - 1) / 2; /* Sum of 0..N-1 */
    expected[3] = 0;             /* Will be calculated */
    
    /* Run all partitioning tests */
    test_openacc_partitioning(results);
    test_openmp_partitioning(results);
    test_struct_partitioning(results);
    
    /* Quick verification (not exhaustive, just ensures code runs) */
    printf("Test results: %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3]);
    
    /* Check that results are non-zero and reasonable */
    int all_ok = 1;
    for (i = 0; i < 4; i++) {
        if (results[i] == 0) {
            printf("Warning: result[%d] is zero\n", i);
            all_ok = 0;
        }
    }
    
    if (all_ok) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Some tests produced unexpected results.\n");
        return 1;
    }
}
