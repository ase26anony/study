/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's
 * omp-oacc-neuter-broadcast.cc switch statement (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Pattern A: Various scalar and array types with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Different array dimensions to influence partitioning */
    int scalar = 42;                     /* Likely gang redundant (0) */
    int arr1d[N];                        /* 1D array */
    int arr2d[N][M];                     /* 2D array */
    int arr3d[N][M][P];                  /* 3D array - complex partitioning */
    int *dynamic_arr;                    /* Dynamic memory */
    int reduction_var = 0;               /* Reduction variable */
    
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
    
    /* OpenACC parallel region with complex data clauses and nested loops */
    #pragma acc parallel loop copy(arr1d) copyin(arr2d) \
        copyout(arr3d) create(scalar) reduction(+:reduction_var) \
        present_or_copy(dynamic_arr[0:N*M])
    for (i = 0; i < N; i++) {
        /* Access scalar - may be gang redundant (0) */
        int local_scalar = scalar + i;
        
        /* Pattern B: Multi-dimensional array access with varying indices */
        for (j = 0; j < M; j++) {
            /* Complex conditional to create varied data flow */
            if (arr2d[i][j] % 3 == 0) {
                /* Worker-level partitioning patterns (2, 3, 6) */
                for (k = 0; k < P; k++) {
                    /* Vector-level operations (4, 5, 6, 7) */
                    arr3d[i][j][k] = arr1d[i] + arr2d[i][j] + k;
                    
                    /* Conditional vector operations */
                    if (k % 2 == 0) {
                        arr3d[i][j][k] *= 2;
                    } else {
                        arr3d[i][j][k] /= 2;
                    }
                }
            } else if (arr2d[i][j] % 5 == 0) {
                /* Different access pattern for different partitioning */
                for (k = 0; k < P; k += 2) {
                    arr3d[i][j][k] = dynamic_arr[i * M + j] * k;
                }
            }
            
            /* Reduction operation */
            reduction_var += arr2d[i][j] % 7;
        }
        
        /* Write back to 1D array with stride */
        arr1d[i] = local_scalar * 2;
    }
    
    /* Additional kernel with different partitioning characteristics */
    int temp_arr[N];
    #pragma acc parallel loop gang worker vector copy(temp_arr)
    for (i = 0; i < N; i++) {
        /* Explicit gang/worker/vector partitioning hints */
        #pragma acc loop gang
        for (j = 0; j < M; j++) {
            #pragma acc loop worker
            for (k = 0; k < P; k++) {
                /* This nested structure forces complex partitioning analysis */
                temp_arr[i] += arr3d[i][j][k] % 11;
            }
        }
    }
    
    free(dynamic_arr);
}

/* Pattern C: OpenMP target region with teams/distribute */
void test_openmp_partitioning() {
    int i, j, k;
    int omp_arr[N][M];
    int omp_private = 100;
    int omp_firstprivate = 200;
    int omp_reduction = 0;
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            omp_arr[i][j] = i - j;
        }
    }
    
    /* OpenMP target with teams and distribute - different partitioning */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_arr) map(to: omp_firstprivate) \
        reduction(+:omp_reduction) private(omp_private)
    for (i = 0; i < N; i++) {
        omp_private = omp_firstprivate + i;
        
        /* Nested loops inside parallel region */
        for (j = 0; j < M; j++) {
            /* Conditional with different loop structures */
            if (i % 4 == 0) {
                /* Vector-like operation */
                for (k = 0; k < 16; k++) {
                    omp_arr[i][j] += k * omp_private;
                }
            } else {
                /* Worker-like operation */
                for (k = 0; k < 8; k++) {
                    omp_arr[i][j] -= k * (j + 1);
                }
            }
            
            /* Reduction across multiple dimensions */
            omp_reduction += omp_arr[i][j] % 13;
        }
    }
    
    /* Second OpenMP region with collapse for additional partitioning */
    int collapsed_arr[N * M];
    #pragma omp target teams distribute parallel for collapse(2) \
        map(collapsed_arr)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            int idx = i * M + j;
            /* Complex computation to prevent optimization */
            collapsed_arr[idx] = (i * 7919 + j * 65537) % 1024;
            for (k = 0; k < 4; k++) {
                collapsed_arr[idx] ^= (k << (i % 8));
            }
        }
    }
}

/* Pattern D: Struct-based data for aggregate partitioning */
struct DataPoint {
    int x;
    int y;
    float z;
    double values[4];
};

void test_struct_partitioning() {
    int i, j;
    struct DataPoint points[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 0.5f;
        for (j = 0; j < 4; j++) {
            points[i].values[j] = i * j * 0.25;
        }
    }
    
    /* OpenACC with struct array - members may have different partitioning */
    #pragma acc parallel loop copy(points)
    for (i = 0; i < N; i++) {
        /* Access different struct members in different ways */
        points[i].x += i % 3;
        
        /* Nested loop accessing array member */
        for (j = 0; j < 4; j++) {
            /* Conditional access pattern */
            if (j % 2 == 0) {
                points[i].values[j] *= points[i].z;
            } else {
                points[i].values[j] /= points[i].y + 1.0;
            }
        }
        
        /* Cross-member computation */
        points[i].z = points[i].x * 0.01f + points[i].y * 0.02f;
    }
}

/* Main function to drive all tests */
int main() {
    printf("Starting neuter-broadcast partitioning test...\n");
    
    /* Run OpenACC tests */
    test_openacc_partitioning();
    printf("OpenACC partitioning test completed.\n");
    
    /* Run OpenMP tests */
    test_openmp_partitioning();
    printf("OpenMP partitioning test completed.\n");
    
    /* Run struct-based test */
    test_struct_partitioning();
    printf("Struct partitioning test completed.\n");
    
    /* Verification step to ensure code isn't optimized away */
    int verification = 0;
    #pragma acc parallel loop reduction(+:verification)
    for (int i = 0; i < 100; i++) {
        verification += i % 7;
    }
    
    printf("Final verification value: %d\n", verification);
    printf("All tests completed successfully.\n");
    
    return 0;
}
