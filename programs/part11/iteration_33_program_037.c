/* test_oacc_partition.c
 * 
 * This program exercises various OpenACC partitioning patterns to trigger
 * the GCC internal partitioning classification logic in omp-oacc-neuter-broadcast.cc.
 * Each test function targets a specific partitioning case.
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE_1D 1024
#define SIZE_2D 32
#define SIZE_3D 16

/* Test 1: Gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1) */
void test_gang_redundant(int *result) {
    int local_sum = 0;
    
    #pragma acc parallel copy(local_sum) num_gangs(1)
    {
        local_sum = 42;  // Simple assignment in gang-redundant region
    }
    
    *result = local_sum;
}

/* Test 2: Gang partitioned (case 1)
 * Single loop with explicit gang partitioning */
void test_gang_partitioned(float *arr) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(arr[0:SIZE_1D]) reduction(+:sum)
    for (int i = 0; i < SIZE_1D; i++) {
        arr[i] = (float)i * 0.5f;
        sum += arr[i];
    }
    
    // Use result to prevent dead code elimination
    arr[0] = sum / SIZE_1D;
}

/* Test 3: Worker partitioned (case 2)
 * Inner loop with worker partitioning within nested loops */
void test_worker_partitioned(float arr[SIZE_2D][SIZE_2D]) {
    #pragma acc parallel copy(arr[0:SIZE_2D][0:SIZE_2D])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE_2D; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE_2D; j++) {
                arr[i][j] = (float)(i * SIZE_2D + j) * 0.25f;
            }
        }
    }
}

/* Test 4: Vector partitioned (case 4)
 * Loop with explicit vector partitioning for element-wise operations */
void test_vector_partitioned(float *arr1, float *arr2) {
    #pragma acc parallel loop vector copy(arr1[0:SIZE_1D], arr2[0:SIZE_1D])
    for (int i = 0; i < SIZE_1D; i++) {
        arr1[i] = (float)i * 1.5f;
        arr2[i] = arr1[i] * 2.0f;
    }
}

/* Test 5: Gang+worker partitioned (case 3)
 * Nested loops with gang and worker partitioning */
void test_gang_worker_partitioned(float arr[SIZE_2D][SIZE_2D]) {
    float tile_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:SIZE_2D][0:SIZE_2D]) reduction(+:tile_sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE_2D; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE_2D; j++) {
                arr[i][j] = (float)(i + j) * 0.75f;
                tile_sum += arr[i][j];
            }
        }
    }
    
    arr[0][0] = tile_sum;
}

/* Test 6: Gang+vector partitioned (case 5)
 * Nested loops with gang and vector partitioning */
void test_gang_vector_partitioned(float arr[SIZE_2D][SIZE_2D]) {
    #pragma acc parallel copy(arr[0:SIZE_2D][0:SIZE_2D])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE_2D; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE_2D; j++) {
                arr[i][j] = (float)(i * j) * 0.33f;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6)
 * Nested loops with worker and vector partitioning */
void test_worker_vector_partitioned(float arr[SIZE_2D][SIZE_2D]) {
    #pragma acc parallel copy(arr[0:SIZE_2D][0:SIZE_2D])
    {
        #pragma acc loop gang
        for (int block = 0; block < 4; block++) {
            #pragma acc loop worker vector
            for (int idx = 0; idx < SIZE_2D * SIZE_2D / 4; idx++) {
                int i = (block * SIZE_2D / 4) + (idx / SIZE_2D);
                int j = idx % SIZE_2D;
                if (i < SIZE_2D) {
                    arr[i][j] = (float)(i ^ j) * 0.67f;
                }
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7)
 * Triple-nested loop with gang, worker, and vector partitioning */
void test_fully_partitioned(float arr[SIZE_3D][SIZE_3D][SIZE_3D]) {
    // Initialize array
    #pragma acc parallel loop gang collapse(2) copy(arr[0:SIZE_3D][0:SIZE_3D][0:SIZE_3D])
    for (int i = 0; i < SIZE_3D; i++) {
        for (int j = 0; j < SIZE_3D; j++) {
            #pragma acc loop vector
            for (int k = 0; k < SIZE_3D; k++) {
                arr[i][j][k] = (float)(i + j + k);
            }
        }
    }
    
    // Stencil computation with full partitioning
    float temp[SIZE_3D][SIZE_3D][SIZE_3D];
    
    #pragma acc parallel copy(arr[0:SIZE_3D][0:SIZE_3D][0:SIZE_3D]) \
                         create(temp[0:SIZE_3D][0:SIZE_3D][0:SIZE_3D])
    {
        #pragma acc loop gang
        for (int i = 1; i < SIZE_3D - 1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < SIZE_3D - 1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < SIZE_3D - 1; k++) {
                    // 3D stencil computation
                    temp[i][j][k] = (arr[i-1][j][k] + arr[i+1][j][k] +
                                     arr[i][j-1][k] + arr[i][j+1][k] +
                                     arr[i][j][k-1] + arr[i][j][k+1]) * 0.1666667f;
                }
            }
        }
        
        // Copy back with different partitioning
        #pragma acc loop gang
        for (int i = 1; i < SIZE_3D - 1; i++) {
            #pragma acc loop worker vector
            for (int j = 1; j < SIZE_3D - 1; j++) {
                for (int k = 1; k < SIZE_3D - 1; k++) {
                    arr[i][j][k] = temp[i][j][k];
                }
            }
        }
    }
}

/* Test 9: Mixed partitioning with runtime condition
 * Forces compiler to analyze both OpenACC and host-fallback paths */
void test_mixed_partitioned(int argc, float *arr) {
    int use_gang = argc > 1;
    int use_vector = argc > 2;
    
    #pragma acc parallel copy(arr[0:SIZE_1D]) if(use_gang)
    {
        if (use_gang) {
            #pragma acc loop gang
            for (int i = 0; i < SIZE_1D; i++) {
                arr[i] = arr[i] * 2.0f;
            }
        } else {
            #pragma acc loop
            for (int i = 0; i < SIZE_1D; i++) {
                arr[i] = arr[i] * 1.5f;
            }
        }
    }
    
    if (use_vector) {
        #pragma acc parallel loop vector copy(arr[0:SIZE_1D])
        for (int i = 0; i < SIZE_1D; i++) {
            arr[i] = arr[i] + (float)i;
        }
    }
}

int main(int argc, char *argv[]) {
    // Allocate and initialize test arrays
    float *arr1d = (float *)malloc(SIZE_1D * sizeof(float));
    float *arr1d_2 = (float *)malloc(SIZE_1D * sizeof(float));
    float (*arr2d)[SIZE_2D] = (float (*)[SIZE_2D])malloc(SIZE_2D * SIZE_2D * sizeof(float));
    float (*arr3d)[SIZE_3D][SIZE_3D] = (float (*)[SIZE_3D][SIZE_3D])malloc(SIZE_3D * SIZE_3D * SIZE_3D * sizeof(float));
    
    for (int i = 0; i < SIZE_1D; i++) {
        arr1d[i] = 1.0f;
        arr1d_2[i] = 2.0f;
    }
    
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            arr2d[i][j] = (float)(i + j);
        }
    }
    
    for (int i = 0; i < SIZE_3D; i++) {
        for (int j = 0; j < SIZE_3D; j++) {
            for (int k = 0; k < SIZE_3D; k++) {
                arr3d[i][j][k] = 0.0f;
            }
        }
    }
    
    int test_result = 0;
    
    // Execute tests based on command-line arguments
    // Using argc to ensure all code paths are compiled
    int test_to_run = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_to_run) {
        case 0:
            test_gang_redundant(&test_result);
            printf("Test 0 (gang redundant): result = %d\n", test_result);
            break;
        case 1:
            test_gang_partitioned(arr1d);
            printf("Test 1 (gang partitioned): arr[0] = %f, arr[%d] = %f\n", 
                   arr1d[0], SIZE_1D-1, arr1d[SIZE_1D-1]);
            break;
        case 2:
            test_worker_partitioned(arr2d);
            printf("Test 2 (worker partitioned): arr[0][0] = %f, arr[%d][%d] = %f\n",
                   arr2d[0][0], SIZE_2D-1, SIZE_2D-1, arr2d[SIZE_2D-1][SIZE_2D-1]);
            break;
        case 3:
            test_vector_partitioned(arr1d, arr1d_2);
            printf("Test 3 (vector partitioned): arr1[0] = %f, arr2[%d] = %f\n",
                   arr1d[0], SIZE_1D-1, arr1d_2[SIZE_1D-1]);
            break;
        case 4:
            test_gang_worker_partitioned(arr2d);
            printf("Test 4 (gang+worker partitioned): arr[0][0] = %f\n", arr2d[0][0]);
            break;
        case 5:
            test_gang_vector_partitioned(arr2d);
            printf("Test 5 (gang+vector partitioned): arr[0][0] = %f\n", arr2d[0][0]);
            break;
        case 6:
            test_worker_vector_partitioned(arr2d);
            printf("Test 6 (worker+vector partitioned): arr[0][0] = %f\n", arr2d[0][0]);
            break;
        case 7:
            test_fully_partitioned(arr3d);
            printf("Test 7 (fully partitioned): arr[1][1][1] = %f\n", arr3d[1][1][1]);
            break;
        default:
            // Run all tests to ensure all OpenACC constructs are processed
            test_gang_redundant(&test_result);
            test_gang_partitioned(arr1d);
            test_worker_partitioned(arr2d);
            test_vector_partitioned(arr1d, arr1d_2);
            test_gang_worker_partitioned(arr2d);
            test_gang_vector_partitioned(arr2d);
            test_worker_vector_partitioned(arr2d);
            test_fully_partitioned(arr3d);
            test_mixed_partitioned(argc, arr1d);
            
            printf("All tests compiled and executed. Sample values:\n");
            printf("  arr1d[0] = %f\n", arr1d[0]);
            printf("  arr2d[0][0] = %f\n", arr2d[0][0]);
            printf("  arr3d[1][1][1] = %f\n", arr3d[1][1][1]);
            break;
    }
    
    // Cleanup
    free(arr1d);
    free(arr1d_2);
    free(arr2d);
    free(arr3d);
    
    return 0;
}
