/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables for different scoping */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Base array for complex access patterns */

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) * 0.001f;
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    /* Local variables with different lifetimes */
    int host_scalar = 100;
    float host_array[N][M];
    double reduction_sum = 0.0;
    
    /* Initialize data */
    init_matrices();
    
    /* Copy base matrix to host array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            host_array[i][j] = global_matrix[i][j];
        }
    }
    
    printf("Starting OpenACC partitioning tests...\n");
    
    /* ============================================
       SCENARIO 1: Gang-only partitioning (case 1)
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copy(host_scalar) \
        copyin(global_matrix) copyout(host_array[0:N][0:M])
    {
        /* gang-private variable - should be gang partitioned */
        int gang_private = acc_gang_id();
        float gang_local[M];
        
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Each gang works on different rows */
            for (int j = 0; j < M; j++) {
                host_array[i][j] = global_matrix[i][j] + gang_private;
            }
        }
    }
    
    /* ============================================
       SCENARIO 2: Vector-only partitioning (case 4)
       ============================================ */
    #pragma acc kernels copy(host_array[0:N][0:M])
    {
        float vector_private;
        
        #pragma acc loop independent vector
        for (int i = 0; i < N; i++) {
            vector_private = sinf((float)i * 0.01f);
            
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                host_array[i][j] += vector_private;
            }
        }
    }
    
    /* ============================================
       SCENARIO 3: Complex nested parallelism 
       Targeting cases 0, 2, 3, 5, 6, 7
       ============================================ */
    {
        /* Variables with mixed attributes */
        int outer_shared = 77;           /* Could become gang redundant */
        int gang_worker_shared[GANGS][WORKERS];
        float fully_partitioned[GANGS][WORKERS][VECTOR_LEN];
        
        #pragma acc data copy(outer_shared) \
            create(gang_worker_shared, fully_partitioned) \
            copy(host_array[0:N][0:M])
        {
            #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
                vector_length(VECTOR_LEN)
            {
                /* Access global variable - should be gang redundant (case 0) */
                int gr = global_gang_redundant;
                
                /* Worker-partitioned variable (case 2) */
                int worker_local = acc_worker_id();
                
                /* Gang+worker partitioned (case 3) */
                int gang_worker_local = acc_gang_id() * 100 + acc_worker_id();
                
                /* Vector partitioned (case 4) - already covered but included */
                int vector_local = acc_vector_id();
                
                /* Gang+vector partitioned (case 5) */
                int gang_vector_local = acc_gang_id() * VECTOR_LEN + acc_vector_id();
                
                /* Worker+vector partitioned (case 6) */
                int worker_vector_local = acc_worker_id() * VECTOR_LEN + acc_vector_id();
                
                /* Fully partitioned (case 7) */
                int fully_local = acc_gang_id() * WORKERS * VECTOR_LEN +
                                 acc_worker_id() * VECTOR_LEN +
                                 acc_vector_id();
                
                /* Store to arrays to prevent optimization */
                gang_worker_shared[acc_gang_id()][acc_worker_id()] = gang_worker_local;
                fully_partitioned[acc_gang_id()][acc_worker_id()][acc_vector_id()] = 
                    (float)fully_local;
                
                /* Complex stencil computation with multi-dimensional access */
                #pragma acc loop gang worker vector collapse(2)
                for (int i = 1; i < N-1; i++) {
                    for (int j = 1; j < M-1; j++) {
                        /* Stencil operation accessing neighbors */
                        float sum = host_array[i-1][j] + host_array[i+1][j] +
                                   host_array[i][j-1] + host_array[i][j+1];
                        
                        /* Mix in partitioned variables */
                        host_array[i][j] = sum * 0.25f + 
                                          (float)(gang_worker_local % 100) * 0.01f +
                                          (float)vector_local * 0.001f;
                    }
                }
                
                /* Conditional partitioning based on runtime values */
                if (gr > 40) {
                    /* Additional partitioned computation path */
                    #pragma acc loop worker
                    for (int i = 0; i < 10; i++) {
                        worker_local += i;
                    }
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 4: Mixed directives with explicit clauses
       ============================================ */
    {
        int explicit_gang_partitioned;
        int explicit_worker_partitioned;
        int explicit_vector_partitioned;
        
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(64) \
            private(explicit_gang_partitioned) \
            private(explicit_worker_partitioned) \
            private(explicit_vector_partitioned) \
            copy(host_array[0:N][0:M])
        {
            explicit_gang_partitioned = acc_gang_id();
            explicit_worker_partitioned = acc_worker_id();
            explicit_vector_partitioned = acc_vector_id();
            
            /* Nested loops with different partitioning directives */
            #pragma acc loop gang independent
            for (int g = 0; g < 4; g++) {
                #pragma acc loop worker independent
                for (int w = 0; w < 2; w++) {
                    #pragma acc loop vector independent
                    for (int v = 0; v < 64; v++) {
                        int idx = g * 128 + w * 64 + v;
                        if (idx < N * M) {
                            int i = idx / M;
                            int j = idx % M;
                            host_array[i][j] += (float)(explicit_gang_partitioned + 
                                                       explicit_worker_partitioned + 
                                                       explicit_vector_partitioned);
                        }
                    }
                }
            }
        }
    }
    
    /* ============================================
       Final reduction and verification
       ============================================ */
    #pragma acc parallel loop reduction(+:reduction_sum) \
        copy(host_array[0:N][0:M]) copy(reduction_sum)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            reduction_sum += host_array[i][j];
        }
    }
    
    printf("Final reduction sum: %f\n", reduction_sum);
    printf("Partitioning test completed.\n");
    
    /* Verify with CPU computation */
    double cpu_sum = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            cpu_sum += host_array[i][j];
        }
    }
    
    printf("CPU verification sum: %f\n", cpu_sum);
    printf("Difference: %e\n", fabs(reduction_sum - cpu_sum));
    
    return 0;
}
