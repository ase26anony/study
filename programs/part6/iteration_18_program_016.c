/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables at different scopes */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Multi-dimensional array */
static int file_static_var = 100;

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j);
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    int host_scalar = 10;
    float host_array[N];
    int result = 0;
    
    /* Initialize data */
    init_matrices();
    for (int i = 0; i < N; i++) {
        host_array[i] = (float)i;
    }
    
    printf("Starting OpenACC partitioning tests...\n");
    
    /* ============================================
       SCENARIO 1: Gang-only partitioning (case 1)
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copy(host_scalar) \
        copyin(global_matrix) copyout(host_array[0:N])
    {
        int gang_private = acc_gang_id;  /* Should be gang partitioned */
        float gang_local[M];
        
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Each gang processes a chunk */
            int gang_idx = gang_private;
            for (int j = 0; j < M; j++) {
                gang_local[j] = global_matrix[i][j] * gang_idx;
            }
            host_array[i] = gang_local[M-1];
        }
    }
    
    /* ============================================
       SCENARIO 2: Vector-only partitioning (case 4)
       ============================================ */
    #pragma acc kernels copy(host_array[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            float vector_private = host_array[i];  /* Should be vector partitioned */
            vector_private *= 2.0f;
            host_array[i] = vector_private + (float)acc_vector_id;
        }
    }
    
    /* ============================================
       SCENARIO 3: Complex nested partitioning
       Targeting cases 0, 2, 3, 5, 6, 7
       ============================================ */
    int outer_var = 50;
    float shared_matrix[GANGS][WORKERS][VECTOR_LEN];
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(outer_var) create(shared_matrix)
    {
        /* Case 0: Gang redundant - same value for all gangs */
        int gang_redundant = global_gang_redundant + file_static_var;
        
        /* Case 2: Worker partitioned */
        int worker_partitioned = acc_worker_id;
        
        /* Case 3: Gang+Worker partitioned */
        int gang_worker_partitioned = acc_gang_id * 100 + acc_worker_id;
        
        /* Case 5: Gang+Vector partitioned */
        float gang_vector_partitioned[VECTOR_LEN];
        
        /* Case 6: Worker+Vector partitioned */
        int worker_vector_partitioned = acc_worker_id * VECTOR_LEN + acc_vector_id;
        
        /* Case 7: Fully partitioned */
        int fully_partitioned = acc_gang_id * 10000 + acc_worker_id * 100 + acc_vector_id;
        
        /* Initialize arrays with partitioning-dependent values */
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN; v++) {
                    /* Complex computation using all partitioned variables */
                    float val = (float)gang_redundant * 0.1f;
                    val += (float)worker_partitioned * 0.2f;
                    val += (float)gang_worker_partitioned * 0.3f;
                    val += (float)worker_vector_partitioned * 0.4f;
                    val += (float)fully_partitioned * 0.5f;
                    
                    /* Stencil-like access pattern */
                    if (v > 0 && v < VECTOR_LEN - 1) {
                        val += gang_vector_partitioned[v-1] * 0.1f;
                        val += gang_vector_partitioned[v+1] * 0.1f;
                    }
                    
                    shared_matrix[g][w][v] = val;
                    gang_vector_partitioned[v] = (float)(g * VECTOR_LEN + v);
                }
            }
        }
        
        /* Conditional partitioning based on runtime values */
        int conditional_var;
        if (gang_redundant > 40) {
            conditional_var = acc_gang_id;  /* Gang partitioned */
        } else {
            conditional_var = acc_vector_id; /* Vector partitioned */
        }
        
        /* Use conditional_var to prevent dead code elimination */
        shared_matrix[0][0][0] += (float)conditional_var;
    }
    
    /* ============================================
       SCENARIO 4: Mixed directives and clauses
       ============================================ */
    float temp_results[WORKERS][VECTOR_LEN];
    int reduction_var = 0;
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copyin(host_array[0:N]) copyout(temp_results) reduction(+:reduction_var)
    {
        /* Firstprivate variable at gang level */
        int firstprivate_gang = acc_gang_id + 1000;
        
        #pragma acc loop gang reduction(+:reduction_var)
        for (int g = 0; g < GANGS; g++) {
            int gang_local = firstprivate_gang + g;
            
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                float worker_local[VECTOR_LEN];
                
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN; v++) {
                    /* Access with stride to complicate analysis */
                    int idx = (g * WORKERS * VECTOR_LEN + w * VECTOR_LEN + v) % N;
                    worker_local[v] = host_array[idx] * (float)(gang_local + w + v);
                    
                    /* Multi-dimensional stencil */
                    if (v > 1 && v < VECTOR_LEN - 2) {
                        worker_local[v] += worker_local[v-2] * 0.25f;
                        worker_local[v] += worker_local[v+2] * 0.25f;
                    }
                    
                    temp_results[w][v] = worker_local[v];
                    reduction_var += (int)worker_local[v];
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 5: Kernels with explicit loop directives
       ============================================ */
    float final_matrix[N][M];
    
    #pragma acc kernels copyin(global_matrix) copyout(final_matrix)
    {
        /* Explicit partitioning directives on loops */
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            float row_local[M];
            
            #pragma acc loop worker
            for (int j = 0; j < M; j += 2) {
                #pragma acc loop vector
                for (int k = 0; k < 2 && j + k < M; k++) {
                    /* Complex access pattern */
                    float val = global_matrix[i][j + k];
                    if (i > 0) val += global_matrix[i-1][j + k] * 0.5f;
                    if (i < N-1) val += global_matrix[i+1][j + k] * 0.5f;
                    if (j + k > 0) val += global_matrix[i][j + k - 1] * 0.3f;
                    if (j + k < M-1) val += global_matrix[i][j + k + 1] * 0.3f;
                    
                    row_local[j + k] = val;
                    final_matrix[i][j + k] = val;
                }
            }
        }
    }
    
    /* Compute checksum to verify execution */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += final_matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Reduction result: %d\n", reduction_var);
    
    return 0;
}
