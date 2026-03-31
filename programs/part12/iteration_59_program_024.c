/* Test program to cover all partitioning states in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=disable -fdump-tree-all -fprofile-arcs -ftest-coverage test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Function containing complex OpenACC regions */
void process_data(int n, int m, int p) {
    /* Pattern A: Various scalar variables with different attributes */
    int scalar_private;          /* Likely gang redundant (0) */
    int scalar_firstprivate = 42; /* Likely gang redundant (0) */
    int scalar_reduction = 0;    /* Reduction variable */
    
    /* Pattern B: Multi-dimensional arrays */
    static int static_3d[M][P][8]; /* Static storage */
    int auto_3d[4][8][16];         /* Automatic storage */
    
    /* Pattern C: Dynamic memory and pointers */
    int *dynamic_arr = (int*)malloc(n * sizeof(int));
    int **ptr_to_ptr = (int**)malloc(m * sizeof(int*));
    
    /* Initialize arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < 8; k++) {
                static_3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                auto_3d[i][j][k] = i * j * k;
            }
        }
    }
    
    for (int i = 0; i < n; i++) {
        dynamic_arr[i] = i % 100;
    }
    
    for (int i = 0; i < m; i++) {
        ptr_to_ptr[i] = (int*)malloc(p * sizeof(int));
        for (int j = 0; j < p; j++) {
            ptr_to_ptr[i][j] = i * j;
        }
    }
    
    /* Pattern D: Struct with multiple members */
    struct DataPoint {
        int id;
        float value;
        double precision;
        char tag;
    };
    
    struct DataPoint data_array[N];
    for (int i = 0; i < n; i++) {
        data_array[i].id = i;
        data_array[i].value = i * 0.5f;
        data_array[i].precision = i * 0.12345;
        data_array[i].tag = 'A' + (i % 26);
    }
    
    /* OpenACC parallel region with complex data clauses */
    #pragma acc parallel loop gang worker vector \
        copy(static_3d) \
        copyin(auto_3d) \
        copyout(dynamic_arr[0:n]) \
        create(ptr_to_ptr[0:m]) \
        copy(data_array[0:n]) \
        private(scalar_private) \
        firstprivate(scalar_firstprivate) \
        reduction(+:scalar_reduction)
    for (int i = 0; i < n; i++) {
        /* Nested loops to create complex data flow */
        for (int j = 0; j < m; j++) {
            /* Conditional operations */
            if (i % 2 == 0) {
                /* Access multi-dimensional arrays with varying indices */
                int idx1 = i % M;
                int idx2 = j % P;
                int idx3 = (i + j) % 8;
                
                /* This access pattern may trigger different partitioning */
                scalar_private = static_3d[idx1][idx2][idx3];
                
                /* Worker-level computation */
                for (int k = 0; k < p; k++) {
                    /* Vector-level computation */
                    if (ptr_to_ptr[j]) {
                        dynamic_arr[i] += ptr_to_ptr[j][k] * k;
                    }
                }
            } else {
                /* Different access pattern for odd indices */
                for (int dim1 = 0; dim1 < 4; dim1++) {
                    for (int dim2 = 0; dim2 < 8; dim2++) {
                        /* Access auto_3d with complex indexing */
                        int val = auto_3d[dim1][dim2][i % 16];
                        dynamic_arr[i] += val;
                    }
                }
            }
            
            /* Struct member access with different patterns */
            if (j % 3 == 0) {
                data_array[i].value += j * 0.1f;
                scalar_reduction += data_array[i].id;
            } else if (j % 3 == 1) {
                data_array[i].precision *= 1.01;
            } else {
                data_array[i].tag = 'Z' - (j % 26);
            }
        }
        
        /* Additional computation with conditional vector operations */
        #pragma acc loop vector
        for (int v = 0; v < 32; v++) {
            if (v < 16) {
                dynamic_arr[i] += v;
            } else {
                dynamic_arr[i] -= v;
            }
        }
    }
    
    /* Second OpenACC region with kernels construct */
    #pragma acc kernels \
        copy(static_3d) \
        copy(dynamic_arr[0:n]) \
        copy(data_array[0:n])
    {
        /* Additional nested loops */
        #pragma acc loop gang
        for (int g = 0; g < m; g++) {
            #pragma acc loop worker
            for (int w = 0; w < p; w++) {
                #pragma acc loop vector
                for (int v = 0; v < 8; v++) {
                    /* Complex indexing across multiple arrays */
                    int idx = (g * p * 8) + (w * 8) + v;
                    if (idx < n) {
                        static_3d[g % M][w % P][v] += dynamic_arr[idx];
                        data_array[idx].value += static_3d[g % M][w % P][v] * 0.01f;
                    }
                }
            }
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < m; i++) {
        free(ptr_to_ptr[i]);
    }
    free(ptr_to_ptr);
    free(dynamic_arr);
}

/* OpenMP version for additional coverage */
#ifdef _OPENMP
void process_data_omp(int n, int m, int p) {
    int arr1[N], arr2[N], arr3[N];
    int multi_dim[4][8][16];
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                multi_dim[i][j][k] = i + j * k;
            }
        }
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(to: arr1[0:n], multi_dim) \
        map(from: arr3[0:n]) \
        map(tofrom: arr2[0:n]) \
        reduction(+:arr3[0:n])
    for (int i = 0; i < n; i++) {
        /* Nested loops with conditionals */
        for (int j = 0; j < m; j++) {
            if (i % 2 == 0) {
                for (int k = 0; k < p; k++) {
                    int idx1 = (i + j + k) % 4;
                    int idx2 = (j * k) % 8;
                    int idx3 = (i * k) % 16;
                    arr3[i] += multi_dim[idx1][idx2][idx3];
                }
            } else {
                #pragma omp simd
                for (int v = 0; v < 16; v++) {
                    arr2[i] += arr1[(i + v) % n] * v;
                }
            }
        }
    }
}
#endif

/* Main function to drive execution */
int main() {
    printf("Starting OpenACC/OpenMP partitioning test...\n");
    
    /* Call OpenACC version */
    process_data(N, M, P);
    
    #ifdef _OPENMP
    /* Call OpenMP version if available */
    process_data_omp(N, M/2, P/2);
    #endif
    
    printf("Test completed successfully.\n");
    
    /* Simple validation */
    int validation = 0;
    #pragma acc parallel loop reduction(+:validation)
    for (int i = 0; i < 100; i++) {
        validation += i;
    }
    
    if (validation == 4950) {  /* Sum of 0..99 */
        printf("Validation passed: %d\n", validation);
        return 0;
    } else {
        printf("Validation failed: %d\n", validation);
        return 1;
    }
}
