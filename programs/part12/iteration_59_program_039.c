/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test_neuter_broadcast.c -o test_neuter_broadcast
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Different types of variables that may get different partitioning states */
    int scalar_gang_redundant = 42;           /* Likely case 0: gang redundant */
    int scalar_private = 0;                   /* May get different partitioning */
    
    /* 1D arrays */
    int arr1d[N];                             /* Case 1: gang partitioned */
    int arr1d_private[N];                     /* Case 2: worker partitioned? */
    
    /* 2D arrays - Pattern B */
    int arr2d[M][N];                          /* Case 3: gang+worker partitioned */
    
    /* 3D arrays - more complex partitioning */
    int arr3d[P][M][N];                       /* Case 7: fully partitioned */
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
        arr1d_private[i] = 0;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            arr2d[i][j] = i * N + j;
        }
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                arr3d[i][j][k] = i * M * N + j * N + k;
            }
        }
    }
    
    /* Pattern C: Pointer-based dynamic memory */
    int *dynamic_arr = (int *)malloc(N * M * sizeof(int));
    for (i = 0; i < N * M; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* OpenACC parallel region with complex data clauses and nested loops */
    #pragma acc parallel loop copy(arr1d[0:N]) copyin(arr2d[0:M][0:N]) \
        copyout(arr1d_private[0:N]) create(arr3d[0:P][0:M][0:N]) \
        copy(dynamic_arr[0:N*M]) private(scalar_private) \
        reduction(+:scalar_gang_redundant) gang vector
    for (i = 0; i < N; i++) {
        int worker_local = 0;
        scalar_private = i;
        
        /* Nested loops to create complex data flow */
        for (j = 0; j < M; j++) {
            /* Access 2D array - may trigger gang+worker partitioning */
            int temp = arr2d[j % M][i];
            
            /* Conditional operations */
            if (temp % 3 == 0) {
                /* Access 3D array - may trigger fully partitioned */
                for (k = 0; k < P; k++) {
                    arr3d[k % P][j % M][i] += temp;
                }
                
                /* Pattern D: Struct-like access through separate arrays */
                worker_local += temp;
                
                /* Access dynamic memory */
                dynamic_arr[(j * N + i) % (N * M)] += worker_local;
            } else if (temp % 5 == 0) {
                /* Different access pattern */
                arr1d_private[i] += temp;
            }
        }
        
        /* Final computation with reduction */
        arr1d[i] = arr1d[i] * 2 + scalar_private;
        scalar_gang_redundant += arr1d[i] % 7;
    }
    
    /* Additional kernel with different data mapping */
    #pragma acc parallel loop gang worker vector collapse(2) \
        copy(arr2d[0:M][0:N]) copy(arr3d[0:P][0:M][0:N])
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            /* Complex conditional nesting */
            if ((i + j) % 8 == 0) {
                for (k = 0; k < P; k++) {
                    /* Mixed array accesses to trigger different partitioning */
                    arr3d[k][i][j] = arr2d[i][j] * k;
                    
                    /* Vector-level operation */
                    int vector_local = arr3d[k][i][j] % 256;
                    arr2d[i][j] += vector_local;
                }
            } else if ((i + j) % 3 == 0) {
                /* Worker-level operation */
                arr2d[i][j] = arr2d[i][j] * 3 / 2;
            }
        }
    }
    
    /* Clean up */
    free(dynamic_arr);
}

/* OpenMP version to trigger different code paths */
void test_openmp_partitioning() {
    int i, j, k;
    
    /* Variables with different storage classes */
    static int static_var = 100;              /* Different partitioning */
    register int reg_var;                     /* Register variable */
    volatile int vol_var = 0;                 /* Volatile variable */
    
    /* Multi-dimensional arrays */
    double dbl_arr[P][M];                     /* Case 4: vector partitioned? */
    float flt_arr[M][N];                      /* Case 5: gang+vector partitioned? */
    
    /* Initialize */
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            dbl_arr[i][j] = i * 1.0 + j * 0.1;
        }
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            flt_arr[i][j] = i * 0.5f + j * 0.25f;
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for collapse(2) \
        map(tofrom: dbl_arr[0:P][0:M]) map(to: flt_arr[0:M][0:N]) \
        private(reg_var) firstprivate(static_var) reduction(+:vol_var)
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            reg_var = i * j;
            
            /* Complex conditional structure */
            if (dbl_arr[i][j] > 50.0) {
                /* Nested loop inside parallel region */
                for (k = 0; k < N; k++) {
                    /* Access different array with different dimensionality */
                    float temp = flt_arr[j % M][k % N];
                    dbl_arr[i][j] += temp;
                    
                    /* Worker+vector operations */
                    if (k % 2 == 0) {
                        vol_var += (int)(temp * 10);
                    }
                }
                static_var++;
            } else {
                /* Different computation path */
                dbl_arr[i][j] = dbl_arr[i][j] * 0.9;
                reg_var = reg_var / 2;
            }
            
            /* Final assignment with mixed operations */
            dbl_arr[i][j] = dbl_arr[i][j] + reg_var * 0.01;
        }
    }
    
    /* Another OpenMP region with different mapping */
    int shared_arr[N];
    #pragma omp parallel for simd simdlen(8) \
        map(alloc: shared_arr[0:N])
    for (i = 0; i < N; i++) {
        /* SIMD vector operations */
        shared_arr[i] = 0;
        for (j = 0; j < 8; j++) {
            shared_arr[i] += i * j;
        }
        
        /* Conditional that depends on vector index */
        if (i % 16 == 0) {
            shared_arr[i] *= 2;
        }
    }
}

/* Simple struct for Pattern D */
struct DataPoint {
    int x;
    int y;
    float value;
    double weight;
};

void test_struct_partitioning() {
    int i;
    struct DataPoint points[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].value = i * 0.5f;
        points[i].weight = i * 0.25;
    }
    
    /* OpenACC with struct array */
    #pragma acc parallel loop copy(points[0:N])
    for (i = 0; i < N; i++) {
        /* Access different struct members - may trigger different partitioning */
        points[i].value = points[i].x * 0.3f + points[i].y * 0.7f;
        
        /* Conditional based on struct member */
        if (points[i].weight > 10.0) {
            points[i].weight *= 0.9;
            points[i].x += 1;
        } else {
            points[i].weight *= 1.1;
            points[i].y += 1;
        }
        
        /* Nested computation */
        for (int j = 0; j < 4; j++) {
            points[i].value += j * 0.1f;
        }
    }
}

/* Main function that validates results */
int main() {
    int i;
    int validation_passed = 1;
    
    printf("Testing OpenACC partitioning...\n");
    test_openacc_partitioning();
    
    printf("Testing OpenMP partitioning...\n");
    test_openmp_partitioning();
    
    printf("Testing struct partitioning...\n");
    test_struct_partitioning();
    
    /* Simple validation */
    int test_arr[10];
    #pragma acc parallel loop copyout(test_arr[0:10])
    for (i = 0; i < 10; i++) {
        test_arr[i] = i * i;
    }
    
    for (i = 0; i < 10; i++) {
        if (test_arr[i] != i * i) {
            validation_passed = 0;
            printf("Validation failed at index %d: expected %d, got %d\n", 
                   i, i * i, test_arr[i]);
        }
    }
    
    if (validation_passed) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}
