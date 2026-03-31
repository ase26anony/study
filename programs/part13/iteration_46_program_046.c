/* gcc -O2 -fopenacc -foffload=nvptx-none -fdump-tree-omplower -o test test.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force partitioning decisions */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with complex OpenACC data partitioning */
void process_data(int N, int M, int argc) {
    int src[512][512];
    int dst[512][512];
    int total_sum = 0;
    int tmp;
    
    /* Initialize arrays with volatile-like behavior using argc */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            src[i][j] = (i * j + argc) % 100;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with explicit data management */
    #pragma acc data copy(src, dst) copyout(total_sum)
    {
        /* Complex parallel region with multiple partitioning scenarios */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp) reduction(+:total_sum) \
                firstprivate(N, M)
        for (int i = 1; i < N-1; i++) {
            for (int j = 1; j < M-1; j++) {
                /* Conditional data access to force different broadcast strategies */
                if ((i + j) % 3 == 0) {
                    tmp = src[i-1][j] + src[i+1][j];
                } else if ((i + j) % 3 == 1) {
                    tmp = src[i][j-1] + src[i][j+1];
                } else {
                    tmp = src[i][j];
                }
                
                /* Data-dependent computation with reduction */
                dst[i][j] = tmp * 2;
                if (dst[i][j] > 50) {
                    total_sum += dst[i][j];
                }
            }
        }
        
        /* Second kernels region with different partitioning approach */
        int row_sums[512] = {0};
        #pragma acc kernels copy(row_sums) copyin(dst)
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                int row_sum = 0;
                #pragma acc loop worker reduction(+:row_sum)
                for (int j = 0; j < M; j++) {
                    /* Access global device data */
                    row_sum += dst[i][j] + global_matrix[i % 256][j % 256];
                }
                row_sums[i] = row_sum;
            }
        }
        
        /* Update global sum using another parallel region */
        #pragma acc parallel loop gang reduction(+:global_sum) \
                copy(global_sum) copyin(row_sums)
        for (int i = 0; i < N; i++) {
            global_sum += row_sums[i];
        }
    }
    
    /* Mixed OpenMP section to stress multi-runtime handling */
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            /* Simple host-side computation */
            dst[i][j] += i + j;
        }
    }
    
    printf("Total sum: %d, Global sum: %d\n", total_sum, global_sum);
}

/* Another function with vector-only partitioning */
void vector_operations(int size, int seed) {
    float array1[1024], array2[1024], result[1024];
    
    /* Initialize with seed to prevent constant folding */
    for (int i = 0; i < size; i++) {
        array1[i] = (i * seed) % 100 * 0.1f;
        array2[i] = (i + seed) % 50 * 0.2f;
    }
    
    /* Vector-partitioned computation */
    #pragma acc parallel loop vector_length(128) \
            copyin(array1, array2) copyout(result)
    for (int i = 0; i < size; i++) {
        /* Complex conditional vector operations */
        if (i % 4 == 0) {
            result[i] = array1[i] * array2[i];
        } else if (i % 4 == 1) {
            result[i] = array1[i] + array2[i];
        } else if (i % 4 == 2) {
            result[i] = array1[i] - array2[i];
        } else {
            result[i] = array1[i] / (array2[i] + 1.0f);
        }
    }
    
    /* Worker+vector partitioned reduction */
    float sum = 0.0f;
    #pragma acc parallel loop gang worker vector reduction(+:sum) \
            copyin(result)
    for (int i = 0; i < size; i++) {
        sum += result[i];
    }
    
    printf("Vector ops sum: %.2f\n", sum);
}

int main(int argc, char **argv) {
    /* Dynamic bounds to prevent constant folding */
    int N = argc > 1 ? atoi(argv[1]) : 256;
    int M = argc > 2 ? atoi(argv[2]) : 256;
    
    /* Initialize global matrix on device */
    #pragma acc parallel loop collapse(2) copyout(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 100;
        }
    }
    
    /* Process with complex partitioning */
    process_data(N, M, argc);
    
    /* Additional vector operations */
    vector_operations(1024, argc);
    
    /* Fully partitioned example with 3D array */
    {
        int cube[64][64][64];
        int cube_sum = 0;
        
        #pragma acc parallel loop collapse(3) gang worker vector \
                reduction(+:cube_sum) copyout(cube)
        for (int x = 0; x < 64; x++) {
            for (int y = 0; y < 64; y++) {
                for (int z = 0; z < 64; z++) {
                    cube[x][y][z] = (x * y * z + argc) % 100;
                    if ((x + y + z) % 2 == 0) {
                        cube_sum += cube[x][y][z];
                    }
                }
            }
        }
        
        printf("Cube sum: %d\n", cube_sum);
    }
    
    return 0;
}
