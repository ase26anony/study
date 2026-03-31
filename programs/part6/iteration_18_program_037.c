/* test-omp-acc-partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define P 256

/* Global variables at different scopes */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Multi-dimensional array */
static double static_worker_partitioned; /* File scope static */

/* Function with complex partitioning scenarios */
void process_data(int mode) {
    int gang_private_scalar;             /* Should become gang partitioned (case 1) */
    float worker_private_array[P];       /* Worker partitioned (case 2) */
    double gang_worker_shared[32][32];   /* Gang+worker partitioned (case 3) */
    int vector_only_var = 0;             /* Vector partitioned (case 4) */
    
    /* Initialize some data */
    for (int i = 0; i < P; i++) {
        worker_private_array[i] = i * 1.5f;
    }
    
    /* Case 1: Gang partitioned variable in gang-only region */
    #pragma acc parallel num_gangs(8) copyin(global_gang_redundant) \
        private(gang_private_scalar)
    {
        gang_private_scalar = acc_gang_id * 100;
        
        /* Access gang-redundant variable */
        if (global_gang_redundant > 0) {
            gang_private_scalar += global_gang_rendant;
        }
    }
    
    /* Case 4: Vector partitioned in vector-only loop */
    #pragma acc kernels copyout(global_matrix[0:N][0:M])
    {
        #pragma acc loop independent gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop independent vector
            for (int j = 0; j < M; j++) {
                /* vector_only_var is vector partitioned here */
                vector_only_var = i + j;
                global_matrix[i][j] = sinf(vector_only_var * 0.01f);
            }
        }
    }
    
    /* Complex nested parallelism targeting combined states */
    #pragma acc parallel num_gangs(4) num_workers(8) vector_length(32) \
        copyin(worker_private_array) copy(gang_worker_shared) \
        private(gang_private_scalar)
    {
        int gang_id = acc_gang_id;
        int worker_id = acc_worker_id;
        int vector_id = acc_vector_id;
        
        /* Case 3: Gang+worker partitioned */
        gang_worker_shared[gang_id][worker_id] = gang_id * 1000.0 + worker_id * 100.0;
        
        /* Case 5: Gang+vector partitioned variable */
        int gang_vector_var = gang_id * 32 + vector_id;
        
        /* Case 6: Worker+vector partitioned variable */
        float worker_vector_array[32];
        worker_vector_array[vector_id] = worker_id * 50.0f + vector_id;
        
        /* Case 7: Fully partitioned (gang+worker+vector) */
        int fully_partitioned = gang_id * 10000 + worker_id * 100 + vector_id;
        
        /* Stencil computation to complicate dependency analysis */
        #pragma acc loop vector
        for (int k = 0; k < 31; k++) {
            /* Access neighboring elements */
            worker_vector_array[k] = worker_vector_array[k] + 
                                     worker_vector_array[k+1] * 0.5f;
            fully_partitioned += (int)(worker_vector_array[k]);
        }
        
        /* Conditional partitioning based on runtime values */
        if (mode > 0) {
            /* Different partitioning when mode > 0 */
            #pragma acc loop worker
            for (int w = 0; w < 8; w++) {
                float conditional_var = w * 2.0f;
                gang_worker_shared[gang_id][w] += conditional_var;
            }
        }
        
        /* Reduction with mixed partitioning */
        double local_sum = 0.0;
        #pragma acc loop vector reduction(+:local_sum)
        for (int v = 0; v < 32; v++) {
            local_sum += worker_vector_array[v];
        }
        
        gang_worker_shared[gang_id][worker_id] += local_sum;
    }
    
    /* Additional test with explicit loop directives */
    int gang_partitioned_array[16];
    int worker_partitioned_array[64];
    int vector_partitioned_array[128];
    
    #pragma acc parallel num_gangs(16) num_workers(4) vector_length(64)
    {
        /* Explicit gang loop */
        #pragma acc loop gang
        for (int g = 0; g < 16; g++) {
            gang_partitioned_array[g] = g * 10;
            
            /* Explicit worker loop inside gang */
            #pragma acc loop worker
            for (int w = 0; w < 4; w++) {
                worker_partitioned_array[g*4 + w] = g * 100 + w * 10;
                
                /* Explicit vector loop inside worker */
                #pragma acc loop vector
                for (int v = 0; v < 64; v++) {
                    vector_partitioned_array[(g*4 + w)*64 + v] = 
                        g * 1000 + w * 100 + v;
                }
            }
        }
    }
}

/* Function with multi-dimensional stencil computation */
void stencil_computation() {
    float input[N][M], output[N][M];
    
    /* Initialize input */
    #pragma acc parallel loop gang vector collapse(2) copyout(input)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            input[i][j] = i * j * 0.001f;
        }
    }
    
    /* Complex stencil with different partitioning */
    #pragma acc kernels copyin(input) copyout(output)
    {
        #pragma acc loop gang independent
        for (int i = 1; i < N-1; i++) {
            #pragma acc loop worker independent
            for (int j = 1; j < M-1; j++) {
                float temp[3][3];  /* Should trigger complex partitioning */
                
                #pragma acc loop vector independent collapse(2)
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {
                        temp[di+1][dj+1] = input[i+di][j+dj];
                    }
                }
                
                /* Compute weighted average */
                output[i][j] = 0.0f;
                #pragma acc loop vector reduction(+:output[i][j])
                for (int di = 0; di < 3; di++) {
                    for (int dj = 0; dj < 3; dj++) {
                        output[i][j] += temp[di][dj] * 0.111f;
                    }
                }
            }
        }
    }
}

int main() {
    int mode = 1;
    double checksum = 0.0;
    
    printf("Testing OpenACC partitioning states...\n");
    
    /* First test: Simple partitioning scenarios */
    process_data(mode);
    
    /* Second test: Multi-dimensional stencil */
    stencil_computation();
    
    /* Third test: Mixed OpenMP/OpenACC style (if supported) */
    #ifdef _OPENMP
    #pragma omp target teams distribute parallel for \
        map(tofrom: checksum) private(mode)
    for (int i = 0; i < N; i++) {
        float local_var = i * 0.5f;  /* Additional partitioning case */
        #pragma omp simd
        for (int j = 0; j < M; j++) {
            checksum += sinf(local_var + j * 0.1f);
        }
    }
    #endif
    
    /* Compute final checksum from global matrix */
    #pragma acc parallel loop reduction(+:checksum) copy(checksum)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += global_matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Partitioning test completed.\n");
    
    return 0;
}
