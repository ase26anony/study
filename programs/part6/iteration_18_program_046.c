/* test_omp_acc_partitioning.c
 * Comprehensive test to trigger all variable partitioning states
 * in GCC's OpenACC/OpenMP offloading support
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables to create complex scoping scenarios */
int global_scalar = 42;
float global_array[N];
static int file_static = 100;

/* Function to initialize arrays */
void init_arrays(float *a, float *b, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(size - i) * 0.1f;
    }
}

/* Main function with complex OpenACC regions */
int main() {
    /* Declare variables at different scopes */
    int host_scalar = 10;
    float host_array[N];
    float matrix[M][N];
    float result[M][N];
    
    /* Initialize data */
    init_arrays(host_array, global_array, N);
    
    /* Initialize matrices */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = (float)(i * N + j) * 0.01f;
            result[i][j] = 0.0f;
        }
    }
    
    printf("Starting OpenACC partitioning test...\n");
    
    /* ============================================
     * Region 1: Gang-only parallel region
     * Targets case 1: "gang partitioned"
     * ============================================ */
    {
        int gang_private = 5;  /* Will be gang partitioned */
        float gang_shared = 3.14f;  /* Shared across gangs */
        
        #pragma acc parallel num_gangs(GANGS) copy(gang_shared) \
            private(gang_private) copyin(host_array[0:N])
        {
            int gang_id = __pgi_gangidx();
            gang_private = gang_id * 10;
            
            #pragma acc loop gang
            for (int i = 0; i < GANGS * 10; i++) {
                /* Simple computation to use variables */
                gang_shared += gang_private * 0.1f;
            }
        }
        printf("Region 1 (gang partitioned) completed: gang_shared = %f\n", gang_shared);
    }
    
    /* ============================================
     * Region 2: Vector-only loop in kernels region
     * Targets case 4: "vector partitioned"
     * ============================================ */
    {
        float vector_private[N];
        
        #pragma acc kernels copyout(vector_private[0:N])
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                /* Each vector lane gets its own copy */
                vector_private[i] = host_array[i] * 2.0f + (float)i;
            }
        }
        printf("Region 2 (vector partitioned) completed\n");
    }
    
    /* ============================================
     * Region 3: Complex nested parallelism
     * Targets combined states (3, 5, 6, 7)
     * ============================================ */
    {
        /* Variables with different partitioning */
        int fully_partitioned = 0;      /* Case 7: fully partitioned */
        int gang_worker_partitioned = 1; /* Case 3: gang+worker partitioned */
        int gang_vector_partitioned = 2; /* Case 5: gang+vector partitioned */
        int worker_vector_partitioned = 3; /* Case 6: worker+vector partitioned */
        
        /* Multi-dimensional array access with stencil */
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
            vector_length(VECTOR_LEN) \
            copy(result[0:M][0:N]) copyin(matrix[0:M][0:N]) \
            private(fully_partitioned) \
            firstprivate(gang_worker_partitioned) \
            private(gang_vector_partitioned) \
            private(worker_vector_partitioned)
        {
            int gang_id = __pgi_gangidx();
            int worker_id = __pgi_workeridx();
            int vector_id = __pgi_vectoridx();
            
            /* Each combination gets unique computation */
            fully_partitioned = gang_id * 1000 + worker_id * 100 + vector_id;
            gang_worker_partitioned += gang_id * 100 + worker_id * 10;
            gang_vector_partitioned += gang_id * 50 + vector_id;
            worker_vector_partitioned += worker_id * 30 + vector_id;
            
            /* Complex nested loops with different partitioning */
            #pragma acc loop gang
            for (int i = 0; i < M; i += M/GANGS) {
                #pragma acc loop worker
                for (int j = 0; j < WORKERS; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < N; k += N/VECTOR_LEN) {
                        /* Stencil computation accessing neighbors */
                        int idx = i * N + k;
                        if (i > 0 && k > 0 && i < M-1 && k < N-1) {
                            result[i][k] = matrix[i-1][k] * 0.25f +
                                          matrix[i+1][k] * 0.25f +
                                          matrix[i][k-1] * 0.25f +
                                          matrix[i][k+1] * 0.25f;
                        } else {
                            result[i][k] = matrix[i][k];
                        }
                        
                        /* Use partitioned variables in computation */
                        result[i][k] += (fully_partitioned + 
                                        gang_worker_partitioned +
                                        gang_vector_partitioned +
                                        worker_vector_partitioned) * 0.0001f;
                    }
                }
            }
        }
        printf("Region 3 (combined partitioning) completed\n");
    }
    
    /* ============================================
     * Region 4: Gang redundant variables
     * Targets case 0: "gang redundant"
     * ============================================ */
    {
        int gang_redundant = 99;  /* Should be gang redundant */
        float redundant_array[GANGS];
        
        /* Use conditional to affect partitioning */
        int use_special_partition = (host_scalar > 5) ? 1 : 0;
        
        #pragma acc parallel num_gangs(GANGS) \
            copy(gang_redundant) copyout(redundant_array[0:GANGS]) \
            copyin(use_special_partition)
        {
            int gang_id = __pgi_gangidx();
            
            /* All gangs see the same value */
            gang_redundant = global_scalar + file_static;
            
            /* Conditional partitioning based on runtime value */
            if (use_special_partition) {
                /* Different partitioning in this path */
                int temp = gang_id * 20;
                gang_redundant += temp;
            }
            
            redundant_array[gang_id] = gang_redundant * 1.5f;
        }
        
        printf("Region 4 (gang redundant) completed: ");
        for (int i = 0; i < GANGS; i++) {
            printf("redundant_array[%d] = %f ", i, redundant_array[i]);
        }
        printf("\n");
    }
    
    /* ============================================
     * Region 5: Worker-only partitioning
     * Targets case 2: "worker partitioned"
     * ============================================ */
    {
        float worker_private[WORKERS * 10];
        
        #pragma acc parallel num_gangs(1) num_workers(WORKERS) \
            copyout(worker_private[0:WORKERS*10])
        {
            int worker_id = __pgi_workeridx();
            
            #pragma acc loop worker
            for (int i = 0; i < WORKERS * 10; i++) {
                if (i / 10 == worker_id) {
                    worker_private[i] = worker_id * 100.0f + (i % 10);
                }
            }
        }
        printf("Region 5 (worker partitioned) completed\n");
    }
    
    /* ============================================
     * Final computation and verification
     * ============================================ */
    {
        float checksum = 0.0f;
        
        #pragma acc parallel loop gang worker vector \
            copyin(result[0:M][0:N]) copyout(checksum) \
            reduction(+:checksum)
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < N; j++) {
                checksum += result[i][j];
            }
        }
        
        printf("Final checksum of result matrix: %f\n", checksum);
        printf("Test completed successfully!\n");
    }
    
    return 0;
}
