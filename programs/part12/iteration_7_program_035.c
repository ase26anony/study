/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the partition string mapping logic
 * in GCC's OpenACC neuter-broadcast pass (lines 335-343 of omp-oacc-neuter-broadcast.cc).
 * It uses various OpenACC compute constructs with explicit data partitioning
 * across gang, worker, and vector dimensions.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Helper function to initialize 3D array */
void init_3d_array(int arr[N][M][P]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * 1000 + j * 100 + k;
            }
        }
    }
}

/* Helper function to verify array was modified */
int verify_3d_array(int arr[N][M][P], int expected_increment) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                int expected = i * 1000 + j * 100 + k + expected_increment;
                if (arr[i][j][k] != expected) {
                    errors++;
                    if (errors < 5) {
                        printf("Error at [%d][%d][%d]: got %d, expected %d\n",
                               i, j, k, arr[i][j][k], expected);
                    }
                }
            }
        }
    }
    return errors;
}

/* ACC routine with explicit gang partitioning */
#pragma acc routine seq
void increment_element(int *elem) {
    *elem += 1;
}

/* ACC routine with vector partitioning */
#pragma acc routine vec
void increment_element_vec(int *elem) {
    *elem += 1;
}

/* Test 1: Basic partition combinations in parallel regions */
void test_basic_partitions(int arr[N][M][P], int use_parallel) {
    if (use_parallel) {
        /* gang redundant (implicit) */
        #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < P; k++) {
                        arr[i][j][k] += 1;
                    }
                }
            }
        }
        
        /* gang partitioned */
        #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang(static:2)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr[i][j][k] += 1;
                    }
                }
            }
        }
        
        /* worker partitioned */
        #pragma acc parallel copy(arr[0:N][0:M][0:P]) worker
        {
            #pragma acc loop worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr[i][j][k] += 1;
                    }
                }
            }
        }
    } else {
        /* Use kernels for different partition types */
        
        /* gang+worker partitioned */
        #pragma acc kernels copy(arr[0:N][0:M][0:P]) gang worker
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr[i][j][k] += 1;
                    }
                }
            }
        }
        
        /* vector partitioned */
        #pragma acc kernels copy(arr[0:N][0:M][0:P]) vector
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        arr[i][j][k] += 1;
                    }
                }
            }
        }
    }
}

/* Test 2: Multi-dimensional array with collapse and complex partitioning */
void test_collapse_partitions(int arr[N][M][P], int argc) {
    /* gang+vector partitioned with collapse */
    #pragma acc parallel loop collapse(3) gang vector copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] += 1;
            }
        }
    }
    
    /* worker+vector partitioned with conditional execution */
    if (argc > 1) {
        #pragma acc kernels loop collapse(2) worker vector copy(arr[0:N][0:M][0:P])
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                #pragma acc loop seq
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 3: Nested compute regions with different partitions */
void test_nested_regions(int arr[N][M][P]) {
    /* Outer region with gang partitioning */
    #pragma acc parallel copy(arr[0:N][0:M][0:P]) gang
    {
        /* Inner region with worker partitioning */
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            #pragma acc parallel loop worker present(arr)
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
}

/* Test 4: Routine directives with partition propagation */
void test_routine_partitions(int arr[N][M][P]) {
    /* Fully partitioned region calling gang routine */
    #pragma acc parallel loop gang worker vector copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                increment_element(&arr[i][j][k]);
            }
        }
    }
    
    /* Vector partitioned region calling vector routine */
    #pragma acc parallel loop vector copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                increment_element_vec(&arr[i][j][k]);
            }
        }
    }
}

/* Test 5: Device data environment with partition clauses */
void test_device_data_partitions(int arr[N][M][P]) {
    /* Establish device data with gang partitioning */
    #pragma acc enter data copyin(arr[0:N][0:M][0:P]) gang
    
    /* Compute region with worker partitioning using present data */
    #pragma acc parallel present(arr) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Another region with vector partitioning */
    #pragma acc parallel present(arr) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] += 1;
                }
            }
        }
    }
    
    /* Clean up device data */
    #pragma acc exit data copyout(arr[0:N][0:M][0:P]) gang
}

int main(int argc, char *argv[]) {
    int array1[N][M][P];
    int array2[N][M][P];
    int total_errors = 0;
    
    /* Initialize arrays */
    init_3d_array(array1);
    init_3d_array(array2);
    
    printf("Testing OpenACC partition combinations...\n");
    
    /* Test 1: Basic partitions (use argc to vary execution path) */
    test_basic_partitions(array1, argc > 1);
    total_errors += verify_3d_array(array1, (argc > 1) ? 3 : 2);
    
    /* Test 2: Collapse with partitions */
    test_collapse_partitions(array1, argc);
    total_errors += verify_3d_array(array1, (argc > 1) ? 5 : 3);
    
    /* Test 3: Nested regions */
    test_nested_regions(array2);
    total_errors += verify_3d_array(array2, 1);
    
    /* Test 4: Routine directives */
    test_routine_partitions(array2);
    total_errors += verify_3d_array(array2, 3);
    
    /* Test 5: Device data environment */
    test_device_data_partitions(array2);
    total_errors += verify_3d_array(array2, 5);
    
    if (total_errors == 0) {
        printf("All tests passed (coverage goal: trigger partition string mapping)\n");
    } else {
        printf("Found %d errors in array validation\n", total_errors);
    }
    
    return total_errors > 0 ? 1 : 0;
}
