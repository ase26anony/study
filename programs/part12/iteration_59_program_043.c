/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * 
 * This program uses OpenACC constructs to create variables with different
 * partitioning states, aiming to cover all cases in the switch statement
 * at lines 335-343 of omp-oacc-neuter-broadcast.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Helper function to verify results */
int verify_results(int *arr1, int *arr2, int size) {
    for (int i = 0; i < size; i++) {
        if (arr1[i] != arr2[i]) {
            return 0;
        }
    }
    return 1;
}

/* Main test function with complex OpenACC region */
void test_partitioning_states() {
    /* Pattern A: Different scalar types with various data clauses */
    int scalar_private = 42;           /* Likely gang redundant (0) */
    int scalar_firstprivate = 100;     /* May be gang partitioned (1) */
    int scalar_reduction = 0;          /* Reduction variable */
    
    /* Pattern B: Multi-dimensional arrays with different access patterns */
    int multi_dim[M][P];               /* 2D array for complex partitioning */
    int multi_dim_3d[10][20][30];      /* 3D array for deeper analysis */
    
    /* Pattern C: Dynamic memory and pointers */
    int *dynamic_arr = (int *)malloc(N * sizeof(int));
    int *dynamic_arr2 = (int *)malloc(N * sizeof(int));
    
    /* Pattern D: Struct/aggregate data */
    struct DataPoint {
        int x;
        int y;
        float z;
        double w;
    };
    struct DataPoint data_array[N];
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        dynamic_arr[i] = i;
        dynamic_arr2[i] = 0;
        data_array[i].x = i;
        data_array[i].y = i * 2;
        data_array[i].z = i * 0.5f;
        data_array[i].w = i * 1.5;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            multi_dim[i][j] = i * P + j;
        }
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                multi_dim_3d[i][j][k] = i * 600 + j * 30 + k;
            }
        }
    }
    
    /* Complex OpenACC region designed to trigger all partitioning states */
    #pragma acc data copyin(multi_dim[0:M][0:P], multi_dim_3d) \
                     copy(dynamic_arr[0:N]) \
                     copyout(dynamic_arr2[0:N]) \
                     copy(data_array[0:N]) \
                     create(scalar_private, scalar_firstprivate) \
                     reduction(+:scalar_reduction)
    {
        #pragma acc parallel loop gang vector_length(32) \
                private(scalar_private) \
                firstprivate(scalar_firstprivate) \
                reduction(+:scalar_reduction)
        for (int i = 0; i < N; i++) {
            /* Access multi-dimensional arrays with complex indexing */
            int idx_2d = i % (M * P);
            int val_2d = multi_dim[idx_2d / P][idx_2d % P];
            
            /* Access 3D array - triggers complex partitioning analysis */
            int idx_3d = i % (10 * 20 * 30);
            int x = idx_3d / (20 * 30);
            int y = (idx_3d % (20 * 30)) / 30;
            int z = idx_3d % 30;
            int val_3d = multi_dim_3d[x][y][z];
            
            /* Nested loops inside parallel region */
            int local_sum = 0;
            #pragma acc loop worker reduction(+:local_sum)
            for (int j = 0; j < 4; j++) {
                local_sum += j * val_2d;
            }
            
            /* Conditional operations */
            if (val_3d % 2 == 0) {
                dynamic_arr2[i] = dynamic_arr[i] + local_sum + scalar_private;
            } else {
                dynamic_arr2[i] = dynamic_arr[i] * 2 + scalar_firstprivate;
            }
            
            /* Struct member access with different patterns */
            data_array[i].x = dynamic_arr2[i] % 100;
            data_array[i].y = (dynamic_arr2[i] * 2) % 100;
            
            /* Vector-level operation */
            #pragma acc loop vector
            for (int k = 0; k < 2; k++) {
                data_array[i].z += k * 0.1f;
            }
            
            scalar_reduction += dynamic_arr2[i] % 10;
        }
        
        /* Additional parallel region with different characteristics */
        #pragma acc parallel loop gang worker vector_length(16)
        for (int i = 0; i < M; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (int j = 0; j < P; j++) {
                row_sum += multi_dim[i][j];
                /* Conditional store to trigger different partitioning */
                if (row_sum > 1000) {
                    multi_dim[i][j] = row_sum % 256;
                }
            }
            
            /* Worker-level operation */
            #pragma acc loop worker
            for (int w = 0; w < 2; w++) {
                scalar_private = row_sum * (w + 1);
            }
        }
    }
    
    /* Verify results */
    int expected_sum = 0;
    for (int i = 0; i < N; i++) {
        int idx_2d = i % (M * P);
        int val_2d = multi_dim[idx_2d / P][idx_2d % P];
        int local_sum = 0;
        for (int j = 0; j < 4; j++) {
            local_sum += j * val_2d;
        }
        
        int idx_3d = i % (10 * 20 * 30);
        int x = idx_3d / (20 * 30);
        int y = (idx_3d % (20 * 30)) / 30;
        int z = idx_3d % 30;
        int val_3d = multi_dim_3d[x][y][z];
        
        int expected;
        if (val_3d % 2 == 0) {
            expected = dynamic_arr[i] + local_sum + 42; /* scalar_private value */
        } else {
            expected = dynamic_arr[i] * 2 + 100; /* scalar_firstprivate value */
        }
        
        if (dynamic_arr2[i] != expected) {
            printf("Mismatch at index %d: got %d, expected %d\n", 
                   i, dynamic_arr2[i], expected);
        }
        
        expected_sum += expected % 10;
    }
    
    if (scalar_reduction == expected_sum) {
        printf("Reduction verification passed: %d\n", scalar_reduction);
    } else {
        printf("Reduction mismatch: got %d, expected %d\n", 
               scalar_reduction, expected_sum);
    }
    
    /* Cleanup */
    free(dynamic_arr);
    free(dynamic_arr2);
}

/* Additional test with OpenMP target for broader coverage */
#ifdef _OPENMP
void test_omp_partitioning() {
    int arr_omp[N];
    int arr_omp_out[N];
    int scalar_omp = 50;
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr_omp[i] = i * 3;
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
                map(to: arr_omp[0:N]) \
                map(from: arr_omp_out[0:N]) \
                private(scalar_omp) \
                reduction(+:scalar_omp)
    for (int i = 0; i < N; i++) {
        int local_var = i % 10;  /* Worker/vector partitioned candidate */
        
        /* Nested loop to create complex data flow */
        for (int j = 0; j < 8; j++) {
            local_var += arr_omp[i] % (j + 1);
        }
        
        arr_omp_out[i] = local_var;
        scalar_omp += local_var % 5;
    }
    
    printf("OpenMP scalar result: %d\n", scalar_omp);
}
#endif

int main() {
    printf("Testing OpenACC partitioning states...\n");
    test_partitioning_states();
    
    #ifdef _OPENMP
    printf("\nTesting OpenMP partitioning states...\n");
    test_omp_partitioning();
    #endif
    
    printf("\nTest completed.\n");
    return 0;
}
