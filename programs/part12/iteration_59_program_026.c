/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * 
 * This program uses OpenACC constructs to create variables with different
 * partitioning attributes, aiming to trigger all 8 partitioning states
 * during the neuter-broadcast transformation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128
#define P 64

/* Pattern A: Different scalar and array types with various data clauses */
void test_pattern_a(int *result) {
    int scalar_private;           /* Likely gang redundant (0) */
    int scalar_firstprivate = 42; /* Likely gang redundant (0) */
    int reduction_sum = 0;        /* Reduction variable */
    int arr_1d[N];                /* 1D array */
    int arr_2d[M][N];             /* 2D array */
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr_1d[i] = i;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr_2d[i][j] = i * N + j;
        }
    }
    
    /* OpenACC parallel region with complex data clauses */
    #pragma acc parallel loop copy(arr_1d[0:N]) copyin(arr_2d) \
        private(scalar_private) firstprivate(scalar_firstprivate) \
        reduction(+:reduction_sum) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        scalar_private = i % 10;
        
        /* Nested loop to create complex data flow */
        for (int j = 0; j < M; j++) {
            /* Conditional access pattern */
            if (j % 2 == 0) {
                arr_1d[i] += arr_2d[j][i] + scalar_private;
            } else {
                arr_1d[i] -= arr_2d[j][i] - scalar_firstprivate;
            }
        }
        
        /* Reduction operation */
        reduction_sum += arr_1d[i];
        
        /* Write to output */
        result[i] = arr_1d[i];
    }
    
    printf("Pattern A reduction sum: %d\n", reduction_sum);
}

/* Pattern B: Multi-dimensional arrays with complex access patterns */
void test_pattern_b(int *output) {
    int arr_3d[P][M][N];  /* 3D array - complex partitioning */
    int temp[M][N];        /* 2D temporary */
    
    /* Initialize 3D array */
    #pragma acc parallel loop collapse(3) copyout(arr_3d)
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                arr_3d[i][j][k] = i * M * N + j * N + k;
            }
        }
    }
    
    /* Complex kernel with multi-dimensional array accesses */
    #pragma acc parallel loop gang worker vector collapse(2) \
        copy(arr_3d) create(temp) copyout(output[0:P*M])
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            int worker_sum = 0;
            int vector_prod = 1;
            
            /* Vector loop */
            #pragma acc loop vector reduction(+:worker_sum) reduction(*:vector_prod)
            for (int k = 0; k < N; k++) {
                int val = arr_3d[i][j][k];
                worker_sum += val;
                
                /* Conditional to create divergent execution */
                if (val % 3 == 0) {
                    vector_prod *= (val % 7) + 1;
                }
            }
            
            temp[i][j] = worker_sum;
            output[i * M + j] = vector_prod;
        }
    }
}

/* Pattern C: Pointers and dynamic memory */
void test_pattern_c(int size) {
    int *dyn_arr1 = (int *)malloc(size * sizeof(int));
    int *dyn_arr2 = (int *)malloc(size * sizeof(int));
    int *dyn_arr3 = (int *)malloc(size * sizeof(int));
    
    /* Initialize dynamic arrays */
    for (int i = 0; i < size; i++) {
        dyn_arr1[i] = i;
        dyn_arr2[i] = size - i;
        dyn_arr3[i] = 0;
    }
    
    /* Map dynamic arrays to device */
    #pragma acc enter data copyin(dyn_arr1[0:size], dyn_arr2[0:size]) \
        create(dyn_arr3[0:size])
    
    /* Kernel with pointer arithmetic */
    #pragma acc parallel loop present(dyn_arr1, dyn_arr2, dyn_arr3)
    for (int i = 0; i < size; i++) {
        int *ptr1 = &dyn_arr1[i];
        int *ptr2 = &dyn_arr2[i];
        
        /* Complex pointer-based computation */
        for (int offset = 0; offset < 10 && i + offset < size; offset++) {
            dyn_arr3[i] += *(ptr1 + offset) * *(ptr2 + offset);
        }
        
        /* Nested conditional */
        if (dyn_arr3[i] > 1000) {
            dyn_arr3[i] = 1000;
        } else if (dyn_arr3[i] < -1000) {
            dyn_arr3[i] = -1000;
        }
    }
    
    #pragma acc exit data copyout(dyn_arr3[0:size])
    
    /* Verify results */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += dyn_arr3[i];
    }
    printf("Pattern C dynamic array sum: %d\n", sum);
    
    free(dyn_arr1);
    free(dyn_arr2);
    free(dyn_arr3);
}

/* Pattern D: Struct/Class-like data (C structs) */
typedef struct {
    int x;
    float y;
    double z;
    int arr[4];
} ComplexData;

void test_pattern_d() {
    ComplexData data_array[N];
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        data_array[i].x = i;
        data_array[i].y = i * 0.5f;
        data_array[i].z = i * 0.25;
        for (int j = 0; j < 4; j++) {
            data_array[i].arr[j] = i * 4 + j;
        }
    }
    
    /* Kernel accessing struct members with different patterns */
    #pragma acc parallel loop copy(data_array)
    for (int i = 0; i < N; i++) {
        /* Different partitioning for different struct members */
        data_array[i].x *= 2;  /* Simple scalar operation */
        
        /* Nested loop accessing struct array member */
        for (int j = 0; j < 4; j++) {
            data_array[i].arr[j] += data_array[i].x;
            
            /* Conditional update of float member */
            if (data_array[i].arr[j] % 2 == 0) {
                data_array[i].y += 1.0f;
            }
        }
        
        /* Complex computation on double member */
        data_array[i].z = data_array[i].z * data_array[i].y / (data_array[i].x + 1);
    }
    
    /* Verify */
    double total_z = 0;
    for (int i = 0; i < N; i++) {
        total_z += data_array[i].z;
    }
    printf("Pattern D total z: %f\n", total_z);
}

/* Combined test with all patterns */
void comprehensive_test() {
    int result_a[N];
    int result_b[P * M];
    
    printf("=== Starting comprehensive neuter-broadcast test ===\n");
    
    /* Execute all patterns */
    test_pattern_a(result_a);
    test_pattern_b(result_b);
    test_pattern_c(512);
    test_pattern_d();
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += result_a[i];
    }
    for (int i = 0; i < P * M; i++) {
        final_sum += result_b[i] % 1000;
    }
    
    printf("Final verification sum: %d\n", final_sum);
    printf("=== Test completed ===\n");
}

int main() {
    /* Enable compiler to see all code paths */
    int iteration;
    
    #pragma acc parallel loop
    for (iteration = 0; iteration < 2; iteration++) {
        if (iteration == 0) {
            comprehensive_test();
        } else {
            /* Alternative path with different partitioning */
            int temp[100];
            #pragma acc loop gang worker vector
            for (int i = 0; i < 100; i++) {
                temp[i] = i * iteration;
                
                /* Nested loops to increase complexity */
                for (int j = 0; j < 10; j++) {
                    for (int k = 0; k < 10; k++) {
                        temp[i] += j * k;
                    }
                }
            }
        }
    }
    
    comprehensive_test();
    
    return 0;
}
