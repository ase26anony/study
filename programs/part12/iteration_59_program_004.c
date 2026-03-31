/* test_neuter_broadcast.c - Comprehensive test for OpenACC/OpenMP neuter-broadcast pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Pattern A: Various scalar and array types with different data clauses */
void test_pattern_a(int *result) {
    int i;
    int scalar_private = 0;          /* Likely gang redundant (0) */
    int scalar_firstprivate = 42;    /* Likely gang redundant (0) */
    int scalar_reduction = 0;        /* Reduction variable */
    
    int arr1d[N];                    /* 1D array */
    int arr2d[M][M];                 /* 2D array */
    int arr3d[P][P][P];              /* 3D array - complex partitioning */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) arr1d[i] = i;
    for (int j = 0; j < M; j++)
        for (int k = 0; k < M; k++)
            arr2d[j][k] = j * M + k;
    
    /* OpenACC parallel region with various data clauses */
    #pragma acc parallel loop copyout(arr1d[0:N]) \
        copyin(arr2d[0:M][0:M]) create(arr3d[0:P][0:P][0:P]) \
        private(scalar_private) firstprivate(scalar_firstprivate) \
        reduction(+:scalar_reduction)
    for (i = 0; i < N; i++) {
        int local_var = i;  /* Private to each iteration */
        
        /* Nested loops accessing multi-dimensional arrays */
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                /* Conditional operations */
                if (arr2d[j][k] % 2 == 0) {
                    arr1d[i] += arr2d[j][k];
                }
            }
        }
        
        /* Access 3D array with complex indexing */
        int idx1 = i % P;
        int idx2 = (i * 2) % P;
        int idx3 = (i * 3) % P;
        arr3d[idx1][idx2][idx3] = i;
        
        scalar_reduction += i;
    }
    
    *result = scalar_reduction;
}

/* Pattern B: Multi-dimensional array with gang/worker/vector partitioning */
void test_pattern_b(int *result) {
    int arr_gang[M][M];      /* Gang partitioned (1) */
    int arr_worker[M][M];    /* Worker partitioned (2) */
    int arr_vector[M][M];    /* Vector partitioned (4) */
    
    /* Initialize */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr_gang[i][j] = i * M + j;
            arr_worker[i][j] = (i + j) % 256;
            arr_vector[i][j] = i ^ j;
        }
    }
    
    int sum = 0;
    
    /* Complex OpenACC loop with gang, worker, vector clauses */
    #pragma acc parallel loop gang worker vector \
        copyin(arr_gang[0:M][0:M], arr_worker[0:M][0:M], arr_vector[0:M][0:M]) \
        copyout(sum)
    for (int i = 0; i < M; i++) {
        int gang_sum = 0;
        int worker_sum = 0;
        int vector_sum = 0;
        
        #pragma acc loop gang
        for (int g = 0; g < M; g++) {
            gang_sum += arr_gang[i][g];
            
            #pragma acc loop worker
            for (int w = 0; w < M; w++) {
                worker_sum += arr_worker[g][w];
                
                #pragma acc loop vector
                for (int v = 0; v < M; v++) {
                    vector_sum += arr_vector[w][v];
                }
            }
        }
        
        sum += gang_sum + worker_sum + vector_sum;
    }
    
    *result = sum;
}

/* Pattern C: Dynamic memory and pointers */
void test_pattern_c(int *result) {
    int *dynamic_arr;
    int **ptr_array;
    int size = N;
    
    /* Allocate dynamic memory */
    dynamic_arr = (int *)malloc(size * sizeof(int));
    ptr_array = (int **)malloc(M * sizeof(int *));
    
    for (int i = 0; i < size; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    for (int i = 0; i < M; i++) {
        ptr_array[i] = (int *)malloc(M * sizeof(int));
        for (int j = 0; j < M; j++) {
            ptr_array[i][j] = i + j;
        }
    }
    
    int total = 0;
    
    /* OpenACC with dynamic data */
    #pragma acc enter data copyin(dynamic_arr[0:size])
    #pragma acc enter data copyin(ptr_array[0:M])
    
    for (int i = 0; i < M; i++) {
        #pragma acc enter data copyin(ptr_array[i][0:M])
    }
    
    #pragma acc parallel loop reduction(+:total) \
        present(dynamic_arr[0:size], ptr_array[0:M])
    for (int i = 0; i < size; i++) {
        total += dynamic_arr[i];
        
        /* Access through pointer array */
        if (i < M) {
            for (int j = 0; j < M; j++) {
                total += ptr_array[i][j];
            }
        }
    }
    
    #pragma acc exit data delete(dynamic_arr[0:size])
    #pragma acc exit data delete(ptr_array[0:M])
    
    for (int i = 0; i < M; i++) {
        free(ptr_array[i]);
    }
    free(ptr_array);
    free(dynamic_arr);
    
    *result = total;
}

/* Pattern D: Struct/Class data (C compatible) */
typedef struct {
    int x;
    int y;
    float z;
    double w;
} DataPoint;

void test_pattern_d(int *result) {
    DataPoint points[N];
    DataPoint local_copy[N];
    
    /* Initialize struct array */
    for (int i = 0; i < N; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 0.5f;
        points[i].w = i * 0.25;
    }
    
    int sum = 0;
    
    /* OpenACC with struct array */
    #pragma acc parallel loop copy(points[0:N]) copyout(local_copy[0:N]) reduction(+:sum)
    for (int i = 0; i < N; i++) {
        /* Different members accessed with different patterns */
        local_copy[i].x = points[i].x * 2;      /* Simple copy */
        local_copy[i].y = points[i].y + i;      /* Modified */
        
        /* Conditional access */
        if (i % 3 == 0) {
            local_copy[i].z = points[i].z * 3.0f;
        } else {
            local_copy[i].z = points[i].z;
        }
        
        /* Complex computation */
        local_copy[i].w = points[i].w * (i % 7);
        
        sum += local_copy[i].x + local_copy[i].y;
    }
    
    *result = sum;
}

/* OpenMP version for comparison */
#ifdef _OPENMP
void test_omp_pattern(int *result) {
    int arr1[N], arr2[N], arr3[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i;
        arr3[i] = 0;
    }
    
    int sum = 0;
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(to: arr1[0:N], arr2[0:N]) map(from: arr3[0:N]) \
        reduction(+:sum) private(i) shared(arr1, arr2, arr3)
    for (int i = 0; i < N; i++) {
        /* Nested conditionals and loops */
        int temp = 0;
        for (int j = 0; j < 10; j++) {
            if (j % 2 == 0) {
                temp += arr1[i] * j;
            } else {
                temp += arr2[i] * j;
            }
        }
        
        arr3[i] = temp;
        
        /* Access with different strides */
        if (i % 4 == 0) {
            sum += arr1[i] * 2;
        } else if (i % 4 == 1) {
            sum += arr2[i] * 3;
        } else {
            sum += arr3[i];
        }
    }
    
    *result = sum;
}
#endif

int main() {
    int result_a = 0, result_b = 0, result_c = 0, result_d = 0;
    int result_omp = 0;
    
    printf("Testing OpenACC/OpenMP neuter-broadcast patterns...\n");
    
    /* Test Pattern A - Basic arrays and scalars */
    test_pattern_a(&result_a);
    printf("Pattern A result: %d\n", result_a);
    
    /* Test Pattern B - Multi-dimensional with explicit clauses */
    test_pattern_b(&result_b);
    printf("Pattern B result: %d\n", result_b);
    
    /* Test Pattern C - Dynamic memory */
    test_pattern_c(&result_c);
    printf("Pattern C result: %d\n", result_c);
    
    /* Test Pattern D - Struct data */
    test_pattern_d(&result_d);
    printf("Pattern D result: %d\n", result_d);
    
    /* Test OpenMP if available */
    #ifdef _OPENMP
    test_omp_pattern(&result_omp);
    printf("OpenMP pattern result: %d\n", result_omp);
    #endif
    
    /* Simple validation */
    int total = result_a + result_b + result_c + result_d;
    #ifdef _OPENMP
    total += result_omp;
    #endif
    
    printf("Total checksum: %d\n", total);
    
    if (total != 0) {
        printf("Test completed successfully.\n");
        return 0;
    } else {
        printf("Warning: All results are zero.\n");
        return 1;
    }
}
