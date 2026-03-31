/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test_neuter_broadcast.c -o test_neuter_broadcast
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Various scalar and array variables with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* 1D arrays with different mappings */
    int arr1[N];                    /* Will likely be gang partitioned */
    int arr2[N];                    /* Will likely be worker partitioned */
    int arr3[N];                    /* Will likely be vector partitioned */
    int scalar1 = 42;               /* Will likely be gang redundant */
    
    /* Multi-dimensional arrays (Pattern B) */
    int md_arr[M][P];               /* Complex partitioning analysis */
    int md_arr2[M][P][8];           /* 3D array for more complex analysis */
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            md_arr[i][j] = i + j;
            for (k = 0; k < 8; k++) {
                md_arr2[i][j][k] = i * j * k;
            }
        }
    }
    
    /* OpenACC parallel region with complex data clauses and nested loops */
    #pragma acc parallel loop copy(arr1[0:N]) copyin(arr2[0:N]) copyout(arr3[0:N]) \
                              copy(md_arr[0:M][0:P]) copy(md_arr2[0:M][0:P][0:8]) \
                              private(scalar1) reduction(+:scalar1)
    for (i = 0; i < N; i++) {
        int local_var = 0;          /* Private variable */
        int gang_var, worker_var, vector_var;
        
        /* Nested loops to create complex data flow */
        for (j = 0; j < M; j++) {
            /* Access multi-dimensional arrays with different indices */
            int temp = md_arr[j % M][i % P];
            
            /* Conditional operations */
            if (i % 2 == 0) {
                /* Pattern that might trigger gang+worker partitioning */
                for (k = 0; k < 8; k++) {
                    md_arr2[j % M][i % P][k] += temp;
                }
            } else {
                /* Different access pattern */
                arr3[i] = arr1[i] + arr2[i] + temp;
            }
            
            /* More complex conditional nesting */
            if (j % 3 == 0) {
                /* Vector-oriented operations */
                #pragma acc loop vector
                for (k = 0; k < 8; k++) {
                    vector_var = md_arr2[j % M][i % P][k] % 256;
                    md_arr2[j % M][i % P][k] = vector_var;
                }
            }
            
            /* Worker-level computation */
            worker_var = (i * j) % 256;
            md_arr[j % M][i % P] = worker_var;
            
            /* Gang-level computation */
            gang_var = (i + j) % 256;
            arr1[i] = (arr1[i] + gang_var) % 256;
        }
        
        /* Reduction operation */
        scalar1 += local_var;
        
        /* Final conditional with vector operation */
        if (i % 4 == 0) {
            #pragma acc loop vector
            for (j = 0; j < 8; j++) {
                arr3[(i + j) % N] = (arr3[(i + j) % N] + md_arr2[i % M][j % P][j % 8]) % 256;
            }
        }
    }
    
    /* Verify results to ensure code isn't dead */
    int checksum = 0;
    for (i = 0; i < N; i++) {
        checksum = (checksum + arr1[i] + arr2[i] + arr3[i]) % 1000000;
    }
    printf("OpenACC checksum: %d\n", checksum);
}

/* Pattern C: Variable-length data and pointers */
void test_pointer_partitioning() {
    int size = 512;
    int *dyn_arr1 = (int *)malloc(size * sizeof(int));
    int *dyn_arr2 = (int *)malloc(size * sizeof(int));
    int *dyn_arr3 = (int *)malloc(size * sizeof(int));
    
    /* Initialize dynamic arrays */
    for (int i = 0; i < size; i++) {
        dyn_arr1[i] = i;
        dyn_arr2[i] = i * 2;
        dyn_arr3[i] = i * 3;
    }
    
    /* OpenACC with pointer-based data */
    #pragma acc parallel loop copy(dyn_arr1[0:size]) copyin(dyn_arr2[0:size]) \
                              copyout(dyn_arr3[0:size])
    for (int i = 0; i < size; i++) {
        /* Complex pointer arithmetic */
        int *ptr1 = &dyn_arr1[i];
        int *ptr2 = &dyn_arr2[(i + 1) % size];
        int *ptr3 = &dyn_arr3[(i + 2) % size];
        
        /* Nested loops with pointer access */
        for (int j = 0; j < 16; j++) {
            if (j % 2 == 0) {
                *ptr1 = (*ptr1 + *ptr2) % 256;
            } else {
                *ptr3 = (*ptr3 + *ptr1) % 256;
            }
            
            /* Vector operations on pointer data */
            #pragma acc loop vector
            for (int k = 0; k < 4; k++) {
                dyn_arr1[(i + k) % size] = (dyn_arr1[(i + k) % size] + j * k) % 256;
            }
        }
    }
    
    /* Verify results */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum = (checksum + dyn_arr1[i] + dyn_arr2[i] + dyn_arr3[i]) % 1000000;
    }
    printf("Pointer checksum: %d\n", checksum);
    
    free(dyn_arr1);
    free(dyn_arr2);
    free(dyn_arr3);
}

/* Pattern D: Struct-based data (C++ style in C) */
typedef struct {
    int x;
    int y;
    float z;
    double w;
} ComplexData;

void test_struct_partitioning() {
    ComplexData data_arr[N];
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        data_arr[i].x = i;
        data_arr[i].y = i * 2;
        data_arr[i].z = i * 0.5f;
        data_arr[i].w = i * 0.25;
    }
    
    /* OpenACC with struct array */
    #pragma acc parallel loop copy(data_arr[0:N])
    for (int i = 0; i < N; i++) {
        /* Access different struct members in different contexts */
        data_arr[i].x = (data_arr[i].x + i) % 256;
        
        /* Nested loop accessing struct members */
        for (int j = 0; j < 8; j++) {
            if (j % 3 == 0) {
                data_arr[i].y = (data_arr[i].y + data_arr[(i + j) % N].x) % 256;
            } else if (j % 3 == 1) {
                data_arr[i].z = data_arr[i].z + data_arr[(i + j) % N].y * 0.1f;
            } else {
                data_arr[i].w = data_arr[i].w + data_arr[(i + j) % N].z * 0.01;
            }
            
            /* Vector operations on struct fields */
            #pragma acc loop vector
            for (int k = 0; k < 4; k++) {
                data_arr[(i + k) % N].x = (data_arr[(i + k) % N].x + k) % 256;
            }
        }
    }
    
    /* Verify results */
    double checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += data_arr[i].x + data_arr[i].y + data_arr[i].z + data_arr[i].w;
    }
    printf("Struct checksum: %f\n", checksum);
}

/* OpenMP version for additional coverage */
void test_openmp_partitioning() {
    int arr1[N], arr2[N], arr3[N];
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
                map(to: arr1[0:N], arr2[0:N]) map(from: arr3[0:N])
    for (int i = 0; i < N; i++) {
        int local_sum = 0;
        
        /* Nested loops with conditionals */
        for (int j = 0; j < 16; j++) {
            if (j % 4 == 0) {
                /* Gang-level pattern */
                arr1[i] = (arr1[i] + j) % 256;
            } else if (j % 4 == 1) {
                /* Worker-level pattern */
                local_sum += arr2[(i + j) % N];
            } else if (j % 4 == 2) {
                /* Vector-level pattern */
                #pragma omp simd
                for (int k = 0; k < 8; k++) {
                    arr3[(i + k) % N] = (arr3[(i + k) % N] + k) % 256;
                }
            } else {
                /* Mixed partitioning pattern */
                arr2[i] = (arr1[i] + arr2[i] + local_sum) % 256;
            }
        }
        
        arr3[i] = local_sum % 256;
    }
    
    /* Verify results */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum = (checksum + arr1[i] + arr2[i] + arr3[i]) % 1000000;
    }
    printf("OpenMP checksum: %d\n", checksum);
}

int main() {
    printf("Testing OpenACC partitioning states...\n");
    test_openacc_partitioning();
    
    printf("Testing pointer-based partitioning...\n");
    test_pointer_partitioning();
    
    printf("Testing struct-based partitioning...\n");
    test_struct_partitioning();
    
    printf("Testing OpenMP partitioning...\n");
    test_openmp_partitioning();
    
    printf("All tests completed.\n");
    return 0;
}
