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
float global_matrix[N][M];               /* Base for complex partitioning */
static int file_static_var = 100;

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) / (N * M);
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    /* Local variables with different scopes */
    int host_scalar = 10;
    float host_array[N];
    double host_matrix[N][M];
    
    /* Initialize data */
    init_matrices();
    
    /* Copy global matrix to local */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            host_matrix[i][j] = global_matrix[i][j];
        }
    }
    
    /* SCENARIO 1: Gang-only partitioning (targeting case 1) */
    printf("Starting gang-only partitioning...\n");
    #pragma acc parallel num_gangs(GANGS) copy(host_scalar) copyin(global_matrix) \
        copyout(host_array[0:N])
    {
        /* This scalar should be gang partitioned */
        int gang_private = acc_gang_id();  /* Case 1: gang partitioned */
        
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            float sum = 0.0f;
            #pragma acc loop vector reduction(+:sum)
            for (int j = 0; j < M; j++) {
                sum += global_matrix[i][j];
            }
            host_array[i] = sum;
        }
        
        /* Access gang redundant variable */
        if (gang_private == 0) {
            host_scalar += global_gang_redundant;
        }
    }
    
    /* SCENARIO 2: Vector-only partitioning (targeting case 4) */
    printf("Starting vector-only partitioning...\n");
    #pragma acc kernels copy(host_matrix) copyin(host_array)
    {
        /* Variables that should become vector partitioned */
        float vector_private[VECTOR_LEN];  /* Case 4: vector partitioned */
        
        #pragma acc loop independent vector(VECTOR_LEN)
        for (int i = 0; i < N; i++) {
            int vector_idx = i % VECTOR_LEN;
            vector_private[vector_idx] = host_array[i];
            
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                host_matrix[i][j] += vector_private[vector_idx];
            }
        }
    }
    
    /* SCENARIO 3: Complex nested parallelism targeting combined states */
    printf("Starting complex nested partitioning...\n");
    {
        /* Variables with mixed attributes */
        int mixed_scalar = 50;                    /* Will have complex partitioning */
        float worker_private_matrix[WORKERS][16]; /* Worker partitioned */
        double gang_vector_private[GANGS][32];    /* Gang+vector partitioned */
        
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
            copy(mixed_scalar) copyin(host_matrix) private(worker_private_matrix)
        {
            int gang_id = acc_gang_id();
            int worker_id = acc_worker_id();
            int vector_id = acc_vector_id();
            
            /* Case 3: gang+worker partitioned */
            int gw_partitioned = gang_id * WORKERS + worker_id;
            
            /* Case 5: gang+vector partitioned */
            int gv_partitioned = gang_id * VECTOR_LEN + vector_id;
            
            /* Case 6: worker+vector partitioned */
            int wv_partitioned = worker_id * VECTOR_LEN + vector_id;
            
            /* Case 7: fully partitioned */
            int fully_partitioned = gang_id * WORKERS * VECTOR_LEN + 
                                   worker_id * VECTOR_LEN + vector_id;
            
            /* Initialize worker-private matrix */
            for (int w = 0; w < WORKERS; w++) {
                for (int k = 0; k < 16; k++) {
                    worker_private_matrix[w][k] = (float)(w * 16 + k);
                }
            }
            
            /* Complex computation with conditional partitioning */
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                float temp = 0.0f;
                
                /* Conditional partitioning based on runtime values */
                if (i % 2 == 0) {
                    /* Use gang+worker partitioning */
                    temp += gw_partitioned;
                } else {
                    /* Use worker+vector partitioning */
                    temp += wv_partitioned;
                }
                
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    /* Stencil-like access pattern complicates analysis */
                    float neighbor_sum = 0.0f;
                    if (j > 0) neighbor_sum += host_matrix[i][j-1];
                    if (j < M-1) neighbor_sum += host_matrix[i][j+1];
                    
                    /* Mix all partitioning types in computation */
                    host_matrix[i][j] = host_matrix[i][j] * 0.5f + 
                                       temp * 0.25f + 
                                       neighbor_sum * 0.25f +
                                       fully_partitioned * 0.01f;
                }
            }
            
            /* Reduction across partitioning levels */
            #pragma acc atomic update
            mixed_scalar += fully_partitioned;
        }
        
        printf("Mixed scalar final value: %d\n", mixed_scalar);
    }
    
    /* SCENARIO 4: Explicit gang redundant variable (targeting case 0) */
    printf("Starting gang-redundant scenario...\n");
    {
        /* Variable that should be marked gang redundant */
        const int gang_redundant_const = 99;  /* Case 0: gang redundant */
        int reduction_result = 0;
        
        #pragma acc parallel num_gangs(4) copy(reduction_result) \
            copyin(gang_redundant_const)
        {
            int local_gang_id = acc_gang_id();
            
            /* All gangs access the same gang-redundant value */
            int local_compute = gang_redundant_const * (local_gang_id + 1);
            
            #pragma acc loop gang
            for (int i = 0; i < 100; i++) {
                /* Use the gang-redundant value in computation */
                local_compute += i;
            }
            
            #pragma acc atomic
            reduction_result += local_compute;
        }
        
        printf("Reduction result with gang-redundant: %d\n", reduction_result);
    }
    
    /* SCENARIO 5: Multi-dimensional array with explicit partitioning */
    printf("Starting multi-dimensional explicit partitioning...\n");
    {
        double multi_dim[GANGS][WORKERS][VECTOR_LEN][8];
        int explicit_partitioning = 0;
        
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
            vector_length(VECTOR_LEN) copyout(multi_dim) copy(explicit_partitioning)
        {
            int g = acc_gang_id();
            int w = acc_worker_id();
            int v = acc_vector_id();
            
            /* Explicitly partitioned in all dimensions */
            #pragma acc loop gang worker vector
            for (int i = 0; i < 8; i++) {
                multi_dim[g][w][v][i] = g * 1000.0 + w * 100.0 + v * 10.0 + i;
                
                /* Conditional to force different partitioning paths */
                if (g % 2 == 0) {
                    /* Force worker+vector partitioning analysis */
                    explicit_partitioning += w * v;
                } else {
                    /* Force gang+vector partitioning analysis */
                    explicit_partitioning += g * v;
                }
            }
        }
        
        /* Verify some results */
        double check_sum = 0.0;
        #pragma acc parallel loop gang reduction(+:check_sum) \
            copyin(multi_dim) copy(check_sum)
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker reduction(+:check_sum)
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector reduction(+:check_sum)
                for (int v = 0; v < VECTOR_LEN; v++) {
                    for (int i = 0; i < 8; i++) {
                        check_sum += multi_dim[g][w][v][i];
                    }
                }
            }
        }
        
        printf("Multi-dim check sum: %f\n", check_sum);
        printf("Explicit partitioning result: %d\n", explicit_partitioning);
    }
    
    /* Final verification computation */
    double final_sum = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            final_sum += host_matrix[i][j];
        }
    }
    
    printf("Final matrix sum: %f\n", final_sum);
    printf("All partitioning scenarios completed.\n");
    
    return 0;
}
