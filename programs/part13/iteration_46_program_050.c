/* This program is designed to trigger execution of the partitioning type
   string lookup function in GCC's omp-oacc-neuter-broadcast.cc (lines 335-343).
   It uses complex OpenACC constructs with various data clauses and partitioning
   scenarios to force the compiler to analyze and represent data partitioning. */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to stress partitioning decisions */
#pragma acc declare create(global_matrix)
static int global_matrix[256][256];

/* Function to perform stencil operation with complex partitioning */
void stencil_operation(int N, int src[][512], int dst[][512], int *total_sum) {
    int i, j;
    int local_sum = 0;
    
    /* Complex OpenACC parallel region with explicit partitioning clauses
       This should trigger analysis of gang+worker+vector partitioning */
    #pragma acc parallel loop gang worker vector collapse(2) \
            private(i, j) firstprivate(N) reduction(+:local_sum) \
            copyin(src[0:N][0:N]) copyout(dst[0:N][0:N])
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < N-1; j++) {
            int temp;  /* Private variable for each thread */
            
            /* Conditional data access to force broadcast analysis */
            if ((i + j) % 3 == 0) {
                /* Stencil computation with data-dependent access */
                temp = src[i-1][j] + src[i+1][j] + src[i][j-1] + src[i][j+1];
            } else if ((i + j) % 3 == 1) {
                temp = src[i][j] * 2 - src[i-1][j-1];
            } else {
                temp = src[i][j] + src[i+1][j+1];
            }
            
            dst[i][j] = temp;
            
            /* Reduction with conditional participation */
            if (temp > N) {
                local_sum += temp % 100;
            }
        }
    }
    
    *total_sum += local_sum;
}

/* Function with different partitioning approach */
void rowwise_sums(int N, int matrix[][512], int row_sums[]) {
    int i, j;
    
    /* Different partitioning: only gang+vector without worker */
    #pragma acc kernels loop gang vector independent \
            copy(matrix[0:N][0:N]) copyout(row_sums[0:N])
    for (i = 0; i < N; i++) {
        int row_sum = 0;
        #pragma acc loop vector reduction(+:row_sum)
        for (j = 0; j < N; j++) {
            /* Access global device data to stress data management */
            row_sum += matrix[i][j] + global_matrix[i%256][j%256];
        }
        row_sums[i] = row_sum;
    }
}

/* Initialize global matrix using OpenMP (mixing pragma types) */
void init_global_matrix() {
    int i, j;
    
    /* Pure OpenMP parallel region outside OpenACC */
    #pragma omp parallel for private(i, j) collapse(2)
    for (i = 0; i < 256; i++) {
        for (j = 0; j < 256; j++) {
            global_matrix[i][j] = (i * 17 + j * 13) % 97;
        }
    }
    
    /* Copy initialized matrix to device */
    #pragma acc update device(global_matrix)
}

int main(int argc, char *argv[]) {
    /* Dynamic bounds to prevent constant folding */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    if (N > 512) N = 512;
    if (N < 64) N = 64;
    
    /* Multi-dimensional arrays for complex access patterns */
    int (*src)[512] = malloc(sizeof(int) * 512 * 512);
    int (*dst)[512] = malloc(sizeof(int) * 512 * 512);
    int *row_sums = malloc(sizeof(int) * N);
    
    /* Volatile variable to prevent optimization */
    volatile int force_computation = 1;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            src[i][j] = (i * 3 + j * 7) % 113;
            dst[i][j] = 0;
        }
    }
    
    /* Initialize global matrix using mixed OpenMP/OpenACC */
    init_global_matrix();
    
    int total_sum = 0;
    
    /* Main OpenACC data region with complex partitioning scenarios */
    #pragma acc data copy(src[0:N][0:N], dst[0:N][0:N]) \
                     create(row_sums[0:N]) copy(total_sum)
    {
        /* First compute region: gang+worker+vector partitioning */
        if (force_computation) {
            stencil_operation(N, src, dst, &total_sum);
        }
        
        /* Second compute region: different partitioning approach */
        #pragma acc parallel loop gang worker num_gangs(8) vector_length(32) \
                present(src, dst, row_sums) reduction(+:total_sum)
        for (int i = 0; i < N; i += 2) {
            int temp_sum = 0;
            #pragma acc loop worker vector reduction(+:temp_sum)
            for (int j = 0; j < N; j++) {
                /* Another conditional pattern */
                if (dst[i][j] > 50) {
                    temp_sum += src[i][j] - dst[i][j];
                } else {
                    temp_sum += dst[i][j] * 2;
                }
            }
            total_sum += temp_sum;
        }
        
        /* Third region: worker+vector partitioning */
        rowwise_sums(N, dst, row_sums);
        
        /* Additional nested parallelism with collapse */
        #pragma acc parallel loop gang(4) worker(2) vector(16) collapse(2) \
                reduction(+:total_sum) private(src, dst)
        for (int i = 0; i < N/2; i++) {
            for (int j = 0; j < N/2; j++) {
                int idx_i = i * 2;
                int idx_j = j * 2;
                
                /* Complex reduction with array accesses */
                total_sum += dst[idx_i][idx_j] % 17;
                total_sum -= src[idx_i+1][idx_j+1] % 13;
            }
        }
    }
    
    /* Final OpenMP section to mix pragma types further */
    int host_sum = 0;
    #pragma omp parallel for reduction(+:host_sum)
    for (int i = 0; i < N; i++) {
        host_sum += row_sums[i] % 71;
    }
    
    total_sum += host_sum;
    
    /* Verification output to prevent dead code elimination */
    printf("Partitioning test result: %d (N=%d)\n", total_sum, N);
    
    /* Cleanup */
    free(src);
    free(dst);
    free(row_sums);
    
    return 0;
}
