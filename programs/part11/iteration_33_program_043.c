/* test_oacc_partition.c - OpenACC partitioning test for GCC coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define DIM 32

/* Case 0: gang redundant */
void test_gang_redundant(int use_acc) {
    float data[SIZE];
    float sum = 0.0f;
    
    #pragma acc parallel if(use_acc) copy(data[0:SIZE]) copy(sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < SIZE; i++) {
            data[i] = i * 1.5f;
        }
        sum = 42.0f;  /* Simple assignment in gang-redundant region */
    }
    
    printf("Gang redundant: sum = %f, data[0] = %f\n", sum, data[0]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(int use_acc) {
    float data[SIZE];
    float sum = 0.0f;
    
    #pragma acc data copy(data[0:SIZE]) if(use_acc)
    {
        #pragma acc parallel if(use_acc) copy(sum)
        {
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < SIZE; i++) {
                data[i] = i * 2.0f;
                sum += data[i];
            }
        }
    }
    
    printf("Gang partitioned: sum = %f\n", sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(int use_acc) {
    float data[DIM][DIM];
    float sum = 0.0f;
    
    #pragma acc data copy(data[0:DIM][0:DIM]) if(use_acc)
    {
        #pragma acc parallel if(use_acc) copy(sum)
        {
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                #pragma acc loop worker reduction(+:sum)
                for (int j = 0; j < DIM; j++) {
                    data[i][j] = (i + j) * 1.0f;
                    sum += data[i][j];
                }
            }
        }
    }
    
    printf("Worker partitioned: sum = %f\n", sum);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int use_acc) {
    float data[DIM][DIM];
    float partial_sums[DIM];
    
    #pragma acc data copy(data[0:DIM][0:DIM], partial_sums[0:DIM]) if(use_acc)
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < DIM; i++) {
                float row_sum = 0.0f;
                #pragma acc loop vector reduction(+:row_sum)
                for (int j = 0; j < DIM; j++) {
                    data[i][j] = i * DIM + j;
                    row_sum += data[i][j];
                }
                partial_sums[i] = row_sum;
            }
        }
    }
    
    printf("Gang+worker partitioned: partial_sums[0] = %f\n", partial_sums[0]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(int use_acc) {
    float a[SIZE], b[SIZE], c[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
    }
    
    #pragma acc data copy(a[0:SIZE], b[0:SIZE], c[0:SIZE]) if(use_acc)
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop vector
            for (int i = 0; i < SIZE; i++) {
                c[i] = a[i] + b[i];  /* Element-wise vector operation */
            }
        }
    }
    
    printf("Vector partitioned: c[0] = %f, c[%d] = %f\n", c[0], SIZE-1, c[SIZE-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(int use_acc) {
    float matrix[DIM][DIM];
    float col_sums[DIM] = {0};
    
    #pragma acc data copy(matrix[0:DIM][0:DIM], col_sums[0:DIM]) if(use_acc)
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < DIM; i++) {
                for (int j = 0; j < DIM; j++) {
                    matrix[i][j] = (i * j) * 0.5f;
                }
            }
            
            #pragma acc loop gang reduction(+:col_sums)
            for (int j = 0; j < DIM; j++) {
                float col_sum = 0.0f;
                #pragma acc loop vector reduction(+:col_sum)
                for (int i = 0; i < DIM; i++) {
                    col_sum += matrix[i][j];
                }
                col_sums[j] = col_sum;
            }
        }
    }
    
    printf("Gang+vector partitioned: col_sums[0] = %f\n", col_sums[0]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int use_acc) {
    float data[DIM][DIM][DIM];
    float slice_sums[DIM];
    
    #pragma acc data create(data[0:DIM][0:DIM][0:DIM]) copy(slice_sums[0:DIM]) if(use_acc)
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang
            for (int k = 0; k < DIM; k++) {
                float slice_sum = 0.0f;
                #pragma acc loop worker vector reduction(+:slice_sum)
                for (int i = 0; i < DIM; i++) {
                    for (int j = 0; j < DIM; j++) {
                        data[k][i][j] = (k * DIM * DIM + i * DIM + j) * 0.1f;
                        slice_sum += data[k][i][j];
                    }
                }
                slice_sums[k] = slice_sum;
            }
        }
    }
    
    printf("Worker+vector partitioned: slice_sums[0] = %f\n", slice_sums[0]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int use_acc) {
    float grid[DIM][DIM];
    float new_grid[DIM][DIM];
    
    /* Initialize grid */
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            grid[i][j] = (i + j) * 1.0f;
        }
    }
    
    #pragma acc data copy(grid[0:DIM][0:DIM]) create(new_grid[0:DIM][0:DIM]) if(use_acc)
    {
        #pragma acc parallel if(use_acc)
        {
            /* Triple-nested loop with explicit partitioning */
            #pragma acc loop gang
            for (int i = 1; i < DIM-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < DIM-1; j++) {
                    float sum = 0.0f;
                    /* Stencil computation - forces careful partitioning */
                    #pragma acc loop vector reduction(+:sum)
                    for (int di = -1; di <= 1; di++) {
                        for (int dj = -1; dj <= 1; dj++) {
                            if (di == 0 && dj == 0) continue;
                            sum += grid[i+di][j+dj];
                        }
                    }
                    new_grid[i][j] = sum / 8.0f;  /* Average of neighbors */
                }
            }
        }
    }
    
    printf("Fully partitioned: new_grid[1][1] = %f\n", new_grid[1][1]);
}

/* Mixed partitioning test */
void test_mixed_partitioning(int use_acc) {
    float data1[SIZE], data2[SIZE];
    float result[SIZE];
    
    #pragma acc data copy(data1[0:SIZE], data2[0:SIZE], result[0:SIZE]) if(use_acc)
    {
        /* Different partitioning in same region */
        #pragma acc parallel if(use_acc)
        {
            /* gang partitioned */
            #pragma acc loop gang
            for (int i = 0; i < SIZE; i++) {
                data1[i] = i * 3.0f;
            }
            
            /* vector partitioned */
            #pragma acc loop vector
            for (int i = 0; i < SIZE; i++) {
                data2[i] = i * 1.5f;
            }
            
            /* worker partitioned */
            #pragma acc loop worker
            for (int i = 0; i < SIZE; i++) {
                result[i] = data1[i] + data2[i];
            }
        }
    }
    
    printf("Mixed partitioning: result[0] = %f, result[%d] = %f\n", 
           result[0], SIZE-1, result[SIZE-1]);
}

int main(int argc, char *argv[]) {
    int use_acc = 1;
    int test_case = 0;
    
    /* Use command line argument to control which test runs */
    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (argc > 2) {
            use_acc = atoi(argv[2]);
        }
    }
    
    printf("Running OpenACC partitioning tests (use_acc=%d)...\n", use_acc);
    
    /* Execute different tests based on input */
    switch (test_case) {
        case 0:
            test_gang_redundant(use_acc);
            break;
        case 1:
            test_gang_partitioned(use_acc);
            break;
        case 2:
            test_worker_partitioned(use_acc);
            break;
        case 3:
            test_gang_worker_partitioned(use_acc);
            break;
        case 4:
            test_vector_partitioned(use_acc);
            break;
        case 5:
            test_gang_vector_partitioned(use_acc);
            break;
        case 6:
            test_worker_vector_partitioned(use_acc);
            break;
        case 7:
            test_fully_partitioned(use_acc);
            break;
        case 8:
            test_mixed_partitioning(use_acc);
            break;
        default:
            /* Run all tests */
            test_gang_redundant(use_acc);
            test_gang_partitioned(use_acc);
            test_worker_partitioned(use_acc);
            test_gang_worker_partitioned(use_acc);
            test_vector_partitioned(use_acc);
            test_gang_vector_partitioned(use_acc);
            test_worker_vector_partitioned(use_acc);
            test_fully_partitioned(use_acc);
            test_mixed_partitioning(use_acc);
            break;
    }
    
    return 0;
}
