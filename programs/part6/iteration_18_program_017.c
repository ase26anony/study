/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables to create complex scoping */
int global_gang_redundant = 0;  /* Should become case 0 */
float global_matrix[N][M];
static double static_partial;

/* Function to trigger various partitioning scenarios */
void run_partitioning_tests() {
    int i, j, k;
    float local_matrix[N][M];
    double temp_results[GANGS][WORKERS];
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i + j);
            local_matrix[i][j] = (float)(i * j);
        }
    }
    
    /* ============================================
       CASE 0: Gang Redundant
       A variable that's the same across all gangs
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copy(global_gang_redundant)
    {
        /* global_gang_redundant should be marked as gang redundant */
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            /* All gangs see the same value */
            if (global_gang_redundant == 0) {
                /* Do something */
                int dummy = g * 2;
            }
        }
    }
    
    /* ============================================
       CASE 1: Gang Partitioned
       Variables partitioned only across gangs
       ============================================ */
    int gang_private[GANGS];  /* Will be partitioned across gangs only */
    
    #pragma acc parallel num_gangs(GANGS) private(gang_private)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            gang_private[g] = g * 100;  /* Each gang gets its own slice */
            
            /* Nested worker loop - but gang_private remains gang-partitioned */
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                /* gang_private[g] accessible here but same for all workers in this gang */
                int val = gang_private[g] + w;
            }
        }
    }
    
    /* ============================================
       CASE 2: Worker Partitioned  
       Variables partitioned only across workers
       ============================================ */
    #pragma acc parallel num_gangs(1) num_workers(WORKERS)
    {
        int worker_local;  /* Should be worker partitioned */
        
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            worker_local = w * 10;  /* Each worker gets its own copy */
            
            /* Vector loop inside worker */
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN; v++) {
                /* worker_local same for all vectors in this worker */
                int result = worker_local + v;
            }
        }
    }
    
    /* ============================================
       CASE 3: Gang+Worker Partitioned
       Variables partitioned across both gangs and workers
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
                copyout(temp_results)
    {
        double gang_worker_var;  /* Should become gang+worker partitioned */
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                /* Each gang-worker pair gets its own copy */
                gang_worker_var = sin((double)(g * w));
                
                /* Store result - each worker in each gang has unique value */
                temp_results[g][w] = gang_worker_var;
                
                /* Vector loop - gang_worker_var same for all vectors */
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN; v++) {
                    double temp = gang_worker_var * v;
                }
            }
        }
    }
    
    /* ============================================
       CASE 4: Vector Partitioned
       Variables partitioned only across vectors
       ============================================ */
    #pragma acc kernels copyin(local_matrix) copyout(global_matrix)
    {
        float vector_private[N];  /* Will be vector partitioned */
        
        #pragma acc loop independent
        for (i = 0; i < N; i++) {
            /* This loop will be vectorized */
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                /* vector_private[i] is vector-partitioned */
                vector_private[i] = local_matrix[i][j] * 2.0f;
                global_matrix[i][j] = vector_private[i] + 1.0f;
            }
        }
    }
    
    /* ============================================
       CASE 5: Gang+Vector Partitioned
       Variables partitioned across gangs and vectors
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN)
    {
        int gang_vector_var;  /* Should be gang+vector partitioned */
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            /* No worker level - direct to vector */
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN * 4; v++) {
                /* Each gang-vector lane gets its own copy */
                gang_vector_var = g * 1000 + v;
                
                /* Perform computation */
                float calc = sinf((float)gang_vector_var);
            }
        }
    }
    
    /* ============================================
       CASE 6: Worker+Vector Partitioned
       Variables partitioned across workers and vectors
       ============================================ */
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        float worker_vector_array[WORKERS][VECTOR_LEN];  /* Worker+vector partitioned */
        
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN; v++) {
                /* Each worker-vector pair gets unique element */
                worker_vector_array[w][v] = (float)(w * VECTOR_LEN + v);
                
                /* Nested computation */
                for (int k = 0; k < 4; k++) {
                    worker_vector_array[w][v] *= 1.01f;
                }
            }
        }
    }
    
    /* ============================================
       CASE 7: Fully Partitioned
       Variables partitioned across gangs, workers, and vectors
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        /* Fully partitioned variable - unique to each gang/worker/vector */
        int fully_partitioned;
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN; v++) {
                    /* Each parallel lane gets its own private copy */
                    fully_partitioned = g * WORKERS * VECTOR_LEN + 
                                       w * VECTOR_LEN + v;
                    
                    /* Complex stencil-like access pattern */
                    if (v > 0 && v < VECTOR_LEN - 1) {
                        /* Access "neighbors" - complicates analysis */
                        int left = fully_partitioned - 1;
                        int right = fully_partitioned + 1;
                        fully_partitioned = (left + right) / 2;
                    }
                    
                    /* Multi-dimensional array access */
                    int idx_i = fully_partitioned % N;
                    int idx_j = (fully_partitioned / N) % M;
                    if (idx_i < N && idx_j < M) {
                        global_matrix[idx_i][idx_j] += 0.5f;
                    }
                }
            }
        }
    }
    
    /* ============================================
       Mixed partitioning with conditional logic
       ============================================ */
    int conditional_var = rand() % 10;
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
                copy(conditional_var) copyin(local_matrix)
    {
        float mixed_partition[VECTOR_LEN];  /* Partitioning depends on condition */
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                /* Conditional partitioning */
                if (conditional_var > 5) {
                    /* Vector-partitioned in this path */
                    #pragma acc loop vector
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        mixed_partition[v] = local_matrix[g % N][(w * VECTOR_LEN + v) % M];
                    }
                } else {
                    /* Worker-partitioned in this path */
                    float worker_shared = local_matrix[g % N][w % M];
                    #pragma acc loop vector
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        mixed_partition[v] = worker_shared + v;
                    }
                }
                
                /* Use the mixed_partition array */
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN; v++) {
                    if (g < N && (w * VECTOR_LEN + v) < M) {
                        global_matrix[g][w * VECTOR_LEN + v] += mixed_partition[v];
                    }
                }
            }
        }
    }
    
    /* Final reduction to ensure computations aren't optimized away */
    double final_sum = 0.0;
    #pragma acc parallel loop reduction(+:final_sum) copyin(global_matrix)
    for (i = 0; i < N; i++) {
        #pragma acc loop reduction(+:final_sum)
        for (j = 0; j < M; j++) {
            final_sum += global_matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", final_sum);
}

int main() {
    printf("Starting OpenACC partitioning tests...\n");
    
    run_partitioning_tests();
    
    /* Additional OpenMP offloading test for completeness */
    #ifdef _OPENMP
    printf("Running OpenMP offloading variant...\n");
    
    int omp_array[N][M];
    #pragma omp target teams distribute parallel for \
                map(tofrom: omp_array) num_teams(GANGS) thread_limit(WORKERS*VECTOR_LEN)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            /* Creates various partitioning scenarios in OpenMP */
            int team_id = omp_get_team_num();
            int thread_id = omp_get_thread_num();
            omp_array[i][j] = team_id * 1000 + thread_id;
        }
    }
    #endif
    
    printf("Tests completed.\n");
    return 0;
}
