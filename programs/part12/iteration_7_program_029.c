/* test_openacc_partitions.c - Comprehensive test for OpenACC partition type coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 64
#define M 32
#define P 16

/* Function prototypes */
void test_gang_redundant(int n, int m, int p, int arr[n][m][p]);
void test_gang_partitioned(int n, int m, int p, int arr[n][m][p]);
void test_worker_partitioned(int n, int m, int p, int arr[n][m][p]);
void test_gang_worker_partitioned(int n, int m, int p, int arr[n][m][p]);
void test_vector_partitioned(int n, int m, int p, int arr[n][m][p]);
void test_gang_vector_partitioned(int n, int m, int p, int arr[n][m][p]);
void test_worker_vector_partitioned(int n, int m, int p, int arr[n][m][p]);
void test_fully_partitioned(int n, int m, int p, int arr[n][m][p]);
void test_nested_regions(int n, int m, int p, int arr[n][m][p]);
void test_persistent_data(int n, int m, int p, int arr[n][m][p]);

/* ACC routine with gang partition */
#pragma acc routine vec gang
void acc_routine_gang(int *val) {
    *val += 1;
}

/* ACC routine with vector partition */
#pragma acc routine seq vector
void acc_routine_vector(int *val) {
    *val *= 2;
}

/* ACC routine with worker partition */
#pragma acc routine worker
void acc_routine_worker(int *val) {
    *val -= 1;
}

/* Test 1: Gang redundant partitioning */
void test_gang_redundant(int n, int m, int p, int arr[n][m][p]) {
    int condition = (n > 0);
    
    #pragma acc parallel if(condition) copy(arr[0:n][0:m][0:p]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang
            for (int j = 0; j < m; j++) {
                #pragma acc loop gang
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] = i + j + k;
                }
            }
        }
    }
}

/* Test 2: Gang partitioned */
void test_gang_partitioned(int n, int m, int p, int arr[n][m][p]) {
    #pragma acc kernels create(arr[0:n][0:m][0:p]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] += i * j * k;
                }
            }
        }
    }
}

/* Test 3: Worker partitioned */
void test_worker_partitioned(int n, int m, int p, int arr[n][m][p]) {
    #pragma acc parallel copy(arr[0:n][0:m][0:p]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                #pragma acc loop worker
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 4: Gang+worker partitioned */
void test_gang_worker_partitioned(int n, int m, int p, int arr[n][m][p]) {
    #pragma acc parallel copy(arr[0:n][0:m][0:p]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop gang worker
            for (int j = 0; j < m; j++) {
                #pragma acc loop gang worker
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] *= 2;
                }
            }
        }
    }
}

/* Test 5: Vector partitioned */
void test_vector_partitioned(int n, int m, int p, int arr[n][m][p]) {
    #pragma acc kernels copy(arr[0:n][0:m][0:p]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                #pragma acc loop vector
                for (int k = 0; k < p; k++) {
                    acc_routine_vector(&arr[i][j][k]);
                }
            }
        }
    }
}

/* Test 6: Gang+vector partitioned */
void test_gang_vector_partitioned(int n, int m, int p, int arr[n][m][p]) {
    #pragma acc parallel copy(arr[0:n][0:m][0:p]) gang vector
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                #pragma acc loop gang vector
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] = arr[i][j][k] % 100;
                }
            }
        }
    }
}

/* Test 7: Worker+vector partitioned */
void test_worker_vector_partitioned(int n, int m, int p, int arr[n][m][p]) {
    #pragma acc kernels copy(arr[0:n][0:m][0:p]) worker vector
    {
        #pragma acc loop worker vector collapse(3)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] += k;
                }
            }
        }
    }
}

/* Test 8: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int n, int m, int p, int arr[n][m][p]) {
    #pragma acc parallel copy(arr[0:n][0:m][0:p]) gang worker vector
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] = i + j * 2 + k * 3;
                }
            }
        }
    }
}

/* Test 9: Nested regions with different partition types */
void test_nested_regions(int n, int m, int p, int arr[n][m][p]) {
    int condition = (n > 10);
    
    /* Outer region with gang partitioning */
    #pragma acc parallel if(condition) copy(arr[0:n][0:m][0:p]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            /* Inner region with worker partitioning */
            #pragma acc parallel present(arr) worker
            {
                #pragma acc loop worker
                for (int j = 0; j < m; j++) {
                    #pragma acc loop worker
                    for (int k = 0; k < p; k++) {
                        acc_routine_worker(&arr[i][j][k]);
                    }
                }
            }
        }
    }
}

/* Test 10: Persistent device data with partition clauses */
void test_persistent_data(int n, int m, int p, int arr[n][m][p]) {
    /* Establish persistent device data region with gang partition */
    #pragma acc enter data copyin(arr[0:n][0:m][0:p]) gang
    
    /* Multiple compute regions accessing the same partitioned data */
    #pragma acc parallel present(arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] += 5;
                }
            }
        }
    }
    
    #pragma acc kernels present(arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                for (int k = 0; k < p; k++) {
                    arr[i][j][k] *= 3;
                }
            }
        }
    }
    
    /* Clean up persistent data */
    #pragma acc exit data copyout(arr[0:n][0:m][0:p])
}

/* Main function with conditional execution paths */
int main(int argc, char *argv[]) {
    int test_arr[N][M][P];
    int i, j, k;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                test_arr[i][j][k] = 0;
            }
        }
    }
    
    /* Use argc to create conditional execution paths */
    int test_case = argc > 1 ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(N, M, P, test_arr);
            break;
        case 1:
            test_gang_partitioned(N, M, P, test_arr);
            break;
        case 2:
            test_worker_partitioned(N, M, P, test_arr);
            break;
        case 3:
            test_gang_worker_partitioned(N, M, P, test_arr);
            break;
        case 4:
            test_vector_partitioned(N, M, P, test_arr);
            break;
        case 5:
            test_gang_vector_partitioned(N, M, P, test_arr);
            break;
        case 6:
            test_worker_vector_partitioned(N, M, P, test_arr);
            break;
        case 7:
            test_fully_partitioned(N, M, P, test_arr);
            break;
        case 8:
            test_nested_regions(N, M, P, test_arr);
            break;
        case 9:
            test_persistent_data(N, M, P, test_arr);
            break;
        default:
            /* Execute all tests sequentially */
            test_gang_redundant(N, M, P, test_arr);
            test_gang_partitioned(N, M, P, test_arr);
            test_worker_partitioned(N, M, P, test_arr);
            test_gang_worker_partitioned(N, M, P, test_arr);
            test_vector_partitioned(N, M, P, test_arr);
            test_gang_vector_partitioned(N, M, P, test_arr);
            test_worker_vector_partitioned(N, M, P, test_arr);
            test_fully_partitioned(N, M, P, test_arr);
            test_nested_regions(N, M, P, test_arr);
            test_persistent_data(N, M, P, test_arr);
            break;
    }
    
    /* Verify results (simple checksum) */
    int sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                sum += test_arr[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d\n", sum);
    return 0;
}
