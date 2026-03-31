/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables for different scoping */
int global_gang_redundant = 0;  /* Should become "gang redundant" (case 0) */
float global_matrix[N][M];
static int file_static_counter = 0;

/* Function to trigger complex partitioning analysis */
void process_matrix(int mode) {
    int local_scalar = mode;  /* Will have different partitioning based on usage */
    float local_array[N/4][M/4];
    int i, j, k;
    
    /* Initialize local array */
    #pragma acc parallel loop gang worker vector collapse(2) copyout(local_array[0:N/4][0:M/4])
    for (i = 0; i < N/4; i++) {
        for (j = 0; j < M/4; j++) {
            local_array[i][j] = i * 100.0f + j;
        }
    }
    
    /* CASE 1: Gang partitioned variable */
    int gang_partitioned = 0;
    #pragma acc parallel num_gangs(GANGS) copy(gang_partitioned)
    {
        #pragma acc loop gang
        for (i = 0; i < GANGS; i++) {
            /* Each gang gets its own copy */
            int gang_private = i * 1000;
            gang_partitioned += gang_private;
        }
    }
    
    /* CASE 2: Worker partitioned variable */
    int worker_partitioned = 0;
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(1) \
                copy(worker_partitioned)
    {
        #pragma acc loop worker
        for (i = 0; i < WORKERS; i++) {
            worker_partitioned += i * 100;
        }
    }
    
    /* CASE 3: Gang+Worker partitioned variable */
    int gang_worker_partitioned = 0;
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(1) \
                copy(gang_worker_partitioned)
    {
        #pragma acc loop gang worker
        for (i = 0; i < GANGS * WORKERS; i++) {
            gang_worker_partitioned += i;
        }
    }
    
    /* CASE 4: Vector partitioned variable */
    float vector_partitioned[VECTOR_LEN];
    #pragma acc parallel vector_length(VECTOR_LEN) \
                copyout(vector_partitioned[0:VECTOR_LEN])
    {
        #pragma acc loop vector
        for (i = 0; i < VECTOR_LEN; i++) {
            vector_partitioned[i] = i * 3.14f;
        }
    }
    
    /* CASE 5: Gang+Vector partitioned variable */
    float gang_vector_partitioned[GANGS][VECTOR_LEN];
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
                copyout(gang_vector_partitioned[0:GANGS][0:VECTOR_LEN])
    {
        #pragma acc loop gang vector
        for (i = 0; i < GANGS * VECTOR_LEN; i++) {
            int gang_idx = i / VECTOR_LEN;
            int vector_idx = i % VECTOR_LEN;
            gang_vector_partitioned[gang_idx][vector_idx] = 
                gang_idx * 1000.0f + vector_idx;
        }
    }
    
    /* CASE 6: Worker+Vector partitioned variable */
    float worker_vector_partitioned[WORKERS][VECTOR_LEN];
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copyout(worker_vector_partitioned[0:WORKERS][0:VECTOR_LEN])
    {
        #pragma acc loop worker vector
        for (i = 0; i < WORKERS * VECTOR_LEN; i++) {
            int worker_idx = i / VECTOR_LEN;
            int vector_idx = i % VECTOR_LEN;
            worker_vector_partitioned[worker_idx][vector_idx] = 
                worker_idx * 500.0f + vector_idx;
        }
    }
    
    /* CASE 7: Fully partitioned variable (Gang+Worker+Vector) */
    int fully_partitioned[GANGS][WORKERS][VECTOR_LEN];
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copyout(fully_partitioned[0:GANGS][0:WORKERS][0:VECTOR_LEN])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (i = 0; i < GANGS; i++) {
            for (j = 0; j < WORKERS; j++) {
                for (k = 0; k < VECTOR_LEN; k++) {
                    fully_partitioned[i][j][k] = i * 10000 + j * 100 + k;
                }
            }
        }
    }
    
    /* Complex nested region with mixed partitioning */
    int mixed_result = 0;
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copy(mixed_result)
    {
        /* Global variable accessed - should be "gang redundant" (case 0) */
        int local_copy = global_gang_redundant;
        
        #pragma acc loop gang reduction(+:mixed_result)
        for (i = 0; i < GANGS; i++) {
            int gang_local = i * 10;
            
            #pragma acc loop worker
            for (j = 0; j < WORKERS; j++) {
                int worker_local = j * 5;
                
                #pragma acc loop vector
                for (k = 0; k < VECTOR_LEN; k++) {
                    mixed_result += gang_local + worker_local + k + local_copy;
                }
            }
        }
    }
    
    /* Stencil computation to complicate dependency analysis */
    float stencil_result[N][M];
    #pragma acc data copyin(global_matrix[0:N][0:M]) \
                     copyout(stencil_result[0:N][0:M])
    {
        #pragma acc kernels
        {
            #pragma acc loop gang worker
            for (i = 1; i < N-1; i++) {
                #pragma acc loop vector
                for (j = 1; j < M-1; j++) {
                    /* 5-point stencil */
                    stencil_result[i][j] = 
                        (global_matrix[i-1][j] + global_matrix[i+1][j] +
                         global_matrix[i][j-1] + global_matrix[i][j+1] +
                         global_matrix[i][j]) / 5.0f;
                }
            }
        }
    }
    
    /* Conditional partitioning based on runtime value */
    int conditional_var = 0;
    #pragma acc parallel num_gangs(GANGS) copy(conditional_var)
    {
        if (mode > 0) {
            /* Different partitioning when condition is true */
            #pragma acc loop gang
            for (i = 0; i < GANGS; i++) {
                conditional_var += i * 100;
            }
        } else {
            #pragma acc loop gang worker
            for (i = 0; i < GANGS * WORKERS; i++) {
                conditional_var += i * 50;
            }
        }
    }
    
    /* Print some results to prevent optimization */
    printf("Results: gp=%d, wp=%d, gwp=%d, mixed=%d, cond=%d\n",
           gang_partitioned, worker_partitioned, 
           gang_worker_partitioned, mixed_result, conditional_var);
}

int main() {
    int i, j;
    
    /* Initialize global matrix */
    #pragma acc parallel loop gang worker vector collapse(2) \
                copyout(global_matrix[0:N][0:M])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            global_matrix[i][j] = i * M + j;
        }
    }
    
    /* Set global gang redundant variable */
    global_gang_redundant = 42;
    
    /* Process with different modes to trigger different paths */
    process_matrix(0);
    process_matrix(1);
    
    /* Additional complex region targeting all partitioning cases */
    int final_result = 0;
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copy(final_result)
    {
        /* Variables at different scopes within the parallel region */
        int gang_private[GANGS];
        int worker_private[WORKERS];
        int vector_private[VECTOR_LEN];
        
        #pragma acc loop gang private(gang_private)
        for (i = 0; i < GANGS; i++) {
            gang_private[i] = i * 1000;
            
            #pragma acc loop worker private(worker_private)
            for (j = 0; j < WORKERS; j++) {
                worker_private[j] = j * 100;
                
                #pragma acc loop vector private(vector_private) \
                            reduction(+:final_result)
                for (int k = 0; k < VECTOR_LEN; k++) {
                    vector_private[k] = k;
                    final_result += gang_private[i] + worker_private[j] + vector_private[k];
                }
            }
        }
    }
    
    printf("Final result: %d\n", final_result);
    
    /* Multi-dimensional array computation with explicit partitioning */
    float md_array[8][16][32];
    #pragma acc data copyout(md_array[0:8][0:16][0:32])
    {
        #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16)
        {
            #pragma acc loop gang
            for (i = 0; i < 8; i++) {
                #pragma acc loop worker
                for (j = 0; j < 16; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 32; k++) {
                        md_array[i][j][k] = i * 10000.0f + j * 100.0f + k;
                    }
                }
            }
        }
    }
    
    /* Verify computation */
    float checksum = 0.0f;
    #pragma acc parallel loop reduction(+:checksum) \
                copyin(md_array[0:8][0:16][0:32])
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 16; j++) {
            for (int k = 0; k < 32; k++) {
                checksum += md_array[i][j][k];
            }
        }
    }
    
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
