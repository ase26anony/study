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
float global_matrix[N][M];               /* Complex multi-dimensional array */
static int module_scalar = 100;

/* Function with complex partitioning scenarios */
void compute_partitioning_patterns() {
    int i, j, k;
    
    /* Initialize global matrix */
    #pragma acc parallel loop gang vector collapse(2) copyout(global_matrix[0:N][0:M])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j);
        }
    }
    
    /* CASE 1: Gang partitioned variables */
    int gang_private_var = 0;
    float gang_partitioned_array[GANGS][VECTOR_LEN];
    
    #pragma acc parallel num_gangs(GANGS) copy(gang_private_var) \
        copy(gang_partitioned_array[0:GANGS][0:VECTOR_LEN])
    {
        int gang_id = __pgi_gangidx();
        #pragma acc loop gang private(gang_private_var)
        for (i = 0; i < GANGS; i++) {
            gang_private_var = gang_id * 100 + i;
            #pragma acc loop vector
            for (k = 0; k < VECTOR_LEN; k++) {
                gang_partitioned_array[i][k] = gang_private_var + k;
            }
        }
    }
    
    /* CASE 0: Gang redundant variable usage */
    int use_global_redundant = global_gang_redundant;
    #pragma acc parallel num_gangs(GANGS) copy(use_global_redundant)
    {
        #pragma acc loop gang
        for (i = 0; i < GANGS; i++) {
            use_global_redundant += i;  /* All gangs access same global */
        }
    }
    
    /* CASE 4: Vector partitioned computation */
    float vector_results[N];
    #pragma acc kernels copyout(vector_results[0:N])
    {
        #pragma acc loop independent vector
        for (i = 0; i < N; i++) {
            float temp = 0.0f;
            #pragma acc loop seq
            for (k = 0; k < 10; k++) {
                temp += sinf((float)i * 0.1f + k);
            }
            vector_results[i] = temp;
        }
    }
    
    /* CASE 2: Worker partitioned with conditional */
    int worker_partitioned = 0;
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(worker_partitioned)
    {
        #pragma acc loop worker
        for (i = 0; i < WORKERS; i++) {
            int worker_id = __pgi_workeridx();
            if (worker_id % 2 == 0) {
                worker_partitioned += worker_id * 10;
            } else {
                worker_partitioned += worker_id * 5;
            }
        }
    }
    
    /* CASES 3, 5, 6, 7: Complex nested partitioning */
    float complex_partitioning[GANGS][WORKERS][VECTOR_LEN];
    int gang_worker_shared = 0;
    int gang_vector_private = 0;
    int worker_vector_scalar = 0;
    int fully_partitioned_var = 0;
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        vector_length(VECTOR_LEN) \
        copy(complex_partitioning[0:GANGS][0:WORKERS][0:VECTOR_LEN]) \
        copy(gang_worker_shared) copy(gang_vector_private) \
        copy(worker_vector_scalar) copy(fully_partitioned_var)
    {
        int gid = __pgi_gangidx();
        int wid = __pgi_workeridx();
        int vid = __pgi_vectoridx();
        
        /* CASE 3: Gang+Worker partitioned */
        #pragma acc loop gang worker private(gang_worker_shared)
        for (i = 0; i < GANGS; i++) {
            for (j = 0; j < WORKERS; j++) {
                gang_worker_shared = gid * 1000 + wid * 100 + i + j;
                
                /* CASE 5: Gang+Vector partitioned */
                #pragma acc loop gang vector private(gang_vector_private)
                for (k = 0; k < VECTOR_LEN; k++) {
                    gang_vector_private = gid * 100 + vid * 10 + k;
                    
                    /* CASE 6: Worker+Vector partitioned */
                    #pragma acc loop worker vector private(worker_vector_scalar)
                    for (int l = 0; l < 8; l++) {
                        worker_vector_scalar = wid * 100 + vid * 10 + l;
                        
                        /* CASE 7: Fully partitioned */
                        #pragma acc loop gang worker vector private(fully_partitioned_var)
                        for (int m = 0; m < 4; m++) {
                            fully_partitioned_var = gid * 1000 + wid * 100 + vid * 10 + m;
                            complex_partitioning[i][j][k] += 
                                gang_worker_shared + gang_vector_private + 
                                worker_vector_scalar + fully_partitioned_var;
                        }
                    }
                }
            }
        }
    }
    
    /* Stencil computation with multi-dimensional access */
    float stencil_input[N][M];
    float stencil_output[N][M];
    
    /* Initialize stencil input */
    #pragma acc parallel loop gang vector collapse(2) \
        copyin(stencil_input[0:N][0:M]) copyout(stencil_output[0:N][0:M])
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            stencil_input[i][j] = (float)(i + j);
        }
    }
    
    /* Complex stencil with runtime condition affecting partitioning */
    #pragma acc kernels copyin(stencil_input) copyout(stencil_output)
    {
        #pragma acc loop gang
        for (i = 1; i < N-1; i++) {
            #pragma acc loop worker
            for (j = 1; j < M-1; j++) {
                float sum = 0.0f;
                int condition = (i + j) % 3;
                
                /* Different partitioning based on runtime condition */
                if (condition == 0) {
                    #pragma acc loop vector reduction(+:sum)
                    for (int di = -1; di <= 1; di++) {
                        for (int dj = -1; dj <= 1; dj++) {
                            sum += stencil_input[i+di][j+dj];
                        }
                    }
                } else if (condition == 1) {
                    #pragma acc loop seq
                    for (int di = -1; di <= 1; di++) {
                        #pragma acc loop vector
                        for (int dj = -1; dj <= 1; dj++) {
                            sum += stencil_input[i+di][j+dj];
                        }
                    }
                } else {
                    #pragma acc loop vector
                    for (int di = -1; di <= 1; di++) {
                        #pragma acc loop seq
                        for (int dj = -1; dj <= 1; dj++) {
                            sum += stencil_input[i+di][j+dj];
                        }
                    }
                }
                
                stencil_output[i][j] = sum / 9.0f;
            }
        }
    }
    
    /* Final reduction with mixed partitioning */
    float final_result = 0.0f;
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        vector_length(VECTOR_LEN) copy(final_result)
    {
        float gang_local[GANGS];
        float worker_local[WORKERS];
        float vector_local[VECTOR_LEN];
        
        #pragma acc loop gang private(gang_local)
        for (i = 0; i < GANGS; i++) {
            gang_local[i] = 0.0f;
            
            #pragma acc loop worker private(worker_local)
            for (j = 0; j < WORKERS; j++) {
                worker_local[j] = 0.0f;
                
                #pragma acc loop vector private(vector_local) reduction(+:worker_local[j])
                for (k = 0; k < VECTOR_LEN; k++) {
                    vector_local[k] = (float)(i * 10000 + j * 100 + k);
                    worker_local[j] += vector_local[k];
                }
                
                gang_local[i] += worker_local[j];
            }
            
            #pragma acc atomic update
            final_result += gang_local[i];
        }
    }
    
    printf("Final result checksum: %f\n", final_result);
}

int main() {
    printf("Testing OpenACC partitioning states...\n");
    
    compute_partitioning_patterns();
    
    /* Additional OpenMP offloading test for completeness */
    #ifdef _OPENMP
    int omp_array[N];
    #pragma omp target teams distribute parallel for map(tofrom: omp_array[0:N])
    for (int i = 0; i < N; i++) {
        omp_array[i] = i * i;
    }
    
    int omp_sum = 0;
    #pragma omp target teams distribute parallel for reduction(+:omp_sum) \
        map(tofrom: omp_sum)
    for (int i = 0; i < N; i++) {
        omp_sum += omp_array[i];
    }
    printf("OpenMP reduction result: %d\n", omp_sum);
    #endif
    
    printf("Test completed.\n");
    return 0;
}
