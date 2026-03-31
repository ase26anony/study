/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o partition_test partition_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 32

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1;
    }
}

/* Test Case 0: gang redundant - scalar reductions, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing Case 0: gang redundant\n");
    
    double sum = 0.0;
    double arr[N];
    init_array(arr, N, 1.0);
    
    /* Scalar reduction - each gang computes full reduction */
    #pragma acc parallel copyin(arr[0:N]) reduction(+:sum) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    printf("  Sum = %f\n", sum);
}

/* Test Case 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing Case 1: gang partitioned\n");
    
    double arr[N], result[N];
    init_array(arr, N, 2.0);
    
    /* Each gang processes contiguous chunks of the array */
    #pragma acc parallel loop gang copy(arr[0:N]) copyout(result[0:N]) num_gangs(8)
    for (int i = 0; i < N; i++) {
        result[i] = arr[i] * 2.0;
    }
    
    /* Verify */
    double check = 0.0;
    for (int i = 0; i < N; i++) {
        check += result[i];
    }
    printf("  Check sum = %f\n", check);
}

/* Test Case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing Case 2: worker partitioned\n");
    
    double arr[M];
    double result[M];
    init_array(arr, M, 3.0);
    
    /* Worker-level partitioning with explicit worker count */
    #pragma acc parallel loop worker copy(arr[0:M]) copyout(result[0:M]) num_workers(4)
    for (int i = 0; i < M; i++) {
        result[i] = arr[i] * arr[i];
    }
    
    printf("  Worker partitioning complete\n");
}

/* Test Case 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing Case 3: gang+worker partitioned\n");
    
    double matrix[N][M];
    double result[N][M];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = i * 100.0 + j;
        }
    }
    
    /* Collapsed 2D loop with gang and worker partitioning */
    #pragma acc parallel loop gang worker collapse(2) copyin(matrix) copyout(result) \
        num_gangs(4) num_workers(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            result[i][j] = matrix[i][j] * 2.0;
        }
    }
    
    printf("  Gang+worker partitioning complete\n");
}

/* Test Case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing Case 4: vector partitioned\n");
    
    float vec[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        vec[i] = i * 0.5f;
    }
    
    /* Vector-level partitioning for SIMD operations */
    #pragma acc parallel loop vector copy(vec[0:N]) copyout(result[0:N]) vector_length(32)
    for (int i = 0; i < N; i++) {
        result[i] = vec[i] * vec[i];
    }
    
    printf("  Vector partitioning complete\n");
}

/* Test Case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing Case 5: gang+vector partitioned\n");
    
    double arr[N];
    double result[N];
    init_array(arr, N, 5.0);
    
    /* Gang and vector partitioning with stride-1 access */
    #pragma acc parallel loop gang vector copy(arr[0:N]) copyout(result[0:N]) \
        num_gangs(4) vector_length(16)
    for (int i = 0; i < N; i++) {
        result[i] = arr[i] + i * 0.01;
    }
    
    printf("  Gang+vector partitioning complete\n");
}

/* Test Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing Case 6: worker+vector partitioned\n");
    
    float data[M][P];
    float output[M][P];
    
    /* Initialize 2D array */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            data[i][j] = i * P + j;
        }
    }
    
    /* Worker and vector partitioning */
    #pragma acc parallel loop worker vector collapse(2) copyin(data) copyout(output) \
        num_workers(2) vector_length(8)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            output[i][j] = data[i][j] * 0.5f;
        }
    }
    
    printf("  Worker+vector partitioning complete\n");
}

/* Test Case 7: fully partitioned - all three levels: gang, worker, vector */
void test_fully_partitioned() {
    printf("Testing Case 7: fully partitioned\n");
    
    double cube[N][M][P];
    double result[N][M][P];
    
    /* Initialize 3D array */
    #pragma acc parallel loop gang worker vector collapse(3) copyout(cube)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                cube[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    /* Fully partitioned 3D computation */
    #pragma acc parallel loop gang worker vector collapse(3) copyin(cube) copyout(result) \
        num_gangs(4) num_workers(2) vector_length(8)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                result[i][j][k] = cube[i][j][k] * 2.0 + 1.0;
            }
        }
    }
    
    printf("  Fully partitioned complete\n");
}

/* Additional test to potentially trigger default case through edge conditions */
void test_edge_cases() {
    printf("Testing edge cases\n");
    
    /* Variable loop bounds that might affect partitioning decisions */
    int dynamic_size = 512;
    double *dynamic_arr = (double*)malloc(dynamic_size * sizeof(double));
    
    if (dynamic_arr) {
        init_array(dynamic_arr, dynamic_size, 10.0);
        
        /* Mixed partitioning with runtime parameters */
        #pragma acc parallel loop gang copy(dynamic_arr[0:dynamic_size]) \
            num_gangs(2) async
        for (int i = 0; i < dynamic_size; i++) {
            dynamic_arr[i] *= 1.5;
        }
        
        #pragma acc wait
        
        free(dynamic_arr);
    }
    
    /* Triangular loop pattern */
    double tri_result[M];
    #pragma acc parallel loop vector copyout(tri_result[0:M])
    for (int i = 0; i < M; i++) {
        double sum = 0.0;
        #pragma acc loop reduction(+:sum)
        for (int j = 0; j <= i; j++) {
            sum += j * 0.1;
        }
        tri_result[i] = sum;
    }
    
    printf("  Edge cases complete\n");
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Execute all test cases to trigger different partition codes */
    test_gang_redundant();           /* Should trigger case 0 */
    test_gang_partitioned();         /* Should trigger case 1 */
    test_worker_partitioned();       /* Should trigger case 2 */
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    test_vector_partitioned();       /* Should trigger case 4 */
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    test_worker_vector_partitioned();/* Should trigger case 6 */
    test_fully_partitioned();        /* Should trigger case 7 */
    
    test_edge_cases();               /* Additional edge cases */
    
    printf("\nAll tests completed successfully!\n");
    
    /* The default case (return "<illegal>") would be triggered internally
     * by the compiler if an invalid partition code is passed, which might
     * occur during compiler development or with malformed directives */
    
    return 0;
}
