/* test_omp_oacc_partitioning.c
 * Comprehensive test to trigger all partitioning states in GCC's OpenACC neutering/broadcast logic
 * Compile with: gcc -O1 -fopenacc -fdump-tree-omplower -fdump-tree-optimized -c test_omp_oacc_partitioning.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables to create different scoping contexts */
int global_redundant = 42;              /* Should become gang redundant (case 0) */
float global_partitioned[M][N];         /* Will be partitioned in various ways */
static int module_scalar = 100;

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            global_partitioned[i][j] = (float)(i * N + j) / (N * M);
        }
    }
}

/* Main function with complex OpenACC regions */
int main() {
    /* Local variables with different lifetimes and scopes */
    int gang_private_scalar = 10;           /* Target: gang partitioned (case 1) */
    float worker_partitioned_array[N];      /* Target: worker partitioned (case 2) */
    double gang_worker_shared[M];           /* Target: gang+worker partitioned (case 3) */
    int vector_only_var = 0;                /* Target: vector partitioned (case 4) */
    float gang_vector_matrix[GANGS][N];     /* Target: gang+vector partitioned (case 5) */
    double worker_vector_array[WORKERS][VECTOR_LEN]; /* Target: worker+vector partitioned (case 6) */
    int fully_partitioned_3d[GANGS][WORKERS][VECTOR_LEN]; /* Target: fully partitioned (case 7) */
    
    /* Initialize local arrays */
    for (int i = 0; i < N; i++) {
        worker_partitioned_array[i] = sinf(i * 0.01f);
    }
    
    for (int i = 0; i < M; i++) {
        gang_worker_shared[i] = cos(i * 0.02);
    }
    
    /* Initialize global matrix */
    init_matrices();
    
    printf("Starting OpenACC partitioning test...\n");
    
    /* ====================================================================
     * REGION 1: Gang redundant variable (case 0)
     * Global variable accessed read-only by all gangs
     * ==================================================================== */
    #pragma acc parallel num_gangs(GANGS) copyout(gang_private_scalar) \
        copy(global_partitioned[0:M][0:N/2])
    {
        /* global_redundant is read-only, should be gang redundant */
        int local_gang_id = 0;
        #pragma acc loop gang private(local_gang_id)
        for (int g = 0; g < GANGS; g++) {
            local_gang_id = g;
            /* Use the gang-redundant variable in computation */
            gang_private_scalar = global_redundant + local_gang_id;
            
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                /* Access global partitioned array - will have complex partitioning */
                int idx = (local_gang_id * WORKERS + w) % M;
                global_partitioned[idx][0] += gang_private_scalar * 0.5f;
            }
        }
    }
    
    /* ====================================================================
     * REGION 2: Kernels region with explicit loop directives
     * Targeting various partitioning states
     * ==================================================================== */
    #pragma acc kernels copyin(worker_partitioned_array[0:N]) \
        copyout(gang_vector_matrix[0:GANGS][0:N]) \
        create(worker_vector_array[0:WORKERS][0:VECTOR_LEN])
    {
        /* Case 1: Gang partitioned scalar */
        int gang_specific = 0;
        
        /* Case 4: Vector partitioned computation */
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            gang_specific = g * 100;  /* Different per gang */
            
            /* Nested worker loop - creates worker partitioning context */
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                /* Case 2: Worker partitioned array access */
                float worker_sum = 0.0f;
                for (int i = 0; i < 16; i++) {
                    int idx = (w * 16 + i) % N;
                    worker_sum += worker_partitioned_array[idx];
                }
                
                /* Case 6: Worker+vector partitioned array */
                #pragma acc loop vector independent
                for (int v = 0; v < VECTOR_LEN; v++) {
                    worker_vector_array[w][v] = worker_sum + (v * 0.1f) + gang_specific;
                }
            }
            
            /* Case 5: Gang+vector partitioned matrix */
            #pragma acc loop vector independent
            for (int j = 0; j < N; j++) {
                gang_vector_matrix[g][j] = gang_specific + sinf(j * 0.01f);
            }
        }
    }
    
    /* ====================================================================
     * REGION 3: Complex nested parallelism with all three levels
     * Targeting combined partitioning states
     * ==================================================================== */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copyin(gang_worker_shared[0:M]) \
        copyout(fully_partitioned_3d[0:GANGS][0:WORKERS][0:VECTOR_LEN])
    {
        /* Case 3: Gang+worker partitioned array */
        double gang_worker_local[M];
        
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            /* Initialize gang+worker partitioned array */
            #pragma acc loop worker independent
            for (int i = 0; i < M; i++) {
                gang_worker_local[i] = gang_worker_shared[i] * (g + 1);
            }
            
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                /* Case 7: Fully partitioned 3D array */
                #pragma acc loop vector independent
                for (int v = 0; v < VECTOR_LEN; v++) {
                    /* Complex computation using all partitioning levels */
                    double base = gang_worker_local[(g + w) % M];
                    fully_partitioned_3d[g][w][v] = base * (v + 1) * 
                                                   sin(g * 0.1) * cos(w * 0.05);
                }
            }
        }
    }
    
    /* ====================================================================
     * REGION 4: Conditional partitioning based on runtime values
     * Forces analysis of different partitioning paths
     * ==================================================================== */
    int conditional_scalar = rand() % 100;
    float conditional_array[VECTOR_LEN * 2];
    
    #pragma acc parallel vector_length(VECTOR_LEN) \
        copy(conditional_array[0:VECTOR_LEN*2])
    {
        #pragma acc loop gang independent
        for (int g = 0; g < 2; g++) {
            int gang_decision = conditional_scalar + g;
            
            #pragma acc loop worker independent
            for (int w = 0; w < 4; w++) {
                /* Conditional vector partitioning */
                if (gang_decision > 50) {
                    /* Case 4: Vector partitioned when condition true */
                    #pragma acc loop vector independent
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        conditional_array[g * VECTOR_LEN + v] = 
                            (v * 1.5f) / (w + 1);
                    }
                } else {
                    /* Different partitioning when condition false */
                    #pragma acc loop vector independent
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        conditional_array[g * VECTOR_LEN + v] = 
                            (v * 0.5f) * (w + 1);
                    }
                }
            }
        }
    }
    
    /* ====================================================================
     * REGION 5: Stencil computation on 2D array
     * Complex data dependencies to influence partitioning
     * ==================================================================== */
    float stencil_input[M][N];
    float stencil_output[M][N];
    
    /* Initialize stencil arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            stencil_input[i][j] = (float)(i + j) / (M + N);
        }
    }
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copyin(stencil_input[0:M][0:N]) \
        copyout(stencil_output[0:M][0:N])
    {
        /* Multi-dimensional stencil with neighbor access */
        #pragma acc loop gang independent collapse(2)
        for (int i = 1; i < M - 1; i++) {
            for (int j = 1; j < N - 1; j++) {
                float sum = 0.0f;
                
                /* Worker-level reduction */
                #pragma acc loop worker reduction(+:sum)
                for (int wi = 0; wi < 2; wi++) {
                    /* Vector-level computation */
                    #pragma acc loop vector reduction(+:sum)
                    for (int vj = 0; vj < 2; vj++) {
                        int ni = i + wi - 1;
                        int nj = j + vj - 1;
                        sum += stencil_input[ni][nj];
                    }
                }
                
                stencil_output[i][j] = sum / 4.0f;
            }
        }
    }
    
    /* Compute checksum to verify execution and prevent dead code elimination */
    double checksum = 0.0;
    
    /* Sum elements from fully_partitioned_3d */
    #pragma acc parallel loop gang reduction(+:checksum) \
        copyin(fully_partitioned_3d[0:GANGS][0:WORKERS][0:VECTOR_LEN])
    for (int g = 0; g < GANGS; g++) {
        double gang_sum = 0.0;
        #pragma acc loop worker reduction(+:gang_sum)
        for (int w = 0; w < WORKERS; w++) {
            double worker_sum = 0.0;
            #pragma acc loop vector reduction(+:worker_sum)
            for (int v = 0; v < VECTOR_LEN; v++) {
                worker_sum += fully_partitioned_3d[g][w][v];
            }
            gang_sum += worker_sum;
        }
        checksum += gang_sum;
    }
    
    /* Add contributions from other arrays */
    for (int g = 0; g < GANGS; g++) {
        for (int j = 0; j < N; j++) {
            checksum += gang_vector_matrix[g][j];
        }
    }
    
    for (int w = 0; w < WORKERS; w++) {
        for (int v = 0; v < VECTOR_LEN; v++) {
            checksum += worker_vector_array[w][v];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
