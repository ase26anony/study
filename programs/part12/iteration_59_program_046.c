/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Mix of scalars and arrays with different data clauses */
void test_openacc_partitioning(int *result) {
    int i, j, k;
    
    /* Different partitioning states through various data attributes */
    int scalar_private;              /* Likely case 0: gang redundant */
    int scalar_firstprivate = 42;    /* Likely case 0: gang redundant */
    int reduction_sum = 0;           /* Reduction variable */
    
    /* Multi-dimensional arrays for complex partitioning */
    int arr_1d[N];                   /* 1D array */
    int arr_2d[M][M];                /* 2D array */
    int arr_3d[P][P][P];            /* 3D array for complex analysis */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) arr_1d[i] = i;
    for (i = 0; i < M; i++)
        for (j = 0; j < M; j++)
            arr_2d[i][j] = i * M + j;
    for (i = 0; i < P; i++)
        for (j = 0; j < P; j++)
            for (k = 0; k < P; k++)
                arr_3d[i][j][k] = i * P * P + j * P + k;
    
    /* Pattern B: Multi-dimensional array access with varying indices */
    #pragma acc parallel loop gang worker vector \
        copy(arr_1d, arr_2d, arr_3d) \
        copyin(scalar_firstprivate) \
        private(scalar_private) \
        reduction(+:reduction_sum) \
        copyout(result[0:N])
    for (i = 0; i < N; i++) {
        int worker_local;
        int vector_local;
        
        /* Nested loops to create complex data flow */
        for (j = 0; j < M; j++) {
            /* Conditional operations for control flow complexity */
            if (i % 2 == 0) {
                /* Access 2D array with gang and worker dimensions */
                worker_local = arr_2d[i % M][j];
                
                /* Pattern C: Pointer-like access through array indexing */
                int *ptr_access = &arr_1d[i];
                *ptr_access += worker_local;
            } else {
                /* Access 3D array with all three dimensions */
                int idx1 = i % P;
                int idx2 = j % P;
                int idx3 = (i + j) % P;
                vector_local = arr_3d[idx1][idx2][idx3];
                
                /* Complex computation involving all partitioning levels */
                arr_1d[i] += vector_local * scalar_firstprivate;
            }
            
            /* Reduction operation */
            reduction_sum += arr_1d[i] % 256;
        }
        
        /* Result depends on partitioning state */
        result[i] = arr_1d[i] + reduction_sum % 1000;
    }
}

/* Pattern D: Struct-based partitioning test */
struct ComplexData {
    int gang_field;
    int worker_field;
    int vector_field;
    float mixed_field;
};

void test_struct_partitioning(struct ComplexData *data, int size) {
    int i;
    
    #pragma acc parallel loop gang worker vector \
        copy(data[0:size])
    for (i = 0; i < size; i++) {
        /* Different fields accessed with different patterns */
        data[i].gang_field = i;          /* Gang-level access */
        data[i].worker_field = i * 2;    /* Worker-level access */
        
        /* Nested loop for vector-level computation */
        for (int v = 0; v < 16; v++) {
            data[i].vector_field += v;   /* Vector-level access */
            data[i].mixed_field += data[i].gang_field * 0.5f;
        }
    }
}

/* OpenMP version for broader coverage */
void test_openmp_partitioning(int *output) {
    int i, j;
    int shared_var = 100;
    int private_var;
    int firstprivate_var = 200;
    int reduction_var = 0;
    
    int array_2d[N][M];
    
    /* Initialize */
    for (i = 0; i < N; i++)
        for (j = 0; j < M; j++)
            array_2d[i][j] = i * M + j;
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: array_2d) \
        map(to: firstprivate_var) \
        private(private_var) \
        reduction(+:reduction_var) \
        map(from: output[0:N*M])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            private_var = i + j;
            
            /* Complex access patterns to trigger different partitioning */
            if ((i + j) % 3 == 0) {
                array_2d[i][j] += shared_var + firstprivate_var;
            } else if ((i + j) % 3 == 1) {
                array_2d[i][j] *= private_var;
            } else {
                array_2d[i][j] -= firstprivate_var - private_var;
            }
            
            reduction_var += array_2d[i][j] % 100;
            output[i * M + j] = array_2d[i][j];
        }
    }
}

/* Dynamic memory pattern */
void test_dynamic_partitioning() {
    int *dyn_array;
    int size = 1000;
    
    dyn_array = (int *)malloc(size * sizeof(int));
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        dyn_array[i] = i * 3;
    }
    
    /* OpenACC with dynamic data */
    #pragma acc enter data copyin(dyn_array[0:size])
    
    #pragma acc parallel loop gang vector \
        present(dyn_array)
    for (int i = 0; i < size; i++) {
        int local_var = i % 10;
        
        /* Nested conditional loops */
        for (int w = 0; w < 4; w++) {
            if (local_var > 5) {
                dyn_array[i] += w * 2;
            } else {
                dyn_array[i] -= w;
            }
        }
        
        /* Vector-level operation */
        for (int v = 0; v < 8; v++) {
            dyn_array[i] += (v % 2 == 0) ? 1 : -1;
        }
    }
    
    #pragma acc exit data copyout(dyn_array[0:size])
    
    free(dyn_array);
}

/* Main test driver */
int main() {
    int i;
    int result[N];
    int omp_result[N * M];
    struct ComplexData struct_data[100];
    
    printf("Testing OpenACC partitioning...\n");
    test_openacc_partitioning(result);
    
    /* Verify results aren't zero */
    int acc_sum = 0;
    for (i = 0; i < N; i++) {
        acc_sum += result[i];
    }
    printf("OpenACC result checksum: %d\n", acc_sum);
    
    printf("Testing struct partitioning...\n");
    test_struct_partitioning(struct_data, 100);
    
    int struct_sum = 0;
    for (i = 0; i < 100; i++) {
        struct_sum += struct_data[i].gang_field + 
                     struct_data[i].worker_field + 
                     struct_data[i].vector_field;
    }
    printf("Struct result checksum: %d\n", struct_sum);
    
    printf("Testing OpenMP partitioning...\n");
    test_openmp_partitioning(omp_result);
    
    int omp_sum = 0;
    for (i = 0; i < N * M; i++) {
        omp_sum += omp_result[i];
    }
    printf("OpenMP result checksum: %d\n", omp_sum);
    
    printf("Testing dynamic memory partitioning...\n");
    test_dynamic_partitioning();
    
    /* Final validation */
    if (acc_sum != 0 && struct_sum != 0 && omp_sum != 0) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Error: Some tests produced zero results.\n");
        return 1;
    }
}
