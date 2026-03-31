/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables that can have different partitioning states */
int global_gang_redundant = 42;          /* Should become case 0 */
float global_matrix[N][M];               /* Complex multi-dimensional array */
static double static_partial[M];         /* Static storage */

/* Function prototypes */
void gang_partitioned_region(int size);
void worker_vector_partitioned(int size);
void fully_partitioned_computation(void);
void mixed_partitioning_with_condition(int threshold);

int main(void) {
    int i, j;
    
    /* Initialize global matrix with test data */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) * 0.01f;
        }
    }
    
    /* Initialize static array */
    for (j = 0; j < M; j++) {
        static_partial[j] = sin(j * 0.1);
    }
    
    printf("Starting OpenACC partitioning tests...\n");
    
    /* 1. GANG-ONLY PARALLEL REGION (targeting case 1) */
    #pragma acc parallel num_gangs(GANGS) copyout(global_matrix[0:N][0:M])
    {
        int gang_private;  /* Should be gang partitioned - case 1 */
        int gang_id = __pgi_gangidx();
        
        #pragma acc loop gang private(gang_private)
        for (i = 0; i < GANGS; i++) {
            gang_private = gang_id * 1000 + i;
            /* Access global_gang_redundant - should be case 0 */
            gang_private += global_gang_redundant;
        }
    }
    
    /* 2. VECTOR-ONLY LOOP in KERNELS region (targeting case 4) */
    #pragma acc kernels copy(global_matrix[0:N][0:M])
    {
        float vector_private[N];  /* Should be vector partitioned - case 4 */
        
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            vector_private[i] = global_matrix[i][0] * 2.0f;
            /* Stencil-like access pattern */
            if (i > 0 && i < N-1) {
                vector_private[i] += global_matrix[i-1][0] + global_matrix[i+1][0];
            }
        }
        
        /* Write back results */
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            global_matrix[i][0] = vector_private[i];
        }
    }
    
    /* 3. COMPLEX REGION WITH ALL CLAUSES (targeting combined states) */
    gang_partitioned_region(M);
    
    /* 4. WORKER+VECTOR PARTITIONING (targeting case 6) */
    worker_vector_partitioned(N);
    
    /* 5. FULLY PARTITIONED COMPUTATION (targeting case 7) */
    fully_partitioned_computation();
    
    /* 6. CONDITIONAL PARTITIONING with runtime-dependent behavior */
    mixed_partitioning_with_condition(500);
    
    /* Verify results with a simple checksum */
    double checksum = 0.0;
    #pragma acc parallel loop gang worker vector reduction(+:checksum) \
                copyin(global_matrix[0:N][0:M])
    for (i = 0; i < N; i++) {
        #pragma acc loop vector
        for (j = 0; j < M; j++) {
            checksum += global_matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Partitioning tests completed.\n");
    
    return 0;
}

/* Function demonstrating gang-partitioned variables (case 1) */
void gang_partitioned_region(int size) {
    int i, j;
    float gang_local[M];  /* Should be gang partitioned */
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copyin(static_partial[0:size]) copyout(gang_local[0:size])
    {
        int worker_local;  /* Should be worker partitioned - case 2 */
        int vector_local;  /* Should be vector partitioned - case 4 */
        
        /* GANG+WORKER PARTITIONED (case 3) */
        int gang_worker_shared = 0;
        
        #pragma acc loop gang
        for (i = 0; i < GANGS; i++) {
            gang_local[i] = 0.0f;
            
            #pragma acc loop worker
            for (j = 0; j < WORKERS; j++) {
                worker_local = j * 10;
                
                /* GANG+VECTOR PARTITIONED (case 5) */
                int gang_vector_var = i * VECTOR_LEN;
                
                #pragma acc loop vector
                for (int k = 0; k < VECTOR_LEN; k++) {
                    vector_local = k;
                    /* WORKER+VECTOR PARTITIONED (case 6) */
                    int worker_vector_var = worker_local + vector_local;
                    
                    /* FULLY PARTITIONED (case 7) */
                    int fully_partitioned = i * WORKERS * VECTOR_LEN + 
                                           j * VECTOR_LEN + k;
                    
                    /* Complex computation accessing multiple scopes */
                    gang_local[i] += static_partial[(i + j + k) % size] * 
                                    (float)fully_partitioned;
                    gang_worker_shared += worker_vector_var;
                    gang_vector_var += vector_local;
                }
                
                /* Access gang_worker_shared across vector iterations */
                worker_local += gang_worker_shared;
            }
        }
    }
}

/* Function demonstrating worker+vector partitioning (case 6) */
void worker_vector_partitioned(int size) {
    int i;
    float temp[N];
    
    #pragma acc parallel num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copyout(temp[0:size])
    {
        /* Variables that should be worker+vector partitioned */
        int worker_vec_private;
        float worker_vec_array[VECTOR_LEN];
        
        #pragma acc loop worker
        for (i = 0; i < WORKERS; i++) {
            worker_vec_private = i * 100;
            
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN; v++) {
                worker_vec_array[v] = (float)(worker_vec_private + v);
                
                /* Multi-dimensional array access with stencil */
                int idx = (i * VECTOR_LEN + v) % size;
                if (idx > 0 && idx < size-1) {
                    temp[idx] = global_matrix[idx][0] + 
                               global_matrix[idx-1][0] + 
                               global_matrix[idx+1][0] +
                               worker_vec_array[v];
                }
            }
        }
    }
}

/* Function demonstrating fully partitioned computation (case 7) */
void fully_partitioned_computation(void) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
                vector_length(VECTOR_LEN) copy(global_matrix[0:N][0:M])
    {
        /* This variable should be fully partitioned across all levels */
        int fully_private;
        
        #pragma acc loop gang
        for (i = 0; i < GANGS; i++) {
            #pragma acc loop worker
            for (j = 0; j < WORKERS; j++) {
                #pragma acc loop vector
                for (k = 0; k < VECTOR_LEN; k++) {
                    /* Calculate unique index for each parallel element */
                    fully_private = i * (WORKERS * VECTOR_LEN) + 
                                   j * VECTOR_LEN + k;
                    
                    /* Map to matrix indices */
                    int row = fully_private % N;
                    int col = (fully_private / N) % M;
                    
                    if (row < N && col < M) {
                        /* Complex stencil computation */
                        float sum = 0.0f;
                        int neighbors = 0;
                        
                        for (int dr = -1; dr <= 1; dr++) {
                            for (int dc = -1; dc <= 1; dc++) {
                                int nr = row + dr;
                                int nc = col + dc;
                                
                                if (nr >= 0 && nr < N && nc >= 0 && nc < M) {
                                    sum += global_matrix[nr][nc];
                                    neighbors++;
                                }
                            }
                        }
                        
                        if (neighbors > 0) {
                            global_matrix[row][col] = sum / neighbors + 
                                                     (float)fully_private * 0.001f;
                        }
                    }
                }
            }
        }
    }
}

/* Function with conditional partitioning */
void mixed_partitioning_with_condition(int threshold) {
    int i, j;
    float conditional_array[N];
    
    /* Runtime condition that affects partitioning */
    int use_worker_level = (threshold > 250) ? 1 : 0;
    
    #pragma acc parallel num_gangs(GANGS) copyout(conditional_array[0:N])
    {
        int partitioning_mode = use_worker_level;
        
        #pragma acc loop gang
        for (i = 0; i < GANGS; i++) {
            float gang_result = 0.0f;
            
            /* Conditional worker-level parallelism */
            if (partitioning_mode) {
                #pragma acc loop worker
                for (j = 0; j < WORKERS; j++) {
                    int worker_var = j * 20;  /* Worker partitioned - case 2 */
                    
                    #pragma acc loop vector
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        /* This creates different partitioning based on condition */
                        int mixed_var;
                        if (partitioning_mode) {
                            mixed_var = worker_var + v;  /* Worker+vector - case 6 */
                        } else {
                            mixed_var = i * VECTOR_LEN + v;  /* Gang+vector - case 5 */
                        }
                        
                        int idx = (i * WORKERS * VECTOR_LEN + 
                                  j * VECTOR_LEN + v) % N;
                        if (idx < N) {
                            conditional_array[idx] = (float)mixed_var * 
                                                   global_matrix[idx][0];
                        }
                    }
                }
            } else {
                /* Vector-only when worker level is disabled */
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN * WORKERS; v++) {
                    int vector_only_var = v;  /* Vector partitioned - case 4 */
                    int idx = (i * VECTOR_LEN * WORKERS + v) % N;
                    if (idx < N) {
                        conditional_array[idx] = (float)vector_only_var;
                    }
                }
            }
        }
    }
}
