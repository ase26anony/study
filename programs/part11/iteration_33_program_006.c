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
            data[i] = i * 0.5f;
        }
        
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < SIZE; i++) {
            sum += data[i];
        }
    }
    
    if (use_acc) printf("Gang redundant sum: %f\n", sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(int use_acc) {
    float data[SIZE];
    float result[SIZE];
    
    #pragma acc data if(use_acc) copyin(data[0:SIZE]) copyout(result[0:SIZE])
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang
            for (int i = 0; i < SIZE; i++) {
                data[i] = i * 1.5f;
            }
        }
        
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang
            for (int i = 0; i < SIZE; i++) {
                result[i] = data[i] * 2.0f;
            }
        }
    }
    
    if (use_acc) printf("Gang partitioned first/last: %f, %f\n", 
                       result[0], result[SIZE-1]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(int use_acc) {
    float matrix[DIM][DIM];
    float vector[DIM];
    
    #pragma acc data if(use_acc) create(matrix[0:DIM][0:DIM]) copyout(vector[0:DIM])
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                #pragma acc loop worker
                for (int j = 0; j < DIM; j++) {
                    matrix[i][j] = i * DIM + j;
                }
            }
        }
        
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                float row_sum = 0.0f;
                #pragma acc loop worker reduction(+:row_sum)
                for (int j = 0; j < DIM; j++) {
                    row_sum += matrix[i][j];
                }
                vector[i] = row_sum;
            }
        }
    }
    
    if (use_acc) printf("Worker partitioned vector[0]: %f\n", vector[0]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int use_acc) {
    float matrix[DIM][DIM];
    float total = 0.0f;
    
    #pragma acc data if(use_acc) copy(matrix[0:DIM][0:DIM]) copy(total)
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < DIM; i++) {
                for (int j = 0; j < DIM; j++) {
                    matrix[i][j] = (i + j) * 0.25f;
                }
            }
        }
        
        #pragma acc parallel if(use_acc) reduction(+:total)
        {
            #pragma acc loop gang worker collapse(2)
            for (int i = 0; i < DIM; i++) {
                for (int j = 0; j < DIM; j++) {
                    total += matrix[i][j];
                }
            }
        }
    }
    
    if (use_acc) printf("Gang+worker total: %f\n", total);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(int use_acc) {
    float data[SIZE];
    float result[SIZE];
    
    #pragma acc data if(use_acc) copyin(data[0:SIZE]) copyout(result[0:SIZE])
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop vector
            for (int i = 0; i < SIZE; i++) {
                data[i] = i * 0.1f;
            }
        }
        
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop vector
            for (int i = 0; i < SIZE; i++) {
                result[i] = data[i] * data[i];
            }
        }
    }
    
    if (use_acc) printf("Vector partitioned result[10]: %f\n", result[10]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(int use_acc) {
    float data[SIZE];
    float max_val = 0.0f;
    
    #pragma acc data if(use_acc) copy(data[0:SIZE]) copy(max_val)
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < SIZE; i++) {
                data[i] = (i % 10) * 1.1f;
            }
        }
        
        #pragma acc parallel if(use_acc) reduction(max:max_val)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < SIZE; i++) {
                if (data[i] > max_val) max_val = data[i];
            }
        }
    }
    
    if (use_acc) printf("Gang+vector max: %f\n", max_val);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int use_acc) {
    float matrix[DIM][DIM];
    float row_max[DIM];
    
    #pragma acc data if(use_acc) create(matrix[0:DIM][0:DIM]) copyout(row_max[0:DIM])
    {
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < DIM; j++) {
                    matrix[i][j] = (i * 1.5f) + (j * 0.5f);
                }
            }
        }
        
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                float local_max = matrix[i][0];
                #pragma acc loop worker vector reduction(max:local_max)
                for (int j = 0; j < DIM; j++) {
                    if (matrix[i][j] > local_max)
                        local_max = matrix[i][j];
                }
                row_max[i] = local_max;
            }
        }
    }
    
    if (use_acc) printf("Worker+vector row_max[0]: %f\n", row_max[0]);
}

/* Case 7: fully partitioned */
void test_fully_partitioned(int use_acc) {
    float volume[DIM][DIM][DIM];
    float result[DIM][DIM][DIM];
    
    #pragma acc data if(use_acc) create(volume[0:DIM][0:DIM][0:DIM]) \
                               copyout(result[0:DIM][0:DIM][0:DIM])
    {
        /* Initialize volume */
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                #pragma acc loop worker
                for (int j = 0; j < DIM; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < DIM; k++) {
                        volume[i][j][k] = i + j + k;
                    }
                }
            }
        }
        
        /* Stencil computation with full partitioning */
        #pragma acc parallel if(use_acc)
        {
            #pragma acc loop gang
            for (int i = 1; i < DIM-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < DIM-1; j++) {
                    #pragma acc loop vector
                    for (int k = 1; k < DIM-1; k++) {
                        result[i][j][k] = (volume[i-1][j][k] + 
                                          volume[i][j-1][k] + 
                                          volume[i][j][k-1]) * 0.333f;
                    }
                }
            }
        }
    }
    
    if (use_acc) printf("Fully partitioned result[1][1][1]: %f\n", 
                       result[1][1][1]);
}

/* Main driver with conditional execution */
int main(int argc, char *argv[]) {
    int test_case = 0;
    int use_acc = 1;
    
    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (argc > 2) {
            use_acc = atoi(argv[2]);
        }
    }
    
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
        default:
            /* Run all tests sequentially */
            test_gang_redundant(use_acc);
            test_gang_partitioned(use_acc);
            test_worker_partitioned(use_acc);
            test_gang_worker_partitioned(use_acc);
            test_vector_partitioned(use_acc);
            test_gang_vector_partitioned(use_acc);
            test_worker_vector_partitioned(use_acc);
            test_fully_partitioned(use_acc);
            break;
    }
    
    return 0;
}
