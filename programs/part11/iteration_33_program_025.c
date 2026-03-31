/* test_oacc_partition.c
 * 
 * This program exercises OpenACC partitioning logic to cover the
 * switch cases in omp-oacc-neuter-broadcast.cc lines 335-343.
 * Each test function targets a specific partitioning pattern.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_1D 1024
#define SIZE_2D 32
#define SIZE_3D 16

/* Test 1: Gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1)
 */
void test_gang_redundant(float *data, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(data[0:n]) copyin(n) reduction(+:sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            data[i] = i * 0.5f;
            sum += data[i];
        }
    }
    
    printf("Gang redundant: sum = %f, data[0] = %f, data[%d] = %f\n", 
           sum, data[0], n-1, data[n-1]);
}

/* Test 2: Gang partitioned (case 1)
 * Outer loop explicitly marked as gang partitioned
 */
void test_gang_partitioned(float *data, int n) {
    float total = 0.0f;
    
    #pragma acc parallel copy(data[0:n]) copyin(n) reduction(+:total)
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            float private_val = i * 0.25f;
            data[i] = private_val * private_val;
            total += data[i];
        }
    }
    
    printf("Gang partitioned: total = %f, data[0] = %f, data[%d] = %f\n", 
           total, data[0], n-1, data[n-1]);
}

/* Test 3: Worker partitioned (case 2)
 * Inner loop marked as worker partitioned within nested loops
 */
void test_worker_partitioned(float data[][SIZE_2D], int rows, int cols) {
    #pragma acc parallel copy(data[0:rows][0:cols]) copyin(rows, cols)
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop worker
            for (int j = 0; j < cols; j++) {
                data[i][j] = (i + j) * 0.1f;
            }
        }
    }
    
    printf("Worker partitioned: data[0][0] = %f, data[%d][%d] = %f\n", 
           data[0][0], rows-1, cols-1, data[rows-1][cols-1]);
}

/* Test 4: Gang+worker partitioned (case 3)
 * Both gang and worker clauses on nested loops
 */
void test_gang_worker_partitioned(float data[][SIZE_2D], int rows, int cols) {
    float row_sums[SIZE_2D] = {0};
    
    #pragma acc parallel copy(data[0:rows][0:cols]) \
                         copy(row_sums[0:rows]) copyin(rows, cols)
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            float row_sum = 0.0f;
            #pragma acc loop worker reduction(+:row_sum)
            for (int j = 0; j < cols; j++) {
                data[i][j] = (i * cols + j) * 0.05f;
                row_sum += data[i][j];
            }
            row_sums[i] = row_sum;
        }
    }
    
    printf("Gang+worker partitioned: row_sums[0] = %f, row_sums[%d] = %f\n", 
           row_sums[0], rows-1, row_sums[rows-1]);
}

/* Test 5: Vector partitioned (case 4)
 * Loop marked for vector partitioning
 */
void test_vector_partitioned(float *data, int n) {
    #pragma acc parallel copy(data[0:n]) copyin(n)
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
        }
    }
    
    printf("Vector partitioned: data[0] = %f, data[%d] = %f\n", 
           data[0], n-1, data[n-1]);
}

/* Test 6: Gang+vector partitioned (case 5)
 * Combined gang and vector partitioning
 */
void test_gang_vector_partitioned(float *data, int n) {
    #pragma acc parallel copy(data[0:n]) copyin(n)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            data[i] = (data[i] - (float)i) * 0.5f;
        }
    }
    
    printf("Gang+vector partitioned: data[0] = %f, data[%d] = %f\n", 
           data[0], n-1, data[n-1]);
}

/* Test 7: Worker+vector partitioned (case 6)
 * Combined worker and vector partitioning
 */
void test_worker_vector_partitioned(float data[][SIZE_2D], int rows, int cols) {
    #pragma acc parallel copy(data[0:rows][0:cols]) copyin(rows, cols)
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < cols; j++) {
                data[i][j] = data[i][j] * data[i][j];
            }
        }
    }
    
    printf("Worker+vector partitioned: data[0][0] = %f, data[%d][%d] = %f\n", 
           data[0][0], rows-1, cols-1, data[rows-1][cols-1]);
}

/* Test 8: Fully partitioned (case 7)
 * Triple-nested loop with explicit gang, worker, and vector clauses
 * Performing a stencil-like computation
 */
void test_fully_partitioned(float data[][SIZE_2D][SIZE_3D], 
                           int x, int y, int z) {
    float temp[SIZE_2D][SIZE_2D][SIZE_3D];
    
    // Initialize temp array
    #pragma acc parallel copy(temp[0:x][0:y][0:z]) copyin(x, y, z)
    {
        #pragma acc loop gang
        for (int i = 0; i < x; i++) {
            #pragma acc loop worker
            for (int j = 0; j < y; j++) {
                #pragma acc loop vector
                for (int k = 0; k < z; k++) {
                    temp[i][j][k] = (i + j + k) * 0.01f;
                }
            }
        }
    }
    
    // Stencil computation with full partitioning
    #pragma acc parallel copy(data[0:x][0:y][0:z]) \
                         copyin(temp[0:x][0:y][0:z], x, y, z)
    {
        #pragma acc loop gang
        for (int i = 1; i < x - 1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < y - 1; j++) {
                #pragma acc loop vector
                for (int k = 1; k < z - 1; k++) {
                    // Simple 3D stencil
                    data[i][j][k] = (temp[i-1][j][k] + 
                                     temp[i][j-1][k] + 
                                     temp[i][j][k-1]) * 0.333f;
                }
            }
        }
    }
    
    printf("Fully partitioned: data[1][1][1] = %f, data[%d][%d][%d] = %f\n", 
           data[1][1][1], x-2, y-2, z-2, data[x-2][y-2][z-2]);
}

/* Main driver that conditionally executes test functions
 * based on command-line arguments to ensure all code paths
 * are compiled and considered by the compiler.
 */
int main(int argc, char *argv[]) {
    // Initialize test data
    float data_1d[SIZE_1D];
    float data_2d[SIZE_2D][SIZE_2D];
    float data_3d[SIZE_2D][SIZE_2D][SIZE_3D];
    
    // Simple initialization
    for (int i = 0; i < SIZE_1D; i++) {
        data_1d[i] = (float)i;
    }
    
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            data_2d[i][j] = (float)(i * SIZE_2D + j);
        }
    }
    
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            for (int k = 0; k < SIZE_3D; k++) {
                data_3d[i][j][k] = (float)(i * SIZE_2D * SIZE_3D + 
                                           j * SIZE_3D + k);
            }
        }
    }
    
    // Use argc to control execution, ensuring compiler analyzes all paths
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 9;  // 0-8 for our 8 cases + default
    }
    
    // Force compiler to consider all switch cases through conditional execution
    switch (test_case) {
        case 0:
            test_gang_redundant(data_1d, SIZE_1D);
            break;
        case 1:
            test_gang_partitioned(data_1d, SIZE_1D);
            break;
        case 2:
            test_worker_partitioned(data_2d, SIZE_2D, SIZE_2D);
            break;
        case 3:
            test_gang_worker_partitioned(data_2d, SIZE_2D, SIZE_2D);
            break;
        case 4:
            test_vector_partitioned(data_1d, SIZE_1D);
            break;
        case 5:
            test_gang_vector_partitioned(data_1d, SIZE_1D);
            break;
        case 6:
            test_worker_vector_partitioned(data_2d, SIZE_2D, SIZE_2D);
            break;
        case 7:
            test_fully_partitioned(data_3d, SIZE_2D, SIZE_2D, SIZE_3D);
            break;
        default:
            // Execute all tests when no specific case is requested
            // This ensures all OpenACC constructs are processed
            if (argc > 2) {
                test_gang_redundant(data_1d, SIZE_1D);
                test_gang_partitioned(data_1d, SIZE_1D);
                test_worker_partitioned(data_2d, SIZE_2D, SIZE_2D);
                test_gang_worker_partitioned(data_2d, SIZE_2D, SIZE_2D);
                test_vector_partitioned(data_1d, SIZE_1D);
                test_gang_vector_partitioned(data_1d, SIZE_1D);
                test_worker_vector_partitioned(data_2d, SIZE_2D, SIZE_2D);
                test_fully_partitioned(data_3d, SIZE_2D, SIZE_2D, SIZE_3D);
            }
            printf("Default case executed (all tests potentially compiled)\n");
            break;
    }
    
    // Print final values to prevent dead code elimination
    printf("Final check - data_1d[0] = %f, data_2d[0][0] = %f, data_3d[0][0][0] = %f\n",
           data_1d[0], data_2d[0][0], data_3d[0][0][0]);
    
    return 0;
}
