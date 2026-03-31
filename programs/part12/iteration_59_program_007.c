/* Test program to cover all partitioning states in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Pattern D: Struct with multiple members */
typedef struct {
    int x;
    float y;
    double z;
} DataStruct;

/* Function containing complex OpenACC region */
void test_openacc_partitioning() {
    /* Pattern A: Various scalar variables */
    int scalar_private = 42;
    int scalar_firstprivate = 100;
    int scalar_reduction = 0;
    
    /* Pattern B: Multi-dimensional arrays */
    int multi_arr[N][M][P];
    float multi_arr2[N][M];
    
    /* Pattern A: 1D arrays */
    int arr1[N];
    float arr2[N];
    double arr3[N];
    
    /* Pattern D: Array of structs */
    DataStruct struct_arr[N];
    
    /* Pattern C: Pointer/dynamic memory */
    int *dyn_arr = (int*)malloc(N * sizeof(int));
    float *dyn_arr2 = (float*)malloc(N * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 1.5f;
        arr3[i] = i * 2.5;
        dyn_arr[i] = i * 3;
        dyn_arr2[i] = i * 4.0f;
        struct_arr[i].x = i;
        struct_arr[i].y = i * 1.1f;
        struct_arr[i].z = i * 2.2;
        
        for (int j = 0; j < M; j++) {
            multi_arr2[i][j] = i * j * 0.5f;
            for (int k = 0; k < P; k++) {
                multi_arr[i][j][k] = i + j + k;
            }
        }
    }
    
    /* Complex OpenACC region with multiple clauses and nesting */
    #pragma acc parallel loop gang vector \
        copy(arr1[0:N], arr2[0:N], arr3[0:N]) \
        copyin(multi_arr[0:N][0:M][0:P], multi_arr2[0:N][0:M]) \
        copyout(struct_arr[0:N]) \
        create(dyn_arr[0:N], dyn_arr2[0:N]) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:scalar_reduction)
    for (int i = 0; i < N; i++) {
        /* Pattern B: Access multi-dimensional arrays with complex indexing */
        for (int j = 0; j < M; j++) {
            /* Pattern A: Conditional operations create complex data flow */
            if (j % 2 == 0) {
                multi_arr2[i][j] *= 2.0f;
                
                /* Pattern B: Nested loop for 3D array */
                for (int k = 0; k < P; k++) {
                    multi_arr[i][j][k] += i * j * k;
                }
            } else {
                multi_arr2[i][j] /= 2.0f;
            }
        }
        
        /* Pattern D: Access struct members */
        struct_arr[i].x = arr1[i] * 2;
        struct_arr[i].y = arr2[i] * 3.0f;
        struct_arr[i].z = arr3[i] * 4.0;
        
        /* Pattern C: Access dynamic memory */
        dyn_arr[i] = i * scalar_private;
        dyn_arr2[i] = i * scalar_firstprivate * 0.5f;
        
        /* Pattern A: Reduction operation */
        scalar_reduction += arr1[i];
        
        /* Complex conditional with multiple branches */
        if (i % 3 == 0) {
            arr1[i] = dyn_arr[i] * 2;
        } else if (i % 3 == 1) {
            arr1[i] = dyn_arr[i] / 2;
        } else {
            arr1[i] = dyn_arr[i] + dyn_arr2[i];
        }
        
        /* Nested loop with vector operations */
        #pragma acc loop vector
        for (int v = 0; v < 16; v++) {
            arr2[i] += v * 0.1f;
        }
    }
    
    /* Additional OpenACC kernels region with different partitioning */
    #pragma acc kernels \
        copy(arr1[0:N], arr3[0:N]) \
        copy(multi_arr2[0:N][0:M]) \
        present(dyn_arr[0:N])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                multi_arr2[i][j] = multi_arr2[i][j] * 0.5f + i + j;
            }
            
            /* Pattern A: Mixed scalar operations */
            int temp = arr1[i];
            arr1[i] = arr3[i];
            arr3[i] = temp;
            
            /* Pattern C: Pointer arithmetic */
            int *ptr = &dyn_arr[i];
            *ptr = *ptr * 3;
        }
    }
    
    /* Clean up */
    free(dyn_arr);
    free(dyn_arr2);
}

/* Function with OpenMP target region for additional coverage */
void test_openmp_partitioning() {
    int omp_arr[N][M];
    float omp_vec[N];
    double omp_scalar = 1.0;
    int omp_reduction = 0;
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        omp_vec[i] = i * 0.25f;
        for (int j = 0; j < M; j++) {
            omp_arr[i][j] = i * j;
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_arr[0:N][0:M], omp_vec[0:N]) \
        map(to: omp_scalar) \
        reduction(+:omp_reduction) \
        private(omp_scalar)
    for (int i = 0; i < N; i++) {
        /* Pattern B: 2D array access with conditional */
        for (int j = 0; j < M; j++) {
            if (omp_arr[i][j] % 2 == 0) {
                omp_arr[i][j] += i + j;
            } else {
                omp_arr[i][j] -= i - j;
            }
        }
        
        /* Pattern A: Vector operations */
        #pragma omp simd
        for (int v = 0; v < 16; v++) {
            omp_vec[i] += v * 0.01f;
        }
        
        omp_reduction += omp_arr[i][0];
    }
    
    /* Nested OpenMP region */
    #pragma omp target teams distribute parallel for collapse(2) \
        map(omp_arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            omp_arr[i][j] = omp_arr[i][j] * 2;
        }
    }
}

/* Main function with validation */
int main() {
    printf("Testing OpenACC partitioning...\n");
    test_openacc_partitioning();
    
    printf("Testing OpenMP partitioning...\n");
    test_openmp_partitioning();
    
    /* Simple validation */
    int validation_arr[10];
    int sum = 0;
    
    #pragma acc parallel loop copyout(validation_arr[0:10]) reduction(+:sum)
    for (int i = 0; i < 10; i++) {
        validation_arr[i] = i * 2;
        sum += validation_arr[i];
    }
    
    /* Check result */
    int expected_sum = 0;
    for (int i = 0; i < 10; i++) {
        expected_sum += i * 2;
    }
    
    if (sum == expected_sum) {
        printf("Validation passed: sum = %d (expected %d)\n", sum, expected_sum);
        return 0;
    } else {
        printf("Validation failed: sum = %d (expected %d)\n", sum, expected_sum);
        return 1;
    }
}
