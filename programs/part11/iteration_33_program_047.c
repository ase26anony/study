/* test_oacc_partition.c
 * 
 * This program exercises GCC's OpenACC partitioning logic to trigger
 * the switch cases in omp-oacc-neuter-broadcast.cc lines 335-343.
 * Each test function targets a specific partitioning pattern.
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define DIM 32

/* Test 1: Gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1).
 */
void test_gang_redundant(int use_acc) {
    float scalar = 1.0f;
    float arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) arr[i] = (float)i;
    
    if (use_acc) {
        #pragma acc parallel copy(arr[0:SIZE]) copyin(scalar)
        {
            #pragma acc loop gang(static:1)
            for (int i = 0; i < SIZE; i++) {
                arr[i] = arr[i] + scalar;
            }
        }
    } else {
        for (int i = 0; i < SIZE; i++) arr[i] = arr[i] + scalar;
    }
    
    printf("Gang redundant: arr[0]=%.1f, arr[%d]=%.1f\n", 
           arr[0], SIZE-1, arr[SIZE-1]);
}

/* Test 2: Gang partitioned (case 1)
 * Single loop with explicit gang clause.
 */
void test_gang_partitioned(int use_acc) {
    float data[SIZE];
    float sum = 0.0f;
    
    for (int i = 0; i < SIZE; i++) data[i] = (float)(i+1);
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:SIZE]) reduction(+:sum)
        {
            #pragma acc loop gang
            for (int i = 0; i < SIZE; i++) {
                data[i] = data[i] * 2.0f;
                sum += data[i];
            }
        }
    } else {
        for (int i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f;
            sum += data[i];
        }
    }
    
    printf("Gang partitioned: sum=%.1f, data[%d]=%.1f\n", 
           sum, SIZE-1, data[SIZE-1]);
}

/* Test 3: Worker partitioned (case 2)
 * Inner loop with worker clause inside nested loops.
 */
void test_worker_partitioned(int use_acc) {
    float matrix[DIM][DIM];
    
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            matrix[i][j] = (float)(i * DIM + j);
    
    if (use_acc) {
        #pragma acc parallel copy(matrix[0:DIM][0:DIM])
        {
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                #pragma acc loop worker
                for (int j = 0; j < DIM; j++) {
                    matrix[i][j] = matrix[i][j] * 0.5f;
                }
            }
        }
    } else {
        for (int i = 0; i < DIM; i++)
            for (int j = 0; j < DIM; j++)
                matrix[i][j] = matrix[i][j] * 0.5f;
    }
    
    printf("Worker partitioned: matrix[0][0]=%.1f, matrix[%d][%d]=%.1f\n",
           matrix[0][0], DIM-1, DIM-1, matrix[DIM-1][DIM-1]);
}

/* Test 4: Gang+worker partitioned (case 3)
 * Nested loops with gang and worker clauses.
 */
void test_gang_worker_partitioned(int use_acc) {
    float grid[DIM][DIM];
    
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            grid[i][j] = (float)(i + j);
    
    if (use_acc) {
        #pragma acc parallel copy(grid[0:DIM][0:DIM])
        {
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                #pragma acc loop worker
                for (int j = 0; j < DIM; j++) {
                    if (i > 0 && j > 0)
                        grid[i][j] = (grid[i-1][j] + grid[i][j-1]) * 0.5f;
                }
            }
        }
    } else {
        for (int i = 0; i < DIM; i++)
            for (int j = 0; j < DIM; j++)
                if (i > 0 && j > 0)
                    grid[i][j] = (grid[i-1][j] + grid[i][j-1]) * 0.5f;
    }
    
    printf("Gang+worker partitioned: grid[1][1]=%.1f\n", grid[1][1]);
}

/* Test 5: Vector partitioned (case 4)
 * Loop with vector clause for element-wise operations.
 */
void test_vector_partitioned(int use_acc) {
    float vec[SIZE];
    
    for (int i = 0; i < SIZE; i++) vec[i] = (float)i;
    
    if (use_acc) {
        #pragma acc parallel copy(vec[0:SIZE])
        {
            #pragma acc loop vector
            for (int i = 0; i < SIZE; i++) {
                vec[i] = vec[i] * vec[i] + 1.0f;
            }
        }
    } else {
        for (int i = 0; i < SIZE; i++)
            vec[i] = vec[i] * vec[i] + 1.0f;
    }
    
    printf("Vector partitioned: vec[0]=%.1f, vec[%d]=%.1f\n",
           vec[0], SIZE-1, vec[SIZE-1]);
}

/* Test 6: Gang+vector partitioned (case 5)
 * Loop with both gang and vector clauses.
 */
void test_gang_vector_partitioned(int use_acc) {
    float data[SIZE];
    
    for (int i = 0; i < SIZE; i++) data[i] = (float)(SIZE - i);
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:SIZE])
        {
            #pragma acc loop gang vector
            for (int i = 0; i < SIZE; i++) {
                data[i] = 1.0f / (data[i] + 1.0f);
            }
        }
    } else {
        for (int i = 0; i < SIZE; i++)
            data[i] = 1.0f / (data[i] + 1.0f);
    }
    
    printf("Gang+vector partitioned: data[0]=%.6f\n", data[0]);
}

/* Test 7: Worker+vector partitioned (case 6)
 * Nested loops with worker and vector clauses.
 */
void test_worker_vector_partitioned(int use_acc) {
    float matrix[DIM][DIM];
    
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            matrix[i][j] = (float)(i * j);
    
    if (use_acc) {
        #pragma acc parallel copy(matrix[0:DIM][0:DIM])
        {
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < DIM; j++) {
                    matrix[i][j] = sqrtf(matrix[i][j] + 1.0f);
                }
            }
        }
    } else {
        for (int i = 0; i < DIM; i++)
            for (int j = 0; j < DIM; j++)
                matrix[i][j] = sqrtf(matrix[i][j] + 1.0f);
    }
    
    printf("Worker+vector partitioned: matrix[2][2]=%.3f\n", matrix[2][2]);
}

/* Test 8: Fully partitioned (case 7)
 * Triple-nested loop with gang, worker, and vector clauses.
 */
void test_fully_partitioned(int use_acc) {
    float cube[DIM][DIM][DIM];
    
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            for (int k = 0; k < DIM; k++)
                cube[i][j][k] = (float)(i + j + k);
    
    if (use_acc) {
        #pragma acc parallel copy(cube[0:DIM][0:DIM][0:DIM])
        {
            #pragma acc loop gang
            for (int i = 1; i < DIM-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < DIM-1; j++) {
                    #pragma acc loop vector
                    for (int k = 1; k < DIM-1; k++) {
                        cube[i][j][k] = (cube[i-1][j][k] + cube[i][j-1][k] + 
                                        cube[i][j][k-1]) / 3.0f;
                    }
                }
            }
        }
    } else {
        for (int i = 1; i < DIM-1; i++)
            for (int j = 1; j < DIM-1; j++)
                for (int k = 1; k < DIM-1; k++)
                    cube[i][j][k] = (cube[i-1][j][k] + cube[i][j-1][k] + 
                                    cube[i][j][k-1]) / 3.0f;
    }
    
    printf("Fully partitioned: cube[1][1][1]=%.3f\n", cube[1][1][1]);
}

int main(int argc, char *argv[]) {
    int use_acc = 1;
    
    /* Use command-line argument to control execution path */
    if (argc > 1) {
        use_acc = atoi(argv[1]);
    }
    
    printf("Testing OpenACC partitioning patterns (use_acc=%d)\n", use_acc);
    
    /* Call all test functions to ensure all OpenACC constructs are compiled */
    test_gang_redundant(use_acc);
    test_gang_partitioned(use_acc);
    test_worker_partitioned(use_acc);
    test_gang_worker_partitioned(use_acc);
    test_vector_partitioned(use_acc);
    test_gang_vector_partitioned(use_acc);
    test_worker_vector_partitioned(use_acc);
    test_fully_partitioned(use_acc);
    
    printf("All partitioning tests completed.\n");
    return 0;
}
