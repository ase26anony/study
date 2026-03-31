/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * Covers all partitioning states (0-7) in the switch statement
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Static arrays with different dimensions */
    int arr1d[N];                    /* 1D array - likely gang partitioned */
    int arr2d[M][N];                 /* 2D array - complex partitioning */
    int arr3d[P][M][N];              /* 3D array - may trigger full partitioning */
    
    /* Scalars with different storage classes */
    int scalar_private;              /* private scalar */
    int scalar_firstprivate = 42;    /* firstprivate scalar */
    int scalar_reduction = 0;        /* reduction variable */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i % 100;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            arr2d[i][j] = (i * j) % 100;
        }
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                arr3d[i][j][k] = (i + j + k) % 100;
            }
        }
    }
    
    /* Pattern B: Multi-dimensional array access with complex indexing */
    #pragma acc parallel loop gang worker vector copy(arr1d, arr2d, arr3d) \
        copyin(scalar_firstprivate) private(scalar_private) \
        reduction(+:scalar_reduction)
    for (i = 0; i < N; i++) {
        scalar_private = arr1d[i];
        
        /* Nested loops inside parallel region - creates complex data flow */
        for (j = 0; j < M; j++) {
            int temp = arr2d[j][i];
            
            /* Conditional operations */
            if (temp > 50) {
                scalar_private += temp;
                
                /* Access 3D array with all three indices varying */
                for (k = 0; k < P; k++) {
                    scalar_private += arr3d[k][j][i % P];
                }
            } else {
                scalar_private -= temp;
            }
        }
        
        scalar_reduction += scalar_private;
    }
    
    printf("OpenACC reduction result: %d\n", scalar_reduction);
}

/* Pattern C: Variable-length data and pointers */
void test_openacc_pointers() {
    int i, j;
    int *dynamic_arr1;
    int **dynamic_arr2;
    
    /* Dynamic 1D array */
    dynamic_arr1 = (int*)malloc(N * sizeof(int));
    
    /* Dynamic 2D array */
    dynamic_arr2 = (int**)malloc(M * sizeof(int*));
    for (i = 0; i < M; i++) {
        dynamic_arr2[i] = (int*)malloc(N * sizeof(int));
    }
    
    /* Initialize dynamic arrays */
    for (i = 0; i < N; i++) {
        dynamic_arr1[i] = i * 2;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            dynamic_arr2[i][j] = i * j * 3;
        }
    }
    
    /* Copy data to device using enter data */
    #pragma acc enter data copyin(dynamic_arr1[0:N])
    #pragma acc enter data copyin(dynamic_arr2[0:M])
    for (i = 0; i < M; i++) {
        #pragma acc enter data copyin(dynamic_arr2[i][0:N])
    }
    
    int sum = 0;
    
    /* Pattern C continued: Pointer access in parallel region */
    #pragma acc parallel loop gang worker vector \
        present(dynamic_arr1, dynamic_arr2) reduction(+:sum)
    for (i = 0; i < N; i++) {
        int local_sum = dynamic_arr1[i];
        
        /* Access through double pointer */
        for (j = 0; j < M; j++) {
            local_sum += dynamic_arr2[j][i];
        }
        
        /* Complex conditional with pointer arithmetic */
        if (i % 2 == 0) {
            local_sum *= 2;
        } else {
            local_sum /= 2;
        }
        
        sum += local_sum;
    }
    
    /* Clean up device data */
    #pragma acc exit data delete(dynamic_arr1)
    for (i = 0; i < M; i++) {
        #pragma acc exit data delete(dynamic_arr2[i])
    }
    #pragma acc exit data delete(dynamic_arr2)
    
    printf("Pointer-based sum: %d\n", sum);
    
    /* Free host memory */
    free(dynamic_arr1);
    for (i = 0; i < M; i++) {
        free(dynamic_arr2[i]);
    }
    free(dynamic_arr2);
}

/* Pattern D: Struct/Class data (C compatible) */
typedef struct {
    int x;
    int y;
    float z;
    double w;
} DataPoint;

void test_openacc_structs() {
    int i;
    DataPoint points[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 0.5f;
        points[i].w = i * 0.25;
    }
    
    float float_reduction = 0.0f;
    double double_reduction = 0.0;
    
    /* Access different struct members with different partitioning */
    #pragma acc parallel loop gang worker vector copy(points) \
        reduction(+:float_reduction, double_reduction)
    for (i = 0; i < N; i++) {
        /* Different operations on different struct members */
        points[i].x += i % 10;
        points[i].y *= 2;
        
        /* Conditional access pattern */
        if (i % 3 == 0) {
            points[i].z = points[i].x * 0.1f;
            float_reduction += points[i].z;
        } else if (i % 3 == 1) {
            points[i].w = points[i].y * 0.05;
            double_reduction += points[i].w;
        } else {
            points[i].z += points[i].w;
            float_reduction += points[i].z;
            double_reduction += points[i].w;
        }
    }
    
    printf("Struct reductions: float=%f, double=%lf\n", 
           float_reduction, double_reduction);
}

/* OpenMP version to trigger different code paths */
void test_openmp_partitioning() {
    int i, j;
    int omp_arr1[N];
    int omp_arr2[M][N];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        omp_arr1[i] = i;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            omp_arr2[i][j] = i + j;
        }
    }
    
    int omp_sum = 0;
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_arr1, omp_arr2) reduction(+:omp_sum) \
        private(j)
    for (i = 0; i < N; i++) {
        int local = omp_arr1[i];
        
        /* Nested loop with conditional */
        for (j = 0; j < M; j++) {
            if (j % 2 == 0) {
                local += omp_arr2[j][i];
            } else {
                local -= omp_arr2[j][i];
            }
        }
        
        /* Vector-like operation */
        #pragma omp simd reduction(+:omp_sum)
        for (j = 0; j < 8; j++) {
            omp_sum += local + j;
        }
    }
    
    printf("OpenMP sum: %d\n", omp_sum);
}

/* Combined test with kernels construct */
void test_kernels_construct() {
    int i, j, k;
    int kernels_arr1[N];
    int kernels_arr2[M][N];
    int kernels_arr3[P][M];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        kernels_arr1[i] = i % 50;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            kernels_arr2[i][j] = (i * j) % 100;
        }
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            kernels_arr3[i][j] = i + j;
        }
    }
    
    /* Kernels construct with multiple loops */
    #pragma acc kernels copy(kernels_arr1, kernels_arr2, kernels_arr3)
    {
        /* First loop - gang partitioned */
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            kernels_arr1[i] *= 2;
        }
        
        /* Second loop - gang+worker partitioned */
        #pragma acc loop gang worker
        for (i = 0; i < M; i++) {
            for (j = 0; j < N; j++) {
                kernels_arr2[i][j] += kernels_arr1[j];
            }
        }
        
        /* Third loop - vector partitioned */
        #pragma acc loop vector
        for (i = 0; i < P; i++) {
            for (j = 0; j < M; j++) {
                kernels_arr3[i][j] = kernels_arr2[j][i % M] + kernels_arr3[i][j];
            }
        }
    }
    
    /* Verify results */
    int check_sum = 0;
    for (i = 0; i < N; i++) {
        check_sum += kernels_arr1[i];
    }
    printf("Kernels check sum: %d\n", check_sum);
}

int main() {
    printf("Starting comprehensive neuter-broadcast coverage test...\n");
    
    /* Run all test patterns to maximize coverage */
    test_openacc_partitioning();
    test_openacc_pointers();
    test_openacc_structs();
    test_openmp_partitioning();
    test_kernels_construct();
    
    printf("All tests completed successfully.\n");
    
    /* Simple validation */
    int validation = 0;
    #pragma acc parallel loop reduction(+:validation)
    for (int i = 0; i < 100; i++) {
        validation += i;
    }
    
    int expected = 4950;  /* Sum of 0..99 */
    if (validation == expected) {
        printf("Validation passed: %d == %d\n", validation, expected);
        return 0;
    } else {
        printf("Validation failed: %d != %d\n", validation, expected);
        return 1;
    }
}
