/* Test program to cover partitioning state mapping in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Different scalar and array types with various data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* 1D arrays with different mappings */
    int arr1[N];            /* Will likely be gang partitioned */
    int arr2[N];            /* Worker partitioned */
    int arr3[N];            /* Vector partitioned */
    int arr4[N];            /* Fully partitioned */
    
    /* Pattern B: Multi-dimensional arrays */
    int md_arr[M][P];       /* Complex partitioning */
    int md_arr2[M][P][8];   /* 3D array for more complex analysis */
    
    /* Pattern C: Pointer-based dynamic memory */
    int *dyn_arr1 = (int*)malloc(N * sizeof(int));
    int *dyn_arr2 = (int*)malloc(N * sizeof(int));
    
    /* Pattern D: Struct/aggregate data */
    struct Data {
        int x;
        float y;
        double z;
    };
    struct Data struct_arr[N];
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        arr4[i] = i * 4;
        dyn_arr1[i] = i * 5;
        dyn_arr2[i] = i * 6;
        
        struct_arr[i].x = i;
        struct_arr[i].y = i * 1.5f;
        struct_arr[i].z = i * 2.5;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            md_arr[i][j] = i * j;
            for (k = 0; k < 8; k++) {
                md_arr2[i][j][k] = i * j * k;
            }
        }
    }
    
    /* Complex OpenACC parallel region with multiple data clauses and nesting */
    #pragma acc parallel loop copy(arr1[0:N]) copyin(arr2[0:N]) \
        copyout(arr3[0:N]) create(arr4[0:N]) \
        copy(md_arr[0:M][0:P]) copy(md_arr2[0:M][0:P][0:8]) \
        copyin(dyn_arr1[0:N]) copyout(dyn_arr2[0:N]) \
        copy(struct_arr[0:N]) \
        reduction(+:i) private(j, k)
    for (i = 0; i < N; i++) {
        /* Pattern A: Different access patterns for different arrays */
        arr1[i] = arr1[i] * 2;          /* Simple gang-level operation */
        
        /* Nested loops to create complex data flow */
        for (j = 0; j < 8; j++) {
            /* Pattern B: Multi-dimensional array access with conditional */
            if (i < M && j < P) {
                md_arr[i % M][j % P] += arr1[i];
                
                /* Further nesting for vector-level operations */
                for (k = 0; k < 4; k++) {
                    /* Pattern B: 3D array access */
                    if (i < M && j < P && k < 8) {
                        md_arr2[i % M][j % P][k] += arr2[i] * k;
                    }
                }
            }
            
            /* Pattern C: Pointer arithmetic and conditional access */
            if (j % 2 == 0) {
                dyn_arr1[i] += dyn_arr2[(i + j) % N];
            } else {
                dyn_arr2[i] += dyn_arr1[(i - j + N) % N];
            }
        }
        
        /* Pattern D: Struct member access with conditional */
        if (arr3[i] > 0) {
            struct_arr[i].x = arr4[i] + struct_arr[i].x;
            struct_arr[i].y = struct_arr[i].y * 2.0f;
            struct_arr[i].z = struct_arr[i].z / 2.0;
        } else {
            struct_arr[i].x = arr4[i] - struct_arr[i].x;
            struct_arr[i].y = struct_arr[i].y / 2.0f;
            struct_arr[i].z = struct_arr[i].z * 2.0;
        }
        
        /* Complex conditional that depends on multiple variables */
        int temp = 0;
        #pragma acc loop vector reduction(+:temp)
        for (j = 0; j < 16; j++) {
            if ((i + j) % 3 == 0) {
                temp += arr1[(i + j) % N];
            } else if ((i + j) % 3 == 1) {
                temp += arr2[(i + j) % N];
            } else {
                temp += arr3[(i + j) % N];
            }
        }
        arr4[i] = temp;
    }
    
    /* Additional OpenACC kernels region with different partitioning */
    #pragma acc kernels copy(arr1[0:N], arr2[0:N]) \
        copyin(arr3[0:N]) copyout(arr4[0:N]) \
        present(md_arr[0:M][0:P])
    {
        #pragma acc loop gang
        for (i = 0; i < M; i++) {
            #pragma acc loop worker
            for (j = 0; j < P; j++) {
                int sum = 0;
                #pragma acc loop vector reduction(+:sum)
                for (k = 0; k < 8; k++) {
                    sum += md_arr2[i][j][k];
                }
                md_arr[i][j] = sum;
                
                /* Cross-access between different arrays */
                if (i * P + j < N) {
                    arr1[i * P + j] += md_arr[i][j];
                    arr2[i * P + j] -= md_arr[i][j] / 2;
                }
            }
        }
        
        /* Independent loop with different partitioning */
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            /* Access with stride to trigger different partitioning */
            for (j = 0; j < 4; j++) {
                int idx = (i + j * 256) % N;
                arr3[idx] = arr1[i] + arr2[idx];
            }
        }
    }
    
    /* Clean up */
    free(dyn_arr1);
    free(dyn_arr2);
}

/* OpenMP version to trigger different code paths */
void test_openmp_partitioning() {
    int i, j, k;
    int omp_arr1[N], omp_arr2[N], omp_arr3[N];
    int omp_md_arr[M][P];
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        omp_arr1[i] = i;
        omp_arr2[i] = i * 2;
        omp_arr3[i] = i * 3;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            omp_md_arr[i][j] = i * j;
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_arr1[0:N]) map(to: omp_arr2[0:N]) \
        map(from: omp_arr3[0:N]) map(tofrom: omp_md_arr[0:M][0:P]) \
        private(i, j, k)
    for (i = 0; i < N; i++) {
        /* Nested operations */
        for (j = 0; j < 8; j++) {
            if (i < M && j < P) {
                omp_md_arr[i % M][j % P] += omp_arr1[i] * j;
            }
            
            /* Conditional with reduction-like pattern */
            int local_sum = 0;
            #pragma omp simd reduction(+:local_sum)
            for (k = 0; k < 4; k++) {
                local_sum += omp_arr2[(i + k) % N];
            }
            omp_arr3[i] = local_sum;
        }
        
        /* Complex conditional chain */
        if (omp_arr1[i] % 2 == 0) {
            #pragma omp simd
            for (j = 0; j < 4; j++) {
                omp_arr2[(i + j) % N] += omp_arr3[i];
            }
        } else if (omp_arr1[i] % 3 == 0) {
            #pragma omp simd
            for (j = 0; j < 4; j++) {
                omp_arr2[(i + j) % N] -= omp_arr3[i];
            }
        }
    }
    
    /* Additional nested parallel region */
    #pragma omp target teams distribute parallel for collapse(2) \
        map(tofrom: omp_md_arr) private(i, j)
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            /* Access pattern that varies across dimensions */
            for (k = 0; k < 4; k++) {
                omp_md_arr[i][j] += omp_arr1[(i * P + j + k) % N];
            }
        }
    }
}

/* Helper function with different variable scopes */
void helper_function(int *input, int *output, int size) {
    int local_arr[64];
    
    #pragma acc parallel loop present(input[0:size], output[0:size]) \
        copy(local_arr[0:64])
    for (int i = 0; i < size; i++) {
        /* Use local array with indexing that depends on loop */
        local_arr[i % 64] = input[i] % 256;
        
        /* Complex output calculation */
        output[i] = 0;
        for (int j = 0; j < 8; j++) {
            output[i] += local_arr[(i + j) % 64] * j;
        }
        
        /* Conditional store back to input */
        if (output[i] > 128) {
            input[i] = output[i] / 2;
        }
    }
}

int main() {
    printf("Testing OpenACC partitioning states...\n");
    test_openacc_partitioning();
    
    printf("Testing OpenMP partitioning states...\n");
    test_openmp_partitioning();
    
    /* Additional test with helper function */
    int size = 512;
    int *in_arr = (int*)malloc(size * sizeof(int));
    int *out_arr = (int*)malloc(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        in_arr[i] = i;
    }
    
    #pragma acc data copyin(in_arr[0:size]) copyout(out_arr[0:size])
    {
        helper_function(in_arr, out_arr, size);
    }
    
    /* Verification */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += out_arr[i];
    }
    printf("Verification sum: %d\n", sum);
    
    free(in_arr);
    free(out_arr);
    
    printf("Test completed successfully.\n");
    return 0;
}
