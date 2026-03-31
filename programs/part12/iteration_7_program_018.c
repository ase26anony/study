/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the uncovered lines in
 * omp-oacc-neuter-broadcast.cc (lines 335-343) by creating various
 * OpenACC compute regions with explicit data partitioning clauses.
 * The compiler's neuter-broadcast pass should process these clauses
 * and invoke the partition code to string mapping function.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Global arrays to test multi-dimensional data mapping */
int arr3d[N][M][P];
int arr2d[N][M];
int arr1d[N];

/* Function prototypes */
void test_gang_redundant(void);
void test_gang_partitioned(void);
void test_worker_partitioned(void);
void test_gang_worker_partitioned(void);
void test_vector_partitioned(void);
void test_gang_vector_partitioned(void);
void test_worker_vector_partitioned(void);
void test_fully_partitioned(void);
void test_nested_regions(void);
void test_routine_directives(void);
void test_persistent_data(void);

/* OpenACC routine with gang partitioning */
#pragma acc routine seq
int process_element(int val, int factor) {
    return val * factor;
}

/* OpenACC routine with vector partitioning */
#pragma acc routine vector
void vector_scale(int *arr, int factor, int n) {
    #pragma acc loop vector
    for (int i = 0; i < n; i++) {
        arr[i] = process_element(arr[i], factor);
    }
}

/* Test case 0: gang redundant */
void test_gang_redundant(void) {
    printf("Testing gang redundant partitioning...\n");
    
    #pragma acc parallel copy(arr1d[0:N]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1d[i] = i;
        }
    }
}

/* Test case 1: gang partitioned */
void test_gang_partitioned(void) {
    printf("Testing gang partitioned...\n");
    
    #pragma acc kernels create(arr2d[0:N][0:M]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop gang
            for (int j = 0; j < M; j++) {
                arr2d[i][j] = i * M + j;
            }
        }
    }
}

/* Test case 2: worker partitioned */
void test_worker_partitioned(void) {
    printf("Testing worker partitioned...\n");
    
    #pragma acc parallel copy(arr1d[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr1d[i] += 1;
        }
    }
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned(void) {
    printf("Testing gang+worker partitioned...\n");
    
    #pragma acc parallel copy(arr3d[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop worker
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] = i * M * P + j * P + k;
                }
            }
        }
    }
}

/* Test case 4: vector partitioned */
void test_vector_partitioned(void) {
    printf("Testing vector partitioned...\n");
    
    #pragma acc parallel copy(arr1d[0:N]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            arr1d[i] *= 2;
        }
    }
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned(void) {
    printf("Testing gang+vector partitioned...\n");
    
    #pragma acc kernels copy(arr2d[0:N][0:M]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr2d[i][j] = arr2d[i][j] * 3 + 1;
            }
        }
    }
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned(void) {
    printf("Testing worker+vector partitioned...\n");
    
    #pragma acc parallel copy(arr1d[0:N]) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            arr1d[i] = arr1d[i] / 2;
        }
    }
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(void) {
    printf("Testing fully partitioned...\n");
    
    #pragma acc parallel copy(arr3d[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] = arr3d[i][j][k] % 100;
                }
            }
        }
    }
}

/* Test nested compute regions with different partition types */
void test_nested_regions(void) {
    printf("Testing nested regions with conditional offloading...\n");
    
    int condition = 1;
    
    /* Outer region with gang partitioning */
    #pragma acc parallel if(condition) copy(arr2d[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Inner region with worker partitioning */
            #pragma acc parallel if(i % 2 == 0) present(arr2d) worker
            {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    arr2d[i][j] += i + j;
                }
            }
        }
    }
}

/* Test routine directives with partition propagation */
void test_routine_directives(void) {
    printf("Testing routine directives...\n");
    
    #pragma acc parallel copy(arr1d[0:N]) vector
    {
        /* This should invoke the vector-partitioned routine */
        vector_scale(arr1d, 2, N);
    }
}

/* Test persistent device data with partition clauses */
void test_persistent_data(void) {
    printf("Testing persistent device data...\n");
    
    /* Establish device data region with gang partitioning */
    #pragma acc enter data copyin(arr3d[0:N][0:M][0:P]) gang
    
    /* Multiple compute regions accessing the same partitioned data */
    #pragma acc parallel present(arr3d) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] += 1;
                }
            }
        }
    }
    
    #pragma acc parallel present(arr3d) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr3d[i][j][k] *= 2;
                }
            }
        }
    }
    
    /* Retrieve data from device */
    #pragma acc exit data copyout(arr3d) gang
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1d[i] = 0;
        for (int j = 0; j < M; j++) {
            arr2d[i][j] = 0;
            for (int k = 0; k < P; k++) {
                arr3d[i][j][k] = 0;
            }
        }
    }
    
    /* Use argc to create conditional execution paths
     * This prevents dead code elimination */
    if (argc > 1) {
        /* Execute all test cases */
        test_gang_redundant();
        test_gang_partitioned();
        test_worker_partitioned();
        test_gang_worker_partitioned();
        test_vector_partitioned();
        test_gang_vector_partitioned();
        test_worker_vector_partitioned();
        test_fully_partitioned();
    } else {
        /* Execute a subset to test different code paths */
        test_gang_redundant();
        test_fully_partitioned();
    }
    
    /* Additional tests based on random input to ensure coverage */
    if (argc > 2) {
        test_nested_regions();
        test_routine_directives();
        test_persistent_data();
    }
    
    /* Simple validation to ensure computations occurred */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr1d[i];
    }
    printf("Final array sum: %d\n", sum);
    
    return 0;
}
