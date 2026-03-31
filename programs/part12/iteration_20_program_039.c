/* Test for OpenACC partition string coverage in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition test_partition.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define MAX_VECTOR 128

/* Structure to store queried execution configuration */
typedef struct {
    int gangs;
    int workers;
    int vector_len;
    long checksum;
} exec_config_t;

/* Host array to store results from device queries */
exec_config_t host_results[NUM_CONFIGS];

/* Device array for runtime queries */
#pragma acc declare create(host_results)

/* Initialize device array */
void init_results() {
    for (int i = 0; i < NUM_CONFIGS; i++) {
        host_results[i].gangs = 0;
        host_results[i].workers = 0;
        host_results[i].vector_len = 0;
        host_results[i].checksum = 0;
    }
    #pragma acc update device(host_results)
}

/* Test case 0: Gang redundant (all 1s) */
void test_gang_redundant(int idx) {
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyin(idx) present(host_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        
        #pragma acc atomic update
        host_results[idx].gangs += g;
        #pragma acc atomic update
        host_results[idx].workers += w;
        #pragma acc atomic update
        host_results[idx].vector_len += v;
        
        /* Simple computation to ensure region isn't optimized away */
        long local_sum = 0;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < 100; i++) {
            local_sum += i * (g + w + v);
        }
        
        #pragma acc atomic update
        host_results[idx].checksum += local_sum;
    }
}

/* Test case 1: Gang partitioned */
void test_gang_partitioned(int idx) {
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(idx) present(host_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        
        #pragma acc atomic update
        host_results[idx].gangs += g;
        #pragma acc atomic update
        host_results[idx].workers += w;
        #pragma acc atomic update
        host_results[idx].vector_len += v;
        
        /* Nested parallelism: gang-level only */
        long local_sum = 0;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < 64; i++) {
            local_sum += i * g;
        }
        
        #pragma acc atomic update
        host_results[idx].checksum += local_sum;
    }
}

/* Test case 2: Worker partitioned */
void test_worker_partitioned(int idx) {
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(idx) present(host_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        
        #pragma acc atomic update
        host_results[idx].gangs += g;
        #pragma acc atomic update
        host_results[idx].workers += w;
        #pragma acc atomic update
        host_results[idx].vector_len += v;
        
        /* Worker-level parallelism */
        long local_sum = 0;
        #pragma acc loop worker reduction(+:local_sum)
        for (int i = 0; i < 32; i++) {
            local_sum += i * w;
        }
        
        #pragma acc atomic update
        host_results[idx].checksum += local_sum;
    }
}

/* Test case 3: Gang+Worker partitioned */
void test_gang_worker_partitioned(int idx) {
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyin(idx) present(host_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        
        #pragma acc atomic update
        host_results[idx].gangs += g;
        #pragma acc atomic update
        host_results[idx].workers += w;
        #pragma acc atomic update
        host_results[idx].vector_len += v;
        
        /* Nested gang+worker parallelism */
        long local_sum = 0;
        #pragma acc loop gang worker reduction(+:local_sum)
        for (int i = 0; i < 16; i++) {
            local_sum += i * (g + w);
        }
        
        #pragma acc atomic update
        host_results[idx].checksum += local_sum;
    }
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned(int idx) {
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(128) \
        copyin(idx) present(host_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        
        #pragma acc atomic update
        host_results[idx].gangs += g;
        #pragma acc atomic update
        host_results[idx].workers += w;
        #pragma acc atomic update
        host_results[idx].vector_len += v;
        
        /* Vector-level parallelism */
        long local_sum = 0;
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = 0; i < v; i++) {
            local_sum += i;
        }
        
        #pragma acc atomic update
        host_results[idx].checksum += local_sum;
    }
}

/* Test case 5: Gang+Vector partitioned */
void test_gang_vector_partitioned(int idx) {
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        copyin(idx) present(host_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        
        #pragma acc atomic update
        host_results[idx].gangs += g;
        #pragma acc atomic update
        host_results[idx].workers += w;
        #pragma acc atomic update
        host_results[idx].vector_len += v;
        
        /* Nested gang+vector parallelism with collapse */
        long local_sum = 0;
        #pragma acc loop gang vector collapse(2) reduction(+:local_sum)
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < v; j++) {
                local_sum += i * j * g;
            }
        }
        
        #pragma acc atomic update
        host_results[idx].checksum += local_sum;
    }
}

/* Test case 6: Worker+Vector partitioned */
void test_worker_vector_partitioned(int idx) {
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
        copyin(idx) present(host_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        
        #pragma acc atomic update
        host_results[idx].gangs += g;
        #pragma acc atomic update
        host_results[idx].workers += w;
        #pragma acc atomic update
        host_results[idx].vector_len += v;
        
        /* Worker+vector nested parallelism */
        long local_sum = 0;
        #pragma acc loop worker vector reduction(+:local_sum)
        for (int i = 0; i < w * v; i++) {
            local_sum += i;
        }
        
        #pragma acc atomic update
        host_results[idx].checksum += local_sum;
    }
}

/* Test case 7: Fully partitioned */
void test_fully_partitioned(int idx) {
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(8) \
        copyin(idx) present(host_results)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        
        #pragma acc atomic update
        host_results[idx].gangs += g;
        #pragma acc atomic update
        host_results[idx].workers += w;
        #pragma acc atomic update
        host_results[idx].vector_len += v;
        
        /* Fully nested parallelism */
        long local_sum = 0;
        #pragma acc loop gang worker vector reduction(+:local_sum)
        for (int i = 0; i < g * w * v; i++) {
            local_sum += i;
        }
        
        #pragma acc atomic update
        host_results[idx].checksum += local_sum;
    }
}

/* Test with dynamic values (conditional partitioning) */
void test_dynamic_partitioning() {
    int dynamic_gangs = 3;
    int dynamic_workers = 5;
    int dynamic_vector = 64;
    
    /* Use kernels directive with dynamic values */
    #pragma acc kernels num_gangs(dynamic_gangs) \
        num_workers(dynamic_workers) vector_length(dynamic_vector)
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers();
        int v = acc_get_vector_length();
        
        /* Force runtime to compute partition mapping */
        volatile int dummy = g + w + v;
        
        /* Simple computation to ensure region executes */
        #pragma acc loop gang worker vector
        for (int i = 0; i < 100; i++) {
            dummy += i;
        }
    }
}

/* Test with data regions and unstructured constructs */
void test_with_data_regions() {
    int N = 1000;
    float *data = (float *)acc_malloc(N * sizeof(float));
    
    if (data == NULL) {
        /* This may trigger diagnostic path with partition string */
        fprintf(stderr, "acc_malloc failed - may show partition info\n");
        return;
    }
    
    /* Unstructured data enter */
    #pragma acc enter data create(data[0:N])
    
    /* Structured compute with mixed partitioning */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
        present(data[0:N])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            data[i] = i * 0.1f;
        }
    }
    
    /* Another region with different partitioning */
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
        present(data[0:N])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            data[i] += 1.0f;
        }
    }
    
    /* Unstructured exit */
    #pragma acc exit data copyout(data[0:N])
    
    /* Verify data (prevents optimization) */
    float sum = 0;
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    printf("Data region checksum: %f\n", sum);
    
    acc_free(data);
}

int main() {
    printf("Testing OpenACC partition string coverage...\n");
    
    /* Initialize results on device */
    init_results();
    
    /* Execute all test configurations */
    test_gang_redundant(0);
    test_gang_partitioned(1);
    test_worker_partitioned(2);
    test_gang_worker_partitioned(3);
    test_vector_partitioned(4);
    test_gang_vector_partitioned(5);
    test_worker_vector_partitioned(6);
    test_fully_partitioned(7);
    
    /* Additional tests for coverage */
    test_dynamic_partitioning();
    test_with_data_regions();
    
    /* Copy results back from device */
    #pragma acc update host(host_results)
    
    /* Print results and compute final checksum */
    long total_checksum = 0;
    printf("\nPartition Configuration Results:\n");
    printf("Idx | Gangs | Workers | Vector | Checksum\n");
    printf("----|-------|---------|--------|----------\n");
    
    for (int i = 0; i < NUM_CONFIGS; i++) {
        printf("%2d  | %5d | %7d | %6d | %10ld\n",
               i,
               host_results[i].gangs,
               host_results[i].workers,
               host_results[i].vector_len,
               host_results[i].checksum);
        total_checksum += host_results[i].checksum;
    }
    
    printf("\nTotal checksum: %ld\n", total_checksum);
    
    /* Force runtime diagnostics by checking device type */
    acc_device_t dev_type = acc_get_device_type();
    printf("Active device type: %d\n", (int)dev_type);
    
    /* Additional OpenMP offload test (if supported) */
    #ifdef _OPENMP
    printf("\nTesting OpenMP offload partition mapping...\n");
    int omp_results[3] = {0, 0, 0};
    
    #pragma omp target teams num_teams(4) thread_limit(32) map(tofrom: omp_results)
    {
        omp_results[0] = omp_get_num_teams();
        omp_results[1] = omp_get_num_threads();
        
        #pragma omp parallel for reduction(+:omp_results[2])
        for (int i = 0; i < 100; i++) {
            omp_results[2] += i;
        }
    }
    
    printf("OpenMP teams: %d, threads: %d, checksum: %d\n",
           omp_results[0], omp_results[1], omp_results[2]);
    #endif
    
    return 0;
}
