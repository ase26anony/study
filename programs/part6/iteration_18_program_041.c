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
float global_matrix[N][M];               /* Multi-dimensional array */
static int module_level_var = 100;

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
    /* Local variables with different types and scopes */
    int host_scalar = 10;
    float host_array[N];
    double host_matrix[N][M];
    int *dynamic_array = (int*)malloc(N * sizeof(int));
    
    /* Initialize data */
    init_matrices();
    for (int i = 0; i < N; i++) {
        host_array[i] = sinf(i * 0.01f);
        dynamic_array[i] = i * 2;
        for (int j = 0; j < M; j++) {
            host_matrix[i][j] = cosf(i * j * 0.0001f);
        }
    }
    
    /* ============================================
       SCENARIO 1: Gang-only partitioning (case 1)
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copy(host_scalar)
    {
        /* Each gang gets its own private copy */
        int gang_private = host_scalar + acc_gang_id();  /* Should be gang partitioned */
        float gang_array[10];
        
        #pragma acc loop gang
        for (int i = 0; i < 10; i++) {
            gang_array[i] = gang_private * i;
        }
        
        /* Use gang_array to prevent optimization */
        float gang_sum = 0.0f;
        for (int i = 0; i < 10; i++) {
            gang_sum += gang_array[i];
        }
        host_scalar += (int)gang_sum;
    }
    
    /* ============================================
       SCENARIO 2: Vector-only partitioning (case 4)
       ============================================ */
    #pragma acc kernels copy(host_array[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            /* Each vector lane gets private computation */
            float vector_private = host_array[i] * 2.0f;  /* Should be vector partitioned */
            host_array[i] = vector_private + sinf(vector_private);
        }
    }
    
    /* ============================================
       SCENARIO 3: Complex nested partitioning
       Targeting cases 2, 3, 5, 6, 7
       ============================================ */
    int conditional_var = 1;
    float result_matrix[N][M];
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copyin(global_matrix, host_matrix) copyout(result_matrix)
    {
        /* Variables at different nested levels */
        int gang_level = acc_gang_id();                    /* Gang partitioned (case 1) */
        int worker_level;                                  /* Worker partitioned (case 2) */
        float vector_level;                                /* Vector partitioned (case 4) */
        
        /* Conditional partitioning based on runtime value */
        if (conditional_var > 0) {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                worker_level = acc_worker_id();            /* Worker partitioned (case 2) */
                
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    vector_level = acc_vector_id();        /* Vector partitioned (case 4) */
                    
                    /* Combined partitioning scenarios */
                    float gang_worker_partitioned = global_matrix[i][j] * gang_level;  /* Case 3 */
                    float gang_vector_partitioned = host_matrix[i][j] * vector_level;  /* Case 5 */
                    float worker_vector_partitioned = (float)worker_level * vector_level; /* Case 6 */
                    
                    /* Fully partitioned computation (case 7) */
                    float fully_partitioned = gang_worker_partitioned + 
                                            gang_vector_partitioned + 
                                            worker_vector_partitioned +
                                            (float)module_level_var;  /* Module-level variable */
                    
                    /* Stencil-like access pattern complicates analysis */
                    float neighbor_sum = 0.0f;
                    if (i > 0) neighbor_sum += global_matrix[i-1][j];
                    if (j > 0) neighbor_sum += global_matrix[i][j-1];
                    if (i < N-1) neighbor_sum += global_matrix[i+1][j];
                    if (j < M-1) neighbor_sum += global_matrix[i][j+1];
                    
                    result_matrix[i][j] = fully_partitioned + neighbor_sum * 0.25f;
                }
            }
        } else {
            /* Alternative path with different partitioning */
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    /* Different combination for the else path */
                    result_matrix[i][j] = global_matrix[i][j] * 2.0f;
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 4: Gang redundant (case 0)
       ============================================ */
    int reduction_sum = 0;
    
    #pragma acc parallel num_gangs(GANGS) copy(reduction_sum)
    {
        /* global_gang_redundant should be marked as gang redundant */
        int local_copy = global_gang_redundant;  /* Read-only, same for all gangs */
        
        #pragma acc loop gang reduction(+:reduction_sum)
        for (int i = 0; i < 100; i++) {
            reduction_sum += local_copy + i;
        }
    }
    
    /* ============================================
       SCENARIO 5: Mixed directives for coverage
       ============================================ */
    float mixed_results[GANGS * WORKERS * VECTOR_LEN];
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copyout(mixed_results[0:GANGS*WORKERS*VECTOR_LEN])
    {
        int gid = acc_gang_id();
        int wid = acc_worker_id();
        int vid = acc_vector_id();
        
        /* Explicit partitioning with loop directives */
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            float gang_private_array[5];  /* Gang partitioned */
            
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                int worker_private = w * 10;  /* Worker partitioned */
                
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN; v++) {
                    /* All combinations in one computation */
                    int idx = g * WORKERS * VECTOR_LEN + w * VECTOR_LEN + v;
                    mixed_results[idx] = 
                        (float)gang_private_array[g % 5] * 0.1f +  /* Gang partitioned */
                        (float)worker_private * 0.01f +            /* Worker partitioned */
                        (float)v * 0.001f +                        /* Vector partitioned */
                        (float)global_gang_redundant * 0.0001f;    /* Gang redundant */
                }
            }
        }
    }
    
    /* ============================================
       Final computation and output
       ============================================ */
    /* Compute checksum to verify execution */
    double final_checksum = 0.0;
    #pragma acc parallel loop reduction(+:final_checksum) copyin(result_matrix)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            final_checksum += result_matrix[i][j];
        }
    }
    
    /* Add contributions from other arrays */
    for (int i = 0; i < N; i++) {
        final_checksum += host_array[i];
    }
    
    for (int i = 0; i < GANGS * WORKERS * VECTOR_LEN; i++) {
        final_checksum += mixed_results[i];
    }
    
    final_checksum += reduction_sum;
    final_checksum += host_scalar;
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Partitioning test completed.\n");
    
    /* Cleanup */
    free(dynamic_array);
    
    return 0;
}
