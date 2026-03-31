/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * 
 * This program uses OpenACC and OpenMP constructs to create variables with
 * different partitioning states, aiming to cover all cases in the switch
 * statement that maps integer codes to human-readable partitioning strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _OPENACC
#define USE_OPENACC 1
#else
#define USE_OPENACC 0
#endif

#ifdef _OPENMP
#define USE_OPENMP 1
#else
#define USE_OPENMP 0
#endif

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_pattern_a(int n) {
    int i, j, k;
    int scalar_private = 42;           /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;     /* Likely gang redundant (0) */
    int reduction_sum = 0;             /* Reduction variable */
    
    /* Arrays with different dimensions and mappings */
    int arr1d[1000];                   /* 1D array - various partitioning */
    int arr2d[50][20];                 /* 2D array - complex partitioning */
    int arr3d[10][10][10];             /* 3D array - more complex */
    
    /* Initialize arrays */
    for (i = 0; i < 1000; i++) arr1d[i] = i % 100;
    for (i = 0; i < 50; i++)
        for (j = 0; j < 20; j++)
            arr2d[i][j] = i + j;
    for (i = 0; i < 10; i++)
        for (j = 0; j < 10; j++)
            for (k = 0; k < 10; k++)
                arr3d[i][j][k] = i * j * k;
    
    /* OpenACC parallel region with complex data clauses */
    #if USE_OPENACC
    #pragma acc parallel loop copy(arr1d[0:1000]) \
        copyin(arr2d[0:50][0:20]) \
        copyout(arr3d[0:10][0:10][0:10]) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum) \
        gang worker vector
    #endif
    #if USE_OPENMP
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr1d[0:1000]) \
        map(to: arr2d[0:50][0:20]) \
        map(from: arr3d[0:10][0:10][0:10]) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum) \
        collapse(2)
    #endif
    for (i = 0; i < n; i++) {
        /* Nested loops and conditionals to create complex data flow */
        int local_var = scalar_private + scalar_firstprivate;
        
        /* Access 1D array - potential for gang partitioning (1) */
        arr1d[i] = arr1d[i] * 2 + local_var;
        
        /* Conditional access to 2D array - worker partitioning (2) */
        if (i < 50) {
            for (j = 0; j < 20; j++) {
                arr2d[i][j] = arr2d[i][j] + (i * j) % 7;
                /* Nested conditional for complex control flow */
                if (arr2d[i][j] > 100) {
                    arr2d[i][j] = 100;
                }
            }
        }
        
        /* Access 3D array with multiple indices - gang+worker partitioning (3) */
        if (i < 10) {
            int idx1 = i % 10;
            int idx2 = (i * 2) % 10;
            int idx3 = (i * 3) % 10;
            arr3d[idx1][idx2][idx3] = arr3d[idx1][idx2][idx3] * 3;
        }
        
        /* Reduction operation */
        reduction_sum += arr1d[i] % 10;
        
        /* Vector-level computation - vector partitioned (4) */
        int vector_temp = 0;
        #if USE_OPENACC
        #pragma acc loop vector
        #endif
        #if USE_OPENMP
        #pragma omp simd
        #endif
        for (j = 0; j < 16; j++) {
            vector_temp += j * (i % 8);
        }
        arr1d[i] += vector_temp % 100;
    }
    
    printf("Pattern A: reduction_sum = %d\n", reduction_sum);
}

/* Pattern B: Multi-dimensional arrays with complex access patterns */
void test_pattern_b() {
    int i, j, k;
    const int DIM1 = 16, DIM2 = 32, DIM3 = 8;
    int cube[DIM1][DIM2][DIM3];
    int matrix[DIM1 * 2][DIM2];
    int vector[DIM1 * DIM2 * DIM3];
    
    /* Initialize data */
    for (i = 0; i < DIM1; i++) {
        for (j = 0; j < DIM2; j++) {
            for (k = 0; k < DIM3; k++) {
                cube[i][j][k] = (i * 1000) + (j * 100) + k;
            }
        }
    }
    
    for (i = 0; i < DIM1 * 2; i++) {
        for (j = 0; j < DIM2; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Complex OpenACC/OpenMP region with nested parallelism */
    #if USE_OPENACC
    #pragma acc parallel loop gang collapse(2) \
        copy(cube[0:DIM1][0:DIM2][0:DIM3]) \
        copy(matrix[0:DIM1*2][0:DIM2]) \
        create(vector[0:DIM1*DIM2*DIM3])
    #endif
    #if USE_OPENMP
    #pragma omp target teams distribute parallel for collapse(2) \
        map(tofrom: cube[0:DIM1][0:DIM2][0:DIM3]) \
        map(tofrom: matrix[0:DIM1*2][0:DIM2]) \
        map(alloc: vector[0:DIM1*DIM2*DIM3])
    #endif
    for (i = 0; i < DIM1; i++) {
        for (j = 0; j < DIM2; j++) {
            /* Worker-level computation */
            int worker_sum = 0;
            
            /* Vector loop inside worker - gang+vector partitioning (5) */
            #if USE_OPENACC
            #pragma acc loop vector
            #endif
            #if USE_OPENMP
            #pragma omp simd
            #endif
            for (k = 0; k < DIM3; k++) {
                cube[i][j][k] = cube[i][j][k] * 2 + (i + j + k);
                worker_sum += cube[i][j][k] % 256;
            }
            
            /* Access matrix with transformed indices - worker+vector partitioning (6) */
            int idx = (i * 2) % (DIM1 * 2);
            #if USE_OPENACC
            #pragma acc loop vector
            #endif
            #if USE_OPENMP
            #pragma omp simd
            #endif
            for (k = 0; k < 8; k++) {
                matrix[idx][j] += worker_sum * k;
            }
            
            /* Fully partitioned access (7) - all dimensions vary */
            int linear_idx = i * DIM2 * DIM3 + j * DIM3;
            #if USE_OPENACC
            #pragma acc loop vector
            #endif
            #if USE_OPENMP
            #pragma omp simd
            #endif
            for (k = 0; k < DIM3; k++) {
                vector[linear_idx + k] = cube[i][j][k] + matrix[idx][j];
            }
        }
    }
    
    /* Verify some results */
    int check_sum = 0;
    for (i = 0; i < DIM1; i += 4) {
        for (j = 0; j < DIM2; j += 8) {
            for (k = 0; k < DIM3; k += 2) {
                check_sum += cube[i][j][k] % 1000;
            }
        }
    }
    printf("Pattern B: check_sum = %d\n", check_sum);
}

/* Pattern C: Dynamic memory and pointers */
void test_pattern_c(int size) {
    int i, j;
    int *dynamic_array;
    int **jagged_array;
    int fixed_array[100][50];
    
    /* Allocate dynamic memory */
    dynamic_array = (int*)malloc(size * sizeof(int));
    jagged_array = (int**)malloc(20 * sizeof(int*));
    for (i = 0; i < 20; i++) {
        jagged_array[i] = (int*)malloc((i + 10) * sizeof(int));
        for (j = 0; j < i + 10; j++) {
            jagged_array[i][j] = i * 100 + j;
        }
    }
    
    /* Initialize arrays */
    for (i = 0; i < size; i++) {
        dynamic_array[i] = i * 3;
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 50; j++) {
            fixed_array[i][j] = i - j;
        }
    }
    
    /* Data region for dynamic arrays */
    #if USE_OPENACC
    #pragma acc enter data copyin(dynamic_array[0:size], fixed_array[0:100][0:50])
    for (i = 0; i < 20; i++) {
        #pragma acc enter data copyin(jagged_array[i][0:i+10])
    }
    #endif
    
    #if USE_OPENMP
    #pragma omp target enter data map(to: dynamic_array[0:size], fixed_array[0:100][0:50])
    #endif
    
    /* Parallel region with pointer accesses */
    #if USE_OPENACC
    #pragma acc parallel loop gang vector \
        present(dynamic_array[0:size], fixed_array[0:100][0:50])
    #endif
    #if USE_OPENMP
    #pragma omp target teams distribute parallel for \
        map(always, tofrom: dynamic_array[0:size]) \
        map(tofrom: fixed_array[0:100][0:50])
    #endif
    for (i = 0; i < size; i++) {
        /* Complex pointer arithmetic */
        int *ptr = dynamic_array + i;
        *ptr = *ptr * 2 + i % 7;
        
        /* Access fixed array with pointer-like indexing */
        if (i < 100) {
            for (j = 0; j < 50; j++) {
                fixed_array[i][j] = fixed_array[i][j] + (i * j) % 11;
            }
        }
        
        /* Conditional dynamic indexing */
        if (i < 20) {
            int *row = jagged_array[i];
            #if USE_OPENACC
            #pragma acc loop seq
            #endif
            for (j = 0; j < i + 10; j++) {
                row[j] = row[j] + i * j;
            }
        }
    }
    
    #if USE_OPENACC
    #pragma acc exit data copyout(dynamic_array[0:size], fixed_array[0:100][0:50])
    for (i = 0; i < 20; i++) {
        #pragma acc exit data copyout(jagged_array[i][0:i+10])
    }
    #endif
    
    #if USE_OPENMP
    #pragma omp target exit data map(from: dynamic_array[0:size], fixed_array[0:100][0:50])
    #endif
    
    /* Cleanup */
    free(dynamic_array);
    for (i = 0; i < 20; i++) {
        free(jagged_array[i]);
    }
    free(jagged_array);
}

/* Pattern D: Mixed OpenACC and OpenMP for maximum coverage */
void test_pattern_d() {
    int i, j;
    const int N = 1000;
    int array_a[N], array_b[N], array_c[N];
    int scalar_redundant = 999;
    int scalar_partitioned = 111;
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        array_a[i] = i;
        array_b[i] = N - i;
        array_c[i] = 0;
    }
    
    /* Combined computation with different partitioning hints */
    #if USE_OPENACC
    #pragma acc parallel loop gang(32) worker(4) vector_length(32) \
        copy(array_a[0:N], array_b[0:N]) \
        copyout(array_c[0:N]) \
        private(scalar_partitioned) \
        firstprivate(scalar_redundant)
    #endif
    #if USE_OPENMP && !USE_OPENACC
    #pragma omp target teams distribute parallel for \
        map(to: array_a[0:N], array_b[0:N]) \
        map(from: array_c[0:N]) \
        private(scalar_partitioned) \
        firstprivate(scalar_redundant) \
        num_teams(32) thread_limit(128)
    #endif
    for (i = 0; i < N; i++) {
        /* Multiple nested scopes for complex live ranges */
        {
            int temp1 = scalar_redundant + i;
            int temp2 = scalar_partitioned * 2;
            
            /* Vector partitioned operations */
            int vec_sum = 0;
            #if USE_OPENACC
            #pragma acc loop vector reduction(+:vec_sum)
            #endif
            #if USE_OPENMP && !USE_OPENACC
            #pragma omp simd reduction(+:vec_sum)
            #endif
            for (j = 0; j < 8; j++) {
                vec_sum += (temp1 + j) * (temp2 - j);
            }
            
            array_c[i] = array_a[i] + array_b[i] + vec_sum;
        }
        
        /* Additional conditional block */
        if (i % 3 == 0) {
            int block_local = scalar_partitioned + i;
            #if USE_OPENACC
            #pragma acc loop seq
            #endif
            for (j = 0; j < 4; j++) {
                array_c[i] += block_local * j;
            }
        } else if (i % 3 == 1) {
            /* Different computation path */
            array_c[i] = array_c[i] * 2 - scalar_redundant;
        }
        
        /* Final adjustment with another nested loop */
        int final_adjust = 0;
        #if USE_OPENACC
        #pragma acc loop worker reduction(+:final_adjust)
        #endif
        for (j = 0; j < 16; j++) {
            final_adjust += (i + j) % 8;
        }
        array_c[i] += final_adjust;
    }
    
    /* Verification */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += array_c[i] % 1000;
    }
    printf("Pattern D: sum = %d\n", sum);
}

/* Main function that runs all patterns */
int main() {
    printf("Testing neuter-broadcast pass coverage...\n");
    
    /* Run all test patterns */
    test_pattern_a(500);
    test_pattern_b();
    test_pattern_c(200);
    test_pattern_d();
    
    printf("All tests completed.\n");
    
    /* Final check to ensure code wasn't optimized away */
    volatile int dummy = 0;
    for (int i = 0; i < 10; i++) {
        dummy += i;
    }
    
    return dummy == 0 ? 0 : 0;  /* Always return 0 for successful execution */
}
