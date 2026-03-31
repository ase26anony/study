/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define P 256

/* Global variables at different scopes */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Base for complex partitioning */
static int file_static_counter = 0;

/* Function to initialize data */
void init_data() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) / (N * M);
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    /* Local variables with different scopes */
    int gang_partitioned_scalar = 10;    /* Target: case 1 */
    float worker_partitioned_array[P];   /* Target: case 2 */
    double gang_worker_partitioned[M];   /* Target: case 3 */
    int vector_partitioned = 0;          /* Target: case 4 */
    float gang_vector_matrix[32][32];    /* Target: case 5 */
    double worker_vector_array[64];      /* Target: case 6 */
    int fully_partitioned_grid[8][8][8]; /* Target: case 7 */
    
    /* Initialize local arrays */
    for (int i = 0; i < P; i++) {
        worker_partitioned_array[i] = sinf(i * 0.1f);
    }
    
    for (int i = 0; i < M; i++) {
        gang_worker_partitioned[i] = cos(i * 0.01);
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            gang_vector_matrix[i][j] = (i + j) * 0.5f;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        worker_vector_array[i] = exp(-i * 0.05);
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                fully_partitioned_grid[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Initialize global data */
    init_data();
    
    /* ============================================
       SCENARIO 1: Gang redundant (case 0)
       ============================================ */
    #pragma acc parallel copyin(global_gang_redundant) \
                         num_gangs(4) num_workers(1) vector_length(1)
    {
        /* global_gang_redundant should be marked as gang redundant
           because it's read-only and same for all gangs */
        int local_copy = global_gang_redundant;
        #pragma acc loop gang
        for (int i = 0; i < 4; i++) {
            /* Use the redundant value */
            file_static_counter += local_copy;
        }
    }
    
    /* ============================================
       SCENARIO 2: Gang partitioned (case 1)
       ============================================ */
    #pragma acc parallel copy(gang_partitioned_scalar) \
                         num_gangs(8) num_workers(1) vector_length(1)
    {
        /* Each gang gets its own copy */
        gang_partitioned_scalar += acc_gang_id();
        
        #pragma acc loop gang
        for (int i = 0; i < 8; i++) {
            if (acc_gang_id() == i) {
                gang_partitioned_scalar *= 2;
            }
        }
    }
    
    /* ============================================
       SCENARIO 3: Worker partitioned (case 2)
       ============================================ */
    #pragma acc parallel copy(worker_partitioned_array[0:P]) \
                         num_gangs(1) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker
        for (int w = 0; w < 4; w++) {
            /* Each worker processes a chunk of the array */
            int start = w * (P / 4);
            int end = (w + 1) * (P / 4);
            for (int i = start; i < end; i++) {
                worker_partitioned_array[i] = worker_partitioned_array[i] * 2.0f;
            }
        }
    }
    
    /* ============================================
       SCENARIO 4: Gang+Worker partitioned (case 3)
       ============================================ */
    #pragma acc parallel copy(gang_worker_partitioned[0:M]) \
                         num_gangs(2) num_workers(4) vector_length(1)
    {
        /* Complex nested partitioning */
        #pragma acc loop gang
        for (int g = 0; g < 2; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 4; w++) {
                int idx = g * 4 + w;
                if (idx < M) {
                    gang_worker_partitioned[idx] += 
                        acc_gang_id() * 0.1 + acc_worker_id() * 0.01;
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 5: Vector partitioned (case 4)
       ============================================ */
    #pragma acc kernels copyout(vector_partitioned)
    {
        int temp_sum = 0;
        #pragma acc loop vector reduction(+:temp_sum)
        for (int v = 0; v < 256; v++) {
            temp_sum += v % 16;
        }
        vector_partitioned = temp_sum;
    }
    
    /* ============================================
       SCENARIO 6: Gang+Vector partitioned (case 5)
       ============================================ */
    #pragma acc parallel copy(gang_vector_matrix) \
                         num_gangs(4) num_workers(1) vector_length(32)
    {
        #pragma acc loop gang
        for (int g = 0; g < 4; g++) {
            #pragma acc loop vector
            for (int v = 0; v < 32; v++) {
                int i = g * 8 + (v % 8);
                int j = v / 8;
                if (i < 32 && j < 32) {
                    gang_vector_matrix[i][j] += 
                        acc_gang_id() * 10.0f + v * 0.1f;
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 7: Worker+Vector partitioned (case 6)
       ============================================ */
    #pragma acc parallel copy(worker_vector_array[0:64]) \
                         num_gangs(1) num_workers(2) vector_length(32)
    {
        #pragma acc loop worker
        for (int w = 0; w < 2; w++) {
            float worker_private = w * 0.5f;
            #pragma acc loop vector
            for (int v = 0; v < 32; v++) {
                int idx = w * 32 + v;
                if (idx < 64) {
                    worker_vector_array[idx] *= 
                        (1.0f + worker_private + v * 0.01f);
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 8: Fully partitioned (case 7)
       ============================================ */
    #pragma acc parallel copy(fully_partitioned_grid) \
                         num_gangs(2) num_workers(4) vector_length(16)
    {
        /* Triple nested loops with all levels of parallelism */
        #pragma acc loop gang
        for (int g = 0; g < 2; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 4; w++) {
                #pragma acc loop vector
                for (int v = 0; v < 16; v++) {
                    int i = g;
                    int j = w;
                    int k = v % 8;
                    if (i < 8 && j < 8 && k < 8) {
                        fully_partitioned_grid[i][j][k] += 
                            acc_gang_id() * 1000 + 
                            acc_worker_id() * 100 + 
                            acc_vector_id();
                    }
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 9: Mixed partitioning with conditions
       ============================================ */
    int conditional_var = rand() % 10;
    float mixed_partitioned[N];
    
    #pragma acc parallel copyin(conditional_var) copy(mixed_partitioned[0:N]) \
                         num_gangs(4) num_workers(2) vector_length(64)
    {
        /* Runtime condition affects partitioning */
        if (conditional_var > 5) {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                mixed_partitioned[i] = global_matrix[i % N][i % M] * 2.0f;
            }
        } else {
            #pragma acc loop gang
            for (int g = 0; g < 4; g++) {
                #pragma acc loop worker vector
                for (int i = g * (N/4); i < (g+1) * (N/4); i++) {
                    mixed_partitioned[i] = global_matrix[i % N][i % M] * 3.0f;
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 10: Stencil computation with complex dependencies
       ============================================ */
    float stencil_input[N][M];
    float stencil_output[N][M];
    
    /* Initialize stencil data */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            stencil_input[i][j] = (float)(i + j) / (N + M);
        }
    }
    
    #pragma acc parallel copyin(stencil_input) copyout(stencil_output) \
                         num_gangs(16) num_workers(2) vector_length(32)
    {
        /* 5-point stencil with mixed partitioning */
        #pragma acc loop gang collapse(2)
        for (int gi = 0; gi < 4; gi++) {
            for (int gj = 0; gj < 4; gj++) {
                #pragma acc loop worker collapse(2)
                for (int wi = 0; wi < 2; wi++) {
                    for (int wj = 0; wj < 2; wj++) {
                        #pragma acc loop vector collapse(2)
                        for (int vi = 0; vi < 4; vi++) {
                            for (int vj = 0; vj < 4; vj++) {
                                int i = gi * 64 + wi * 32 + vi * 8;
                                int j = gj * 64 + wj * 32 + vj * 8;
                                
                                if (i > 0 && i < N-1 && j > 0 && j < M-1) {
                                    /* 5-point stencil computation */
                                    stencil_output[i][j] = 
                                        (stencil_input[i-1][j] +
                                         stencil_input[i+1][j] +
                                         stencil_input[i][j-1] +
                                         stencil_input[i][j+1] +
                                         stencil_input[i][j]) * 0.2f;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    /* Compute checksum to verify execution */
    double checksum = 0.0;
    checksum += gang_partitioned_scalar;
    checksum += worker_partitioned_array[P/2];
    checksum += gang_worker_partitioned[M/2];
    checksum += vector_partitioned;
    checksum += gang_vector_matrix[16][16];
    checksum += worker_vector_array[32];
    checksum += fully_partitioned_grid[4][4][4];
    checksum += mixed_partitioned[N/2];
    checksum += stencil_output[N/2][M/2];
    
    printf("Final checksum: %f\n", checksum);
    printf("File static counter: %d\n", file_static_counter);
    
    return 0;
}
