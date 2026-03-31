/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * 
 * This program uses OpenACC and OpenMP constructs to create variables with
 * different partitioning states, aiming to cover all cases in the switch
 * statement that maps integer codes to human-readable partitioning strings.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int i;
    int scalar_private = 42;           /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;     /* Could be gang partitioned (1) */
    int reduction_sum = 0;             /* Reduction variable */
    int arr1d[100];                    /* 1D array */
    int arr2d[10][20];                 /* 2D array */
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) arr1d[i] = i;
    for (int x = 0; x < 10; x++)
        for (int y = 0; y < 20; y++)
            arr2d[x][y] = x * y;
    
    #if USE_OPENACC
    #pragma acc parallel loop copy(arr1d[0:100]) copyin(arr2d) \
        private(scalar_private) firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum)
    #elif USE_OPENMP
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr1d[0:100]) map(to: arr2d) \
        private(scalar_private) firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum)
    #endif
    for (i = 0; i < 100; i++) {
        int local_var = scalar_private + scalar_firstprivate;
        
        /* Complex nested loops and conditionals to create varied data flow */
        for (int j = 0; j < 5; j++) {
            if (i % 2 == 0) {
                arr1d[i] += local_var + j;
            } else {
                arr1d[i] -= local_var - j;
            }
            
            /* Access 2D array with complex indexing */
            if (i < 10) {
                for (int k = 0; k < 20; k++) {
                    arr2d[i][k] += (i * k) % 7;
                }
            }
        }
        
        reduction_sum += arr1d[i] % 13;
    }
    
    printf("Pattern A result: reduction_sum = %d\n", reduction_sum);
}

/* Pattern B: Multi-dimensional arrays with varied access patterns */
void test_pattern_b(int n) {
    int arr3d[5][10][15];  /* 3D array - likely complex partitioning */
    int arr4d[3][4][5][6]; /* 4D array - even more complex */
    
    /* Initialize arrays */
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 10; j++)
            for (int k = 0; k < 15; k++)
                arr3d[i][j][k] = i * 100 + j * 10 + k;
                
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 5; k++)
                for (int l = 0; l < 6; l++)
                    arr4d[i][j][k][l] = i * 1000 + j * 100 + k * 10 + l;
    
    #if USE_OPENACC
    #pragma acc parallel loop collapse(2) copy(arr3d) copyout(arr4d[0:3][0:4][0:5][0:6])
    #elif USE_OPENMP
    #pragma omp target teams distribute parallel for collapse(2) \
        map(tofrom: arr3d) map(from: arr4d[0:3][0:4][0:5][0:6])
    #endif
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            /* Worker-level partitioning (2) and vector partitioning (4) */
            int worker_local = i * j;
            
            for (int k = 0; k < 15; k++) {
                /* Complex conditional access patterns */
                if (k % 3 == 0) {
                    arr3d[i][j][k] += worker_local;
                } else if (k % 3 == 1) {
                    arr3d[i][j][k] -= worker_local;
                } else {
                    arr3d[i][j][k] *= (worker_local % 7) + 1;
                }
                
                /* Cross-dimensional access for 4D array */
                if (i < 3 && j < 4 && k < 5) {
                    for (int l = 0; l < 6; l++) {
                        /* This creates gang+worker+vector partitioned access (7) */
                        arr4d[i][j][k][l] = arr3d[i][j][k] * l;
                    }
                }
            }
        }
    }
    
    /* Verify some values */
    int check_sum = 0;
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 10; j++)
            for (int k = 0; k < 15; k++)
                check_sum = (check_sum + arr3d[i][j][k]) % 1000;
    
    printf("Pattern B check sum: %d\n", check_sum);
}

/* Pattern C: Pointers and dynamic memory */
void test_pattern_c(int n) {
    int *dynamic_arr = (int*)malloc(n * sizeof(int));
    int *dynamic_arr2 = (int*)malloc(n * n * sizeof(int));
    int static_arr[50];
    
    if (!dynamic_arr || !dynamic_arr2) {
        printf("Memory allocation failed\n");
        return;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        dynamic_arr[i] = i * 2;
        static_arr[i % 50] = i;
    }
    
    for (int i = 0; i < n * n; i++) {
        dynamic_arr2[i] = i % 17;
    }
    
    #if USE_OPENACC
    #pragma acc enter data copyin(dynamic_arr[0:n], dynamic_arr2[0:n*n])
    #pragma acc enter data copyin(static_arr[0:50])
    
    #pragma acc parallel loop present(dynamic_arr[0:n], dynamic_arr2[0:n*n], static_arr[0:50])
    #elif USE_OPENMP
    #pragma omp target map(tofrom: dynamic_arr[0:n]) \
                      map(to: dynamic_arr2[0:n*n], static_arr[0:50])
    #pragma omp teams distribute parallel for
    #endif
    for (int i = 0; i < n; i++) {
        /* Pointer arithmetic creating complex data flow */
        int *ptr = &dynamic_arr[i];
        int idx = *ptr % n;
        
        /* Mixed static and dynamic array access */
        static_arr[i % 50] += dynamic_arr2[idx * n + i % n];
        
        /* Nested loops with pointer-based computation */
        for (int j = 0; j < 10; j++) {
            if (j % 2 == 0) {
                dynamic_arr[i] += static_arr[j % 50] * j;
            } else {
                dynamic_arr[i] -= dynamic_arr2[(i * j) % (n * n)] / (j + 1);
            }
        }
    }
    
    #if USE_OPENACC
    #pragma acc exit data copyout(dynamic_arr[0:n])
    #pragma acc exit data delete(dynamic_arr2[0:n*n], static_arr[0:50])
    #endif
    
    /* Verify results */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum = (sum + dynamic_arr[i]) % 10000;
    }
    printf("Pattern C sum: %d\n", sum);
    
    free(dynamic_arr);
    free(dynamic_arr2);
}

/* Pattern D: Structs and classes (C++ style in C) */
typedef struct {
    int x;
    float y;
    double z;
    int arr[5];
} ComplexStruct;

void test_pattern_d(int n) {
    ComplexStruct structs[20];
    ComplexStruct *struct_ptr = &structs[0];
    
    /* Initialize struct array */
    for (int i = 0; i < 20; i++) {
        structs[i].x = i;
        structs[i].y = i * 1.5f;
        structs[i].z = i * 2.5;
        for (int j = 0; j < 5; j++) {
            structs[i].arr[j] = i * 10 + j;
        }
    }
    
    #if USE_OPENACC
    #pragma acc parallel loop copy(structs[0:20])
    #elif USE_OPENMP
    #pragma omp target teams distribute parallel for map(tofrom: structs[0:20])
    #endif
    for (int i = 0; i < 20; i++) {
        /* Access different struct members with different patterns */
        struct_ptr = &structs[i];
        
        /* Different partitioning for different members */
        struct_ptr->x *= 2;                    /* Likely gang partitioned (1) */
        struct_ptr->y += struct_ptr->x * 0.5f; /* Worker partitioned (2) */
        
        /* Vector operations on array member */
        for (int j = 0; j < 5; j++) {
            struct_ptr->arr[j] += (int)(struct_ptr->y) * j;
            
            /* Conditional creating gang+vector partitioning (5) */
            if (j % 2 == 0) {
                struct_ptr->z += struct_ptr->arr[j] * 0.1;
            }
        }
        
        /* Nested conditionals for worker+vector partitioning (6) */
        for (int j = 0; j < 3; j++) {
            if (i % 3 == j) {
                struct_ptr->x += struct_ptr->arr[j % 5];
            }
        }
    }
    
    /* Verify results */
    double total_z = 0.0;
    for (int i = 0; i < 20; i++) {
        total_z += structs[i].z;
    }
    printf("Pattern D total_z: %.2f\n", total_z);
}

/* Combined test with all patterns to maximize coverage */
void test_combined(int n) {
    /* Create variables that might get different partitioning encodings */
    int gang_redundant_var = 0;    /* Case 0 */
    int gang_partitioned_arr[n];   /* Case 1 */
    int worker_partitioned_arr[n][10]; /* Case 2 */
    int gang_worker_partitioned_arr[n][n]; /* Case 3 */
    int vector_partitioned_arr[100]; /* Case 4 */
    int gang_vector_partitioned_arr[n][100]; /* Case 5 */
    int worker_vector_partitioned_arr[10][n]; /* Case 6 */
    int fully_partitioned_arr[n][n][n]; /* Case 7 */
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        gang_partitioned_arr[i] = i;
        for (int j = 0; j < 10; j++) worker_partitioned_arr[i][j] = i * j;
        for (int j = 0; j < n; j++) gang_worker_partitioned_arr[i][j] = i + j;
        for (int j = 0; j < 100; j++) gang_vector_partitioned_arr[i][j] = i * 100 + j;
        for (int j = 0; j < n; j++) 
            for (int k = 0; k < n; k++) 
                fully_partitioned_arr[i][j][k] = i * 10000 + j * 100 + k;
    }
    for (int i = 0; i < 100; i++) vector_partitioned_arr[i] = i * 2;
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < n; j++) worker_vector_partitioned_arr[i][j] = i * n + j;
    
    #if USE_OPENACC
    #pragma acc parallel loop copy(gang_partitioned_arr[0:n], \
                                   worker_partitioned_arr[0:n][0:10], \
                                   gang_worker_partitioned_arr[0:n][0:n], \
                                   vector_partitioned_arr[0:100], \
                                   gang_vector_partitioned_arr[0:n][0:100], \
                                   worker_vector_partitioned_arr[0:10][0:n], \
                                   fully_partitioned_arr[0:n][0:n][0:n]) \
        firstprivate(gang_redundant_var)
    #elif USE_OPENMP
    #pragma omp target teams distribute parallel for collapse(2) \
        map(tofrom: gang_partitioned_arr[0:n], \
                    worker_partitioned_arr[0:n][0:10], \
                    gang_worker_partitioned_arr[0:n][0:n], \
                    vector_partitioned_arr[0:100], \
                    gang_vector_partitioned_arr[0:n][0:100], \
                    worker_vector_partitioned_arr[0:10][0:n], \
                    fully_partitioned_arr[0:n][0:n][0:n]) \
        firstprivate(gang_redundant_var)
    #endif
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            /* Access all arrays with different patterns to trigger different partitioning */
            gang_redundant_var += i + j;  /* Case 0 */
            
            gang_partitioned_arr[i] += worker_partitioned_arr[i][j % 10]; /* Case 1 */
            
            if (i < 10) worker_partitioned_arr[i][j % 10] *= 2; /* Case 2 */
            
            gang_worker_partitioned_arr[i][j] += gang_partitioned_arr[i]; /* Case 3 */
            
            vector_partitioned_arr[(i + j) % 100] += i * j; /* Case 4 */
            
            gang_vector_partitioned_arr[i][j % 100] += vector_partitioned_arr[j % 100]; /* Case 5 */
            
            if (j < 10) worker_vector_partitioned_arr[j][i] += i * 3; /* Case 6 */
            
            for (int k = 0; k < n; k++) {
                fully_partitioned_arr[i][j][k] += 
                    gang_worker_partitioned_arr[i][j] + 
                    gang_vector_partitioned_arr[i][k % 100] + 
                    worker_vector_partitioned_arr[j % 10][k]; /* Case 7 */
            }
        }
    }
    
    /* Compute checksum */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += gang_partitioned_arr[i];
        for (int j = 0; j < 10; j++) checksum += worker_partitioned_arr[i][j];
        for (int j = 0; j < n; j++) checksum += gang_worker_partitioned_arr[i][j];
        for (int j = 0; j < 100; j++) checksum += gang_vector_partitioned_arr[i][j];
        for (int j = 0; j < n; j++) 
            for (int k = 0; k < n; k++) 
                checksum += fully_partitioned_arr[i][j][k];
    }
    for (int i = 0; i < 100; i++) checksum += vector_partitioned_arr[i];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < n; j++) checksum += worker_vector_partitioned_arr[i][j];
    
    printf("Combined test checksum: %lld\n", checksum);
}

int main() {
    const int N = 16;  /* Small enough for testing, large enough for partitioning */
    
    printf("Testing neuter-broadcast pass coverage...\n");
    
    /* Run all test patterns */
    test_pattern_a(N);
    test_pattern_b(N);
    test_pattern_c(N);
    test_pattern_d(N);
    test_combined(N);
    
    printf("All tests completed.\n");
    
    return 0;
}
