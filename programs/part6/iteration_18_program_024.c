/* test_omp_acc_partitioning.c
 * Comprehensive test to exercise all OpenACC variable partitioning states
 * Compile with: gcc -O1 -fopenacc -fdump-tree-omplower -o test test_omp_acc_partitioning.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables to create complex scoping scenarios */
int global_gang_redundant = 0;          /* Should become case 0 */
float global_matrix[N][M];
static double static_partitioned = 1.0;

/* Function to initialize matrices */
void init_matrices(float A[N][M], float B[N][M]) {
    #pragma acc parallel loop gang vector collapse(2) copyout(A[0:N][0:M], B[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            A[i][j] = (float)(i + j) * 0.1f;
            B[i][j] = (float)(i * j) * 0.01f;
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    /* Local variables with different storage durations */
    int local_scalar = 42;                     /* Mixed partitioning */
    float local_array[N];                      /* Array for vector partitioning */
    double matrix_a[N][M], matrix_b[N][M];     /* 2D arrays for complex access */
    int reduction_result = 0;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        local_array[i] = sinf(i * 0.01f);
        for (int j = 0; j < M; j++) {
            matrix_a[i][j] = (i + j) * 0.5;
            matrix_b[i][j] = (i - j) * 0.5;
        }
    }
    
    printf("Starting OpenACC partitioning test...\n");
    
    /* ============================================
     * SCENARIO 1: Gang redundant (case 0)
     * Global variable accessed by all gangs
     * ============================================ */
    #pragma acc parallel num_gangs(GANGS) copy(global_gang_redundant)
    {
        int gang_id = __pgi_gangidx();
        #pragma acc atomic update
        global_gang_redundant += gang_id;
    }
    printf("Gang redundant result: %d\n", global_gang_redundant);
    
    /* ============================================
     * SCENARIO 2: Gang partitioned (case 1)
     * Private scalar at gang level
     * ============================================ */
    #pragma acc parallel num_gangs(GANGS) copy(local_scalar)
    {
        int gang_private = local_scalar + __pgi_gangidx();  /* Gang partitioned */
        #pragma acc loop worker vector independent
        for (int i = 0; i < 10; i++) {
            /* Access gang-private variable */
            gang_private += i;
        }
        if (__pgi_gangidx() == 0) {
            local_scalar = gang_private;
        }
    }
    
    /* ============================================
     * SCENARIO 3: Worker partitioned (case 2)
     * Using worker-only parallelism
     * ============================================ */
    int worker_partitioned = 0;
    #pragma acc parallel num_workers(WORKERS) vector_length(1) copy(worker_partitioned)
    {
        int worker_local = __pgi_workeridx();  /* Worker partitioned */
        #pragma acc atomic update
        worker_partitioned += worker_local;
    }
    
    /* ============================================
     * SCENARIO 4: Gang+Worker partitioned (case 3)
     * Nested gang/worker parallelism
     * ============================================ */
    int gw_partitioned = 0;
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(1) copy(gw_partitioned)
    {
        int gang_worker_var = __pgi_gangidx() * 100 + __pgi_workeridx();
        
        /* Conditional partitioning based on runtime values */
        if (gang_worker_var % 2 == 0) {
            int conditional_var = gang_worker_var * 2;  /* Gang+Worker partitioned */
            #pragma acc atomic update
            gw_partitioned += conditional_var;
        }
    }
    
    /* ============================================
     * SCENARIO 5: Vector partitioned (case 4)
     * Vector-only operations on arrays
     * ============================================ */
    #pragma acc parallel vector_length(VECTOR_LEN) copy(local_array[0:N])
    {
        int vec_idx = __pgi_vectoridx();
        if (vec_idx < N) {
            float vector_private = local_array[vec_idx];  /* Vector partitioned */
            vector_private *= 2.0f;
            local_array[vec_idx] = vector_private;
        }
    }
    
    /* ============================================
     * SCENARIO 6: Gang+Vector partitioned (case 5)
     * Mixed gang and vector parallelism
     * ============================================ */
    float gv_result = 0.0f;
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) copy(gv_result)
    {
        float gang_vector_var = __pgi_gangidx() * 10.0f + __pgi_vectoridx() * 0.1f;
        
        /* Complex computation with mixed partitioning */
        #pragma acc loop seq
        for (int iter = 0; iter < 5; iter++) {
            gang_vector_var = sinf(gang_vector_var) + cosf(gang_vector_var);
        }
        
        #pragma acc atomic update
        gv_result += gang_vector_var;
    }
    
    /* ============================================
     * SCENARIO 7: Worker+Vector partitioned (case 6)
     * Worker and vector nested parallelism
     * ============================================ */
    double wv_matrix[16][VECTOR_LEN];
    #pragma acc parallel num_workers(4) vector_length(VECTOR_LEN) copy(wv_matrix)
    {
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        if (worker_id < 16 && vector_id < VECTOR_LEN) {
            double worker_vector_var = (worker_id + 1) * (vector_id + 1) * 0.01;
            
            /* Stencil-like access pattern */
            if (vector_id > 0 && vector_id < VECTOR_LEN - 1) {
                worker_vector_var += wv_matrix[worker_id][vector_id - 1] * 0.25;
                worker_vector_var += wv_matrix[worker_id][vector_id + 1] * 0.25;
            }
            
            wv_matrix[worker_id][vector_id] = worker_vector_var;
        }
    }
    
    /* ============================================
     * SCENARIO 8: Fully partitioned (case 7)
     * All three levels of parallelism
     * ============================================ */
    int fully_partitioned_result = 0;
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                copy(fully_partitioned_result)
    {
        /* Variables with different partitioning */
        int gang_var = __pgi_gangidx();                    /* Gang partitioned */
        int worker_var = __pgi_workeridx();                /* Worker partitioned */
        int vector_var = __pgi_vectoridx();                /* Vector partitioned */
        
        /* Fully partitioned computation */
        int fully_partitioned = gang_var * 1000 + worker_var * 100 + vector_var;
        
        /* Multi-dimensional array access with complex pattern */
        #pragma acc loop seq
        for (int k = 0; k < 3; k++) {
            fully_partitioned = (fully_partitioned * 13 + 7) % 1000;
        }
        
        #pragma acc atomic update
        fully_partitioned_result += fully_partitioned;
    }
    
    /* ============================================
     * COMPLEX SCENARIO: Mixed partitioning in kernels
     * Using kernels directive with explicit clauses
     * ============================================ */
    double complex_result = 0.0;
    #pragma acc kernels copyin(matrix_a, matrix_b) copyout(complex_result)
    {
        double gang_shared[GANGS];      /* Gang-level sharing */
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            double worker_accum[WORKERS]; /* Worker-level partitioning */
            
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                double vector_sum = 0.0;  /* Vector-level partitioning */
                
                int start_i = (N / GANGS) * g;
                int end_i = (N / GANGS) * (g + 1);
                int start_j = (M / WORKERS) * w;
                int end_j = (M / WORKERS) * (w + 1);
                
                #pragma acc loop vector reduction(+:vector_sum)
                for (int i = start_i; i < end_i && i < N; i++) {
                    for (int j = start_j; j < end_j && j < M; j++) {
                        /* Stencil computation accessing neighbors */
                        double center = matrix_a[i][j] + matrix_b[i][j];
                        if (i > 0) center += matrix_a[i-1][j] * 0.1;
                        if (i < N-1) center += matrix_a[i+1][j] * 0.1;
                        if (j > 0) center += matrix_a[i][j-1] * 0.1;
                        if (j < M-1) center += matrix_a[i][j+1] * 0.1;
                        
                        vector_sum += center;
                    }
                }
                
                worker_accum[w] = vector_sum;
            }
            
            /* Reduce worker results at gang level */
            gang_shared[g] = 0.0;
            #pragma acc loop seq
            for (int w = 0; w < WORKERS; w++) {
                gang_shared[g] += worker_accum[w];
            }
        }
        
        /* Final reduction across gangs */
        complex_result = 0.0;
        #pragma acc loop seq
        for (int g = 0; g < GANGS; g++) {
            complex_result += gang_shared[g];
        }
    }
    
    /* ============================================
     * FINAL VALIDATION AND OUTPUT
     * ============================================ */
    printf("\nPartitioning test results:\n");
    printf("  Gang redundant:        %d\n", global_gang_redundant);
    printf("  Local scalar:          %d\n", local_scalar);
    printf("  Worker partitioned:    %d\n", worker_partitioned);
    printf("  Gang+Worker:           %d\n", gw_partitioned);
    printf("  Gang+Vector result:    %.2f\n", gv_result);
    printf("  Fully partitioned:     %d\n", fully_partitioned_result);
    printf("  Complex stencil result: %.2f\n", complex_result);
    
    /* Verify array modifications */
    float array_sum = 0.0f;
    #pragma acc parallel loop reduction(+:array_sum) copyin(local_array[0:N])
    for (int i = 0; i < N; i++) {
        array_sum += local_array[i];
    }
    printf("  Modified array sum:    %.2f\n", array_sum);
    
    printf("\nTest completed successfully!\n");
    
    return 0;
}
