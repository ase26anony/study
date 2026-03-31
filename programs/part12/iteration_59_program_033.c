/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's
 * omp-oacc-neuter-broadcast.cc switch statement (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 16

/* Pattern A: Various scalar and array variables with different data clauses */
void test_openacc_partitioning() {
    int i, j, k;
    
    /* Different array dimensions to trigger various partitioning */
    int scalar = 42;                     /* Likely gang redundant (0) */
    int arr1d[N];                        /* 1D array */
    int arr2d[N][M];                     /* 2D array */
    int arr3d[N][M][P];                  /* 3D array - complex partitioning */
    int *dynamic_arr;                    /* Dynamic memory */
    int reduction_var = 0;               /* Reduction variable */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        arr1d[i] = i;
        for (j = 0; j < M; j++) {
            arr2d[i][j] = i * j;
            for (k = 0; k < P; k++) {
                arr3d[i][j][k] = i * j * k;
            }
        }
    }
    
    dynamic_arr = (int*)malloc(N * M * sizeof(int));
    for (i = 0; i < N * M; i++) {
        dynamic_arr[i] = i * 2;
    }
    
    /* OpenACC parallel region with complex data clauses and nesting */
    #pragma acc parallel loop gang vector copy(scalar, arr1d, arr2d) \
        copyin(arr3d) copyout(dynamic_arr[0:N*M]) reduction(+:reduction_var)
    for (i = 0; i < N; i++) {
        /* Nested loops to create complex data flow */
        #pragma acc loop worker
        for (j = 0; j < M; j++) {
            int temp = arr2d[i][j];
            
            /* Conditional operations */
            if (temp % 2 == 0) {
                #pragma acc loop vector
                for (k = 0; k < P; k++) {
                    /* Access 3D array with all three indices varying */
                    arr3d[i][j][k] += scalar;
                    
                    /* Vector-level computation */
                    if (k % 3 == 0) {
                        arr3d[i][j][k] *= 2;
                    }
                }
                
                /* Worker-level computation */
                arr2d[i][j] = temp * 3;
                
                /* Gang-level reduction */
                reduction_var += arr2d[i][j];
            } else {
                /* Different computation path */
                #pragma acc loop vector
                for (k = 0; k < P; k++) {
                    arr3d[i][j][k] -= scalar;
                }
                arr2d[i][j] = temp / 2;
            }
            
            /* Write to dynamic array with complex indexing */
            dynamic_arr[i * M + j] = arr2d[i][j] + arr1d[i];
        }
        
        /* Gang-level operation on 1D array */
        arr1d[i] = i + reduction_var % 100;
    }
    
    /* Verify results to ensure code isn't optimized away */
    int checksum = 0;
    for (i = 0; i < N; i++) {
        checksum += arr1d[i] % 256;
        for (j = 0; j < M; j++) {
            checksum += arr2d[i][j] % 256;
            checksum += dynamic_arr[i * M + j] % 256;
        }
    }
    
    printf("OpenACC checksum: %d\n", checksum);
    free(dynamic_arr);
}

/* Pattern B: OpenMP target region with teams/distribute */
void test_openmp_partitioning() {
    int i, j, k;
    
    /* Different variable types and storage classes */
    static int static_var = 100;         /* Static storage */
    const int const_var = 200;           /* Constant */
    int private_var;                     /* Will be made private */
    int firstprivate_var = 300;          /* Firstprivate */
    
    int arr_shared[N][M];                /* Shared array */
    int arr_private[N];                  /* Private array */
    int arr_firstprivate[M];             /* Firstprivate array */
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        arr_private[i] = i * 2;
        for (j = 0; j < M; j++) {
            arr_shared[i][j] = i + j;
        }
    }
    for (j = 0; j < M; j++) {
        arr_firstprivate[j] = j * 3;
    }
    
    /* Complex OpenMP target region with multiple nesting levels */
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr_shared) map(to: arr_firstprivate) \
        map(from: arr_private) private(private_var) \
        firstprivate(firstprivate_var) shared(static_var)
    for (i = 0; i < N; i++) {
        private_var = i;
        
        /* Teams level - gang partitioning */
        #pragma omp parallel for collapse(2)
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                /* Complex computation involving all variable types */
                int temp = arr_shared[i][j];
                
                /* Conditional with different partitioning requirements */
                if (temp % 4 == 0) {
                    /* Vector-like operation */
                    arr_shared[i][j] = temp + const_var + firstprivate_var;
                } else if (temp % 4 == 1) {
                    /* Worker-like operation */
                    arr_shared[i][j] = temp * static_var - private_var;
                } else if (temp % 4 == 2) {
                    /* Gang-like operation */
                    arr_shared[i][j] = temp / (firstprivate_var + 1);
                } else {
                    /* Mixed partitioning */
                    arr_shared[i][j] = (temp + private_var) * (const_var % 10);
                }
                
                /* Update private array with conditional */
                if (j % 2 == 0 && k % 3 == 0) {
                    #pragma omp atomic
                    arr_private[i] += arr_shared[i][j] % 100;
                }
            }
        }
        
        /* Update static variable (shared across teams) */
        #pragma omp atomic
        static_var += i % 10;
    }
    
    /* Verification */
    int omp_checksum = static_var;
    for (i = 0; i < N; i++) {
        omp_checksum += arr_private[i] % 256;
        for (j = 0; j < M; j++) {
            omp_checksum += arr_shared[i][j] % 256;
        }
    }
    printf("OpenMP checksum: %d\n", omp_checksum);
}

/* Pattern C: Mixed OpenACC/OpenMP with structs (C++ style in C) */
typedef struct {
    int x;
    float y;
    double z;
    int arr[10];
} ComplexStruct;

void test_struct_partitioning() {
    int i, j;
    ComplexStruct struct_array[N];
    
    /* Initialize struct array */
    for (i = 0; i < N; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 1.5f;
        struct_array[i].z = i * 2.5;
        for (j = 0; j < 10; j++) {
            struct_array[i].arr[j] = i * j;
        }
    }
    
    /* OpenACC with struct array - each member may have different partitioning */
    #pragma acc parallel loop copy(struct_array)
    for (i = 0; i < N; i++) {
        /* Access different struct members with different patterns */
        struct_array[i].x *= 2;                     /* Simple gang operation */
        
        #pragma acc loop worker
        for (j = 0; j < 10; j++) {
            /* Worker-level operation on array member */
            struct_array[i].arr[j] += struct_array[i].x;
            
            /* Conditional vector operations */
            if (j % 3 == 0) {
                struct_array[i].y += 1.0f;
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    struct_array[i].z += 0.25 * k;
                }
            }
        }
        
        /* Mixed partitioning for floating point members */
        struct_array[i].y = struct_array[i].y * 0.9f + struct_array[i].x;
        struct_array[i].z = struct_array[i].z / (struct_array[i].x + 1);
    }
    
    /* Verification */
    double struct_checksum = 0;
    for (i = 0; i < N; i++) {
        struct_checksum += struct_array[i].x + struct_array[i].y + struct_array[i].z;
        for (j = 0; j < 10; j++) {
            struct_checksum += struct_array[i].arr[j];
        }
    }
    printf("Struct checksum: %.2f\n", struct_checksum);
}

/* Pattern D: Pointer-based accesses with variable-length data */
void test_pointer_partitioning() {
    int i, j;
    int *ptr_array[5];
    int sizes[5] = {N, N/2, N/4, N*2, N};
    
    /* Allocate arrays of different sizes */
    for (i = 0; i < 5; i++) {
        ptr_array[i] = (int*)malloc(sizes[i] * sizeof(int));
        for (j = 0; j < sizes[i]; j++) {
            ptr_array[i][j] = i * 1000 + j;
        }
    }
    
    /* OpenACC with multiple pointer arrays - complex partitioning analysis */
    #pragma acc parallel loop gang copy(ptr_array[0:5][0:sizes[0]]) \
        copyin(ptr_array[1:4][0:sizes[1]]) create(ptr_array[2:3][0:sizes[2]])
    for (i = 0; i < sizes[0]; i++) {
        /* Access different pointer arrays with different indices */
        int idx1 = i % sizes[1];
        int idx2 = i % sizes[2];
        int idx3 = i % sizes[3];
        int idx4 = i % sizes[4];
        
        /* Complex pointer arithmetic and conditional accesses */
        if (i % 3 == 0) {
            #pragma acc loop worker
            for (j = 0; j < 8; j++) {
                ptr_array[1][idx1] += ptr_array[0][i] * j;
                
                /* Vector operations on some arrays */
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    ptr_array[2][idx2 + k % sizes[2]] += k;
                    ptr_array[3][idx3] -= ptr_array[4][idx4] / (k + 1);
                }
            }
        } else {
            ptr_array[0][i] = ptr_array[1][idx1] + ptr_array[2][idx2];
        }
        
        /* Reduction-like operation across pointer arrays */
        ptr_array[4][idx4] = (ptr_array[0][i] + ptr_array[1][idx1] + 
                             ptr_array[2][idx2] + ptr_array[3][idx3]) / 4;
    }
    
    /* Verification and cleanup */
    int ptr_checksum = 0;
    for (i = 0; i < 5; i++) {
        for (j = 0; j < sizes[i] && j < 100; j++) { /* Check first 100 elements */
            ptr_checksum += ptr_array[i][j] % 256;
        }
        free(ptr_array[i]);
    }
    printf("Pointer checksum: %d\n", ptr_checksum);
}

int main() {
    printf("Testing all partitioning states in neuter-broadcast pass...\n");
    
    /* Call all test functions to trigger different partitioning scenarios */
    test_openacc_partitioning();
    test_openmp_partitioning();
    test_struct_partitioning();
    test_pointer_partitioning();
    
    printf("All tests completed.\n");
    return 0;
}
