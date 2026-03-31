/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define P 256

/* Global variables for different scoping */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Multi-dimensional array */
static double static_partial[M];         /* Static storage */

/* Function to initialize data */
void init_data() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (i * 0.1f) + (j * 0.01f);
        }
    }
    for (int j = 0; j < M; j++) {
        static_partial[j] = sin(j * 0.1);
    }
}

int main() {
    /* Local variables with different types and scopes */
    int gang_partitioned_scalar = 0;     /* Should become gang partitioned (case 1) */
    float worker_partitioned_array[32];  /* Should become worker partitioned (case 2) */
    double gang_worker_partitioned = 0.0;/* Should become gang+worker partitioned (case 3) */
    int vector_partitioned = 0;          /* Should become vector partitioned (case 4) */
    float gang_vector_matrix[16][32];    /* Should become gang+vector partitioned (case 5) */
    double worker_vector_reduction = 0.0;/* Should become worker+vector partitioned (case 6) */
    int fully_partitioned_grid[8][16][32];/* Should become fully partitioned (case 7) */
    
    /* Initialize local arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 32; j++) {
            gang_vector_matrix[i][j] = i * j * 0.01f;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 32; k++) {
                fully_partitioned_grid[i][j][k] = i + j * 10 + k * 100;
            }
        }
    }
    
    init_data();
    
    /* ========== CASE 0: Gang Redundant ========== */
    /* Global variable accessed by all gangs without partitioning */
    #pragma acc parallel num_gangs(4) copy(global_gang_redundant)
    {
        int local_gang_id = 0;
        #pragma acc loop gang private(local_gang_id)
        for (int g = 0; g < 4; g++) {
            local_gang_id = g;
            /* All gangs read the same global variable */
            int temp = global_gang_redundant + local_gang_id;
            /* Force computation to prevent optimization */
            global_gang_redundant = (temp > 50) ? temp : global_gang_redundant;
        }
    }
    
    /* ========== CASE 1: Gang Partitioned ========== */
    /* Scalar partitioned across gangs only */
    #pragma acc parallel num_gangs(8) copy(gang_partitioned_scalar)
    {
        #pragma acc loop gang
        for (int g = 0; g < 8; g++) {
            /* Each gang gets its own copy */
            int gang_private = g * 10;
            gang_partitioned_scalar += gang_private;
        }
    }
    
    /* ========== CASE 2: Worker Partitioned ========== */
    /* Array partitioned across workers only */
    #pragma acc parallel num_workers(4) vector_length(32) copyout(worker_partitioned_array[0:32])
    {
        #pragma acc loop worker
        for (int w = 0; w < 4; w++) {
            /* Each worker processes 8 elements of the array */
            for (int i = w * 8; i < (w + 1) * 8; i++) {
                worker_partitioned_array[i] = w * 100.0f + i;
            }
        }
    }
    
    /* ========== CASE 3: Gang+Worker Partitioned ========== */
    /* Variable partitioned across both gangs and workers */
    #pragma acc parallel num_gangs(2) num_workers(4) copy(gang_worker_partitioned)
    {
        #pragma acc loop gang worker
        for (int g = 0; g < 2; g++) {
            for (int w = 0; w < 4; w++) {
                /* Each gang-worker pair gets its own value */
                gang_worker_partitioned += (g + 1) * (w + 1) * 0.1;
            }
        }
    }
    
    /* ========== CASE 4: Vector Partitioned ========== */
    /* Using kernels directive with vector partitioning */
    #pragma acc kernels copy(vector_partitioned)
    {
        #pragma acc loop independent vector(32)
        for (int i = 0; i < 1024; i++) {
            /* Vector partitioned computation */
            vector_partitioned += i % 32;
        }
    }
    
    /* ========== CASE 5: Gang+Vector Partitioned ========== */
    /* 2D array partitioned across gangs and vectors */
    #pragma acc parallel num_gangs(4) vector_length(16) copy(gang_vector_matrix)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 32; j++) {
                /* Stencil-like access pattern complicates analysis */
                float neighbor_sum = 0.0f;
                if (i > 0) neighbor_sum += gang_vector_matrix[i-1][j];
                if (j > 0) neighbor_sum += gang_vector_matrix[i][j-1];
                gang_vector_matrix[i][j] = neighbor_sum * 0.5f + i * j * 0.01f;
            }
        }
    }
    
    /* ========== CASE 6: Worker+Vector Partitioned ========== */
    /* Reduction variable partitioned across workers and vectors */
    #pragma acc parallel num_workers(2) vector_length(64) copy(worker_vector_reduction)
    {
        #pragma acc loop worker vector reduction(+:worker_vector_reduction)
        for (int i = 0; i < 4096; i++) {
            /* Complex conditional partitioning */
            if (i % 3 == 0) {
                worker_vector_reduction += sin(i * 0.01);
            } else if (i % 3 == 1) {
                worker_vector_reduction += cos(i * 0.01);
            } else {
                worker_vector_reduction += tan(i * 0.01);
            }
        }
    }
    
    /* ========== CASE 7: Fully Partitioned ========== */
    /* 3D array partitioned across gangs, workers, and vectors */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(8) copy(fully_partitioned_grid)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 16; j++) {
                for (int k = 0; k < 32; k++) {
                    /* Complex multi-dimensional stencil */
                    int sum = 0;
                    if (i > 0) sum += fully_partitioned_grid[i-1][j][k];
                    if (j > 0) sum += fully_partitioned_grid[i][j-1][k];
                    if (k > 0) sum += fully_partitioned_grid[i][j][k-1];
                    if (i < 7) sum += fully_partitioned_grid[i+1][j][k];
                    if (j < 15) sum += fully_partitioned_grid[i][j+1][k];
                    if (k < 31) sum += fully_partitioned_grid[i][j][k+1];
                    fully_partitioned_grid[i][j][k] = sum / 6;
                }
            }
        }
    }
    
    /* ========== Mixed Complex Region ========== */
    /* Region combining multiple partitioning strategies */
    int mixed_result = 0;
    float mixed_array[N];
    
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
                copyin(global_matrix) copyout(mixed_array[0:N]) copy(mixed_result)
    {
        /* Different variables with different partitioning */
        int gang_local;          /* Gang partitioned */
        float worker_local[4];   /* Worker partitioned */
        double vector_local;     /* Vector partitioned */
        
        #pragma acc loop gang private(gang_local)
        for (int g = 0; g < 4; g++) {
            gang_local = g * 100;
            
            #pragma acc loop worker private(worker_local)
            for (int w = 0; w < 2; w++) {
                for (int v = 0; v < 4; v++) {
                    worker_local[v] = w * 10.0f + v;
                }
                
                #pragma acc loop vector private(vector_local)
                for (int v = 0; v < 32; v++) {
                    vector_local = v * 0.01;
                    
                    /* Access global matrix with stencil pattern */
                    int idx = g * 256 + w * 32 + v;
                    if (idx < N) {
                        float val = global_matrix[idx][0];
                        if (idx > 0) val += global_matrix[idx-1][0];
                        if (idx < N-1) val += global_matrix[idx+1][0];
                        mixed_array[idx] = val * 0.333f + gang_local + worker_local[v%4] + vector_local;
                        
                        /* Conditional update */
                        if ((g + w + v) % 7 == 0) {
                            mixed_result++;
                        }
                    }
                }
            }
        }
    }
    
    /* Final computation to use all results and prevent dead code elimination */
    float final_sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        final_sum += worker_partitioned_array[i];
    }
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 32; j++) {
            final_sum += gang_vector_matrix[i][j];
        }
    }
    
    printf("Results: global=%d, gang_scalar=%d, gang_worker=%.2f\n",
           global_gang_redundant, gang_partitioned_scalar, gang_worker_partitioned);
    printf("vector=%d, worker_vector=%.4f, mixed_result=%d\n",
           vector_partitioned, worker_vector_reduction, mixed_result);
    printf("Final checksum: %.2f\n", final_sum);
    
    return 0;
}
