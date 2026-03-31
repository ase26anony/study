/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning(void) {
    int i, j, k;
    
    /* Different partitioning states will be assigned to these variables */
    int scalar_private;              /* Likely gang redundant (0) */
    int scalar_firstprivate = 42;    /* Likely gang redundant (0) */
    int reduction_sum = 0;           /* Reduction variable */
    
    /* 1D arrays with different mappings */
    int arr1[N];                     /* copy - may get gang partitioned (1) */
    int arr2[N];                     /* copyin - may get different partitioning */
    int arr3[N];                     /* copyout */
    int arr4[N];                     /* create */
    
    /* Pattern B: Multi-dimensional arrays */
    int md_arr[M][P];                /* Multi-dimensional array */
    int md_arr2[P][M][8];            /* 3D array */
    
    /* Pattern C: Pointer-based dynamic memory */
    int *dyn_arr1 = (int*)malloc(N * sizeof(int));
    int *dyn_arr2 = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
        arr4[i] = i * 3;
        dyn_arr1[i] = i * 4;
        dyn_arr2[i] = i * 5;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            md_arr[i][j] = i * P + j;
        }
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < 8; k++) {
                md_arr2[i][j][k] = i * M * 8 + j * 8 + k;
            }
        }
    }
    
    /* OpenACC parallel region with complex data clauses and nested loops */
    #pragma acc parallel loop copy(arr1[0:N]) copyin(arr2[0:N]) \
                copyout(arr3[0:N]) create(arr4[0:N]) \
                copy(md_arr[0:M][0:P]) copy(md_arr2[0:P][0:M][0:8]) \
                copy(dyn_arr1[0:N]) copy(dyn_arr2[0:N]) \
                private(scalar_private) firstprivate(scalar_firstprivate) \
                reduction(+:reduction_sum)
    for (i = 0; i < N; i++) {
        scalar_private = i % 10;
        
        /* Nested loops accessing multi-dimensional arrays */
        if (i < M) {
            for (j = 0; j < P; j++) {
                /* Access with different index patterns to trigger
                   different partitioning analyses */
                md_arr[i][j] += scalar_private;
                
                /* Conditional access pattern */
                if (j % 2 == 0) {
                    /* Vector-like access pattern */
                    for (k = 0; k < 8 && (i*P+j) < N; k++) {
                        int idx = i * P + j;
                        if (idx < P && i < M && j < 8) {
                            md_arr2[idx % P][i % M][j % 8] += scalar_private;
                        }
                    }
                }
            }
        }
        
        /* Complex data flow with multiple array accesses */
        arr3[i] = arr1[i] + arr2[i] + arr4[i] + scalar_private;
        
        /* Access dynamic arrays with stride */
        if (i % 4 == 0) {
            dyn_arr1[i] = dyn_arr2[N - i - 1] + scalar_firstprivate;
        }
        
        /* Reduction operation */
        reduction_sum += arr1[i] % 100;
        
        /* Worker-level partitioning might be triggered here */
        int worker_local = arr2[i] % 20;
        for (j = 0; j < worker_local && j < 10; j++) {
            arr4[(i + j) % N] += j;
        }
        
        /* Vector-like operations */
        int vector_temp[4];
        for (k = 0; k < 4; k++) {
            vector_temp[k] = arr1[(i + k) % N] * k;
            arr3[i] += vector_temp[k];
        }
    }
    
    /* Verify results */
    int verify_sum = 0;
    for (i = 0; i < N; i++) {
        verify_sum += arr1[i] % 100;
    }
    
    printf("OpenACC: Reduction sum = %d (expected %d)\n", reduction_sum, verify_sum);
    
    free(dyn_arr1);
    free(dyn_arr2);
}

/* Pattern D: Struct-based data for additional partitioning complexity */
#ifdef __cplusplus
extern "C" {
#endif

struct DataPoint {
    float x, y, z;
    int id;
    double value;
};

void test_struct_partitioning(void) {
    int i, j;
    const int NUM_POINTS = 512;
    struct DataPoint points[NUM_POINTS];
    float grid[32][32];
    double partial_sums[16];
    
    /* Initialize */
    for (i = 0; i < NUM_POINTS; i++) {
        points[i].x = i * 0.1f;
        points[i].y = i * 0.2f;
        points[i].z = i * 0.3f;
        points[i].id = i;
        points[i].value = i * 1.5;
    }
    
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            grid[i][j] = i * 32.0f + j;
        }
    }
    
    for (i = 0; i < 16; i++) {
        partial_sums[i] = 0.0;
    }
    
    /* OpenACC with struct array */
    #pragma acc parallel loop copy(points[0:NUM_POINTS]) copy(grid[0:32][0:32]) \
                copyout(partial_sums[0:16])
    for (i = 0; i < NUM_POINTS; i++) {
        int grid_x = (int)(points[i].x * 32) % 32;
        int grid_y = (int)(points[i].y * 32) % 32;
        
        /* Access struct members - may trigger different partitioning */
        float old_val = grid[grid_x][grid_y];
        grid[grid_x][grid_y] = old_val + points[i].z;
        
        /* Reduction into partial sums array */
        int bucket = points[i].id % 16;
        #pragma acc atomic
        partial_sums[bucket] += points[i].value;
        
        /* Nested loop within parallel region */
        for (j = 0; j < 4; j++) {
            points[i].value += grid[(grid_x + j) % 32][grid_y] * 0.01f;
        }
    }
    
    double total = 0.0;
    for (i = 0; i < 16; i++) {
        total += partial_sums[i];
    }
    printf("Struct test: Total value = %f\n", total);
}

#ifdef __cplusplus
}
#endif

/* OpenMP version to trigger different code paths */
void test_openmp_partitioning(void) {
    int i, j;
    const int SIZE = 256;
    int a[SIZE], b[SIZE], c[SIZE];
    int matrix[16][16];
    int vector[16];
    
    /* Initialize */
    for (i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
        c[i] = 0;
    }
    
    for (i = 0; i < 16; i++) {
        vector[i] = i;
        for (j = 0; j < 16; j++) {
            matrix[i][j] = i * 16 + j;
        }
    }
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
                map(to: a[0:SIZE], b[0:SIZE], matrix[0:16][0:16]) \
                map(from: c[0:SIZE]) \
                map(tofrom: vector[0:16])
    for (i = 0; i < SIZE; i++) {
        /* Gang-level computation */
        c[i] = a[i] + b[i];
        
        /* Worker-level computation with nested loops */
        if (i % 16 == 0) {
            int team_id = i / 16;
            for (j = 0; j < 16; j++) {
                /* Access matrix with 2D indices */
                matrix[team_id % 16][j] += c[i];
                
                /* Vector-like operation */
                vector[j] += matrix[team_id % 16][j] % 10;
            }
        }
        
        /* Vector-like computation */
        int temp = 0;
        #pragma omp simd reduction(+:temp)
        for (j = 0; j < 8; j++) {
            temp += a[(i + j) % SIZE] * j;
        }
        c[i] += temp;
    }
    
    /* Verify */
    int errors = 0;
    for (i = 0; i < SIZE; i++) {
        int expected = a[i] + b[i];
        for (j = 0; j < 8 && i < SIZE - 8; j++) {
            expected += a[(i + j) % SIZE] * j;
        }
        if (c[i] != expected) {
            errors++;
        }
    }
    printf("OpenMP: Errors = %d\n", errors);
}

/* Main function that runs all tests */
int main(void) {
    printf("Testing GCC neuter-broadcast pass coverage...\n");
    
    /* Test OpenACC partitioning */
    test_openacc_partitioning();
    
    /* Test struct-based partitioning */
    test_struct_partitioning();
    
    /* Test OpenMP partitioning */
    test_openmp_partitioning();
    
    printf("All tests completed.\n");
    return 0;
}
