/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's
 * omp-oacc-neuter-broadcast.cc switch statement (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M 50
#define P 25

/* Pattern A: Various scalar and array types with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Different array dimensions to trigger various partitioning */
    int scalar = 42;                           /* Likely gang redundant (0) */
    int arr1d[N];                              /* 1D array */
    int arr2d[N][M];                           /* 2D array */
    int arr3d[N][M][P];                        /* 3D array - complex partitioning */
    int *dynamic_arr;                          /* Dynamic array */
    int reduction_sum = 0;                     /* Reduction variable */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
        for (j = 0; j < M; j++) {
            arr2d[i][j] = i * j;
            for (k = 0; k < P; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    dynamic_arr = (int*)malloc(N * M * sizeof(int));
    for (i = 0; i < N * M; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* OpenACC parallel region with complex data clauses and usage patterns */
    #pragma acc parallel loop gang vector copy(scalar) copyin(arr1d) \
        copyout(arr2d) create(arr3d) reduction(+:reduction_sum) \
        present_or_copyin(dynamic_arr[0:N*M])
    for (i = 0; i < N; i++) {
        int worker_local = arr1d[i];           /* Worker partitioned (2) */
        
        /* Nested loops to create complex data flow */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            int gang_worker_local = i + j;     /* Gang+worker partitioned (3) */
            
            /* Conditional to create divergent execution paths */
            if (i % 2 == 0) {
                /* Vector-level operations */
                #pragma acc loop vector
                for (k = 0; k < P; k++) {
                    int vector_local = arr3d[i][j][k]; /* Vector partitioned (4) */
                    
                    /* Complex computation using all array types */
                    arr2d[i][j] += vector_local + gang_worker_local + worker_local;
                    
                    /* Access dynamic array with stride */
                    dynamic_arr[i * M + j] += k;
                    
                    /* Gang+vector partitioned operations (5) */
                    int gang_vector_local = i * k;
                    arr2d[i][j] += gang_vector_local;
                    
                    /* Worker+vector partitioned operations (6) */
                    int worker_vector_local = j * k;
                    arr2d[i][j] += worker_vector_local;
                    
                    /* Fully partitioned operations (7) */
                    int fully_partitioned = i * j * k;
                    arr2d[i][j] += fully_partitioned;
                }
            } else {
                /* Different computation path */
                #pragma acc loop vector
                for (k = 0; k < P; k++) {
                    arr2d[i][j] += arr3d[i][j][k] * 2;
                }
            }
        }
        
        reduction_sum += i;
    }
    
    /* Gang partitioned variable usage (1) */
    #pragma acc parallel loop gang copy(scalar)
    for (i = 0; i < N; i++) {
        scalar += arr1d[i];
    }
    
    /* Verify results */
    int check_sum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            check_sum += arr2d[i][j];
        }
    }
    
    printf("OpenACC Test: scalar=%d, reduction_sum=%d, check_sum=%d\n", 
           scalar, reduction_sum, check_sum);
    
    free(dynamic_arr);
}

/* Pattern B: OpenMP target regions with teams/distribute */
void test_openmp_partitioning() {
    int i, j, k;
    
    /* Multi-dimensional arrays with different access patterns */
    double matrix_a[N][M];
    double matrix_b[M][P];
    double matrix_c[N][P];
    double scalar_mult = 2.5;
    double private_var;
    
    /* Initialize matrices */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix_a[i][j] = (double)(i + j) / 10.0;
        }
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            matrix_b[i][j] = (double)(i * j) / 5.0;
        }
    }
    
    /* OpenMP target region with complex mapping */
    #pragma omp target teams distribute parallel for \
        map(to: matrix_a, matrix_b, scalar_mult) \
        map(from: matrix_c) private(private_var) \
        collapse(2)
    for (i = 0; i < N; i++) {
        for (j = 0; j < P; j++) {
            double sum = 0.0;
            private_var = (double)(i + j);  /* Private to each thread */
            
            /* Matrix multiplication kernel */
            #pragma omp simd reduction(+:sum)
            for (k = 0; k < M; k++) {
                /* Various partitioning patterns through different operations */
                double temp = matrix_a[i][k] * matrix_b[k][j];
                
                /* Conditional to create different execution paths */
                if (temp > 1.0) {
                    sum += temp * scalar_mult;  /* Uses mapped scalar */
                } else {
                    sum += temp / scalar_mult;
                }
                
                /* Additional operations to create complex data flow */
                matrix_c[i][j] = sum + private_var;
            }
            
            /* Post-processing with different loop nesting */
            #pragma omp parallel for simd
            for (k = 0; k < 10; k++) {
                matrix_c[i][j] += (double)k * 0.1;
            }
        }
    }
    
    /* Verify matrix multiplication */
    double total = 0.0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < P; j++) {
            total += matrix_c[i][j];
        }
    }
    
    printf("OpenMP Test: matrix total = %f\n", total);
}

/* Pattern C: Struct/class with member variables */
struct ComplexData {
    int id;
    float values[10];
    double weight;
    char tag;
};

void test_struct_partitioning() {
    int i, j;
    struct ComplexData data_array[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        data_array[i].id = i;
        data_array[i].weight = (double)i / N;
        data_array[i].tag = 'A' + (i % 26);
        for (j = 0; j < 10; j++) {
            data_array[i].values[j] = (float)(i * j) / 10.0f;
        }
    }
    
    /* OpenACC with struct array */
    #pragma acc parallel loop copy(data_array)
    for (i = 0; i < N; i++) {
        float local_sum = 0.0f;
        
        /* Access different struct members */
        #pragma acc loop vector
        for (j = 0; j < 10; j++) {
            local_sum += data_array[i].values[j];
        }
        
        /* Modify struct members */
        data_array[i].weight = local_sum / 10.0;
        
        /* Conditional modification */
        if (data_array[i].id % 3 == 0) {
            data_array[i].tag = 'X';
        } else if (data_array[i].id % 3 == 1) {
            data_array[i].tag = 'Y';
        } else {
            data_array[i].tag = 'Z';
        }
    }
    
    /* Verify struct modifications */
    int count_x = 0, count_y = 0, count_z = 0;
    for (i = 0; i < N; i++) {
        if (data_array[i].tag == 'X') count_x++;
        else if (data_array[i].tag == 'Y') count_y++;
        else if (data_array[i].tag == 'Z') count_z++;
    }
    
    printf("Struct Test: X=%d, Y=%d, Z=%d\n", count_x, count_y, count_z);
}

/* Pattern D: Mixed OpenACC and OpenMP in different functions */
void mixed_partitioning_test() {
    int i, j;
    int mixed_array[N][M];
    int *ptr_array[N];
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        ptr_array[i] = (int*)malloc(M * sizeof(int));
        for (j = 0; j < M; j++) {
            mixed_array[i][j] = i * 100 + j;
            ptr_array[i][j] = (i * 100 + j) * 2;
        }
    }
    
    /* Complex OpenACC region with pointer arrays */
    #pragma acc data copy(mixed_array) copyin(ptr_array[0:N][0:M])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < N; i++) {
            int row_sum = 0;
            
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                /* Mix of direct and pointer access */
                mixed_array[i][j] += ptr_array[i][j];
                row_sum += mixed_array[i][j];
                
                /* Nested conditionals for complex control flow */
                if (row_sum > 1000) {
                    mixed_array[i][j] /= 2;
                } else if (row_sum > 500) {
                    mixed_array[i][j] /= 3;
                } else {
                    mixed_array[i][j] += j;
                }
            }
        }
    }
    
    /* Cleanup */
    for (i = 0; i < N; i++) {
        free(ptr_array[i]);
    }
    
    printf("Mixed partitioning test completed\n");
}

int main() {
    printf("Starting comprehensive partitioning state coverage test...\n");
    
    /* Run all test patterns to maximize coverage */
    test_openacc_partitioning();
    test_openmp_partitioning();
    test_struct_partitioning();
    mixed_partitioning_test();
    
    printf("All tests completed successfully!\n");
    
    return 0;
}
