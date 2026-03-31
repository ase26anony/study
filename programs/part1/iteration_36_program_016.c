#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

// Prevent optimization
volatile int force_runtime = 1;

// Function to use partition-dependent results
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

// Test case 0: Gang redundant (single gang)
void test_gang_redundant(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Single gang, redundant across gangs
    #pragma acc parallel num_gangs(1) copy(data[0:n])
    {
        int idx = acc_gang_id * n + acc_worker_id * 32 + acc_vector_id;
        if (idx < n) {
            data[idx] += 1;
        }
    }
    
    // Force compiler to consider partition mapping
    if (force_runtime) {
        int sum = 0;
        for (int i = 0; i < n; i++) sum += data[i];
        use_result(sum);
    }
    
    free(data);
}

// Test case 1: Gang partitioned
void test_gang_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Multiple gangs, partitioned by gang
    #pragma acc parallel num_gangs(8) copy(data[0:n])
    {
        int gang_size = n / 8;
        int start = acc_gang_id * gang_size;
        int end = (acc_gang_id + 1) * gang_size;
        if (end > n) end = n;
        
        for (int i = start; i < end; i++) {
            data[i] *= 2;
        }
    }
    
    free(data);
}

// Test case 2: Worker partitioned
void test_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Single gang, multiple workers
    #pragma acc parallel num_gangs(1) num_workers(4) copy(data[0:n])
    {
        int worker_chunk = n / 4;
        int start = acc_worker_id * worker_chunk;
        int end = (acc_worker_id + 1) * worker_chunk;
        if (acc_worker_id == 3) end = n;
        
        for (int i = start; i < end; i++) {
            data[i] += acc_worker_id;
        }
    }
    
    free(data);
}

// Test case 3: Gang+worker partitioned
void test_gang_worker_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Both gang and worker partitioning
    #pragma acc parallel num_gangs(4) num_workers(2) copy(data[0:n])
    {
        int gang_chunk = n / 4;
        int gang_start = acc_gang_id * gang_chunk;
        int gang_end = (acc_gang_id + 1) * gang_chunk;
        
        int worker_chunk = gang_chunk / 2;
        int start = gang_start + acc_worker_id * worker_chunk;
        int end = start + worker_chunk;
        
        if (acc_gang_id == 3 && acc_worker_id == 1) {
            end = gang_end;
        }
        
        for (int i = start; i < end; i++) {
            data[i] = acc_gang_id * 10 + acc_worker_id;
        }
    }
    
    free(data);
}

// Test case 4: Vector partitioned
void test_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Vector partitioning only
    #pragma acc parallel vector_length(32) copy(data[0:n])
    {
        int idx = acc_vector_id;
        while (idx < n) {
            data[idx] <<= 1;
            idx += 32;
        }
    }
    
    free(data);
}

// Test case 5: Gang+vector partitioned
void test_gang_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Gang and vector partitioning
    #pragma acc parallel num_gangs(4) vector_length(16) copy(data[0:n])
    {
        int gang_chunk = n / 4;
        int gang_start = acc_gang_id * gang_chunk;
        int gang_end = gang_start + gang_chunk;
        
        int idx = gang_start + acc_vector_id;
        while (idx < gang_end) {
            data[idx] += acc_gang_id;
            idx += 16;
        }
    }
    
    free(data);
}

// Test case 6: Worker+vector partitioned
void test_worker_vector_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // Worker and vector partitioning
    #pragma acc parallel num_workers(2) vector_length(8) copy(data[0:n])
    {
        int worker_chunk = n / 2;
        int worker_start = acc_worker_id * worker_chunk;
        int worker_end = worker_start + worker_chunk;
        
        int idx = worker_start + acc_vector_id;
        while (idx < worker_end) {
            data[idx] |= (1 << acc_worker_id);
            idx += 8;
        }
    }
    
    free(data);
}

// Test case 7: Fully partitioned (gang+worker+vector)
void test_fully_partitioned(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // All three levels of partitioning
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(4) copy(data[0:n])
    {
        int gang_chunk = n / 2;
        int gang_start = acc_gang_id * gang_chunk;
        int gang_end = gang_start + gang_chunk;
        
        int worker_chunk = gang_chunk / 2;
        int worker_start = gang_start + acc_worker_id * worker_chunk;
        int worker_end = worker_start + worker_chunk;
        
        int idx = worker_start + acc_vector_id;
        while (idx < worker_end) {
            data[idx] = acc_gang_id * 100 + acc_worker_id * 10 + acc_vector_id;
            idx += 4;
        }
    }
    
    free(data);
}

// OpenMP equivalents to trigger similar partitioning logic
void test_omp_partitioning(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) data[i] = i;
    
    // OpenMP target with teams and distribute
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(32) map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] += omp_get_team_num() * 1000 + omp_get_thread_num();
    }
    
    // Nested parallelism for combined partitioning
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(64) simdlen(8) map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
    }
    
    free(data);
}

// Function to test invalid partition codes
void test_invalid_partition(int code) {
    // This might trigger the default case if the compiler
    // generates invalid partition codes through edge cases
    int n = 100;
    int *data = (int*)malloc(n * sizeof(int));
    
    // Use runtime value to potentially generate edge cases
    int gangs = (code % 3) + 1;
    int workers = (code % 4) + 1;
    int vectors = (code % 8) + 1;
    
    // Dynamic partitioning that might overflow
    #pragma acc parallel num_gangs(gangs) num_workers(workers) \
        vector_length(vectors) copy(data[0:n])
    {
        int idx = acc_gang_id * workers * vectors + 
                 acc_worker_id * vectors + acc_vector_id;
        if (idx < n) {
            data[idx] = code;
        }
    }
    
    free(data);
}

int main() {
    int n = 1024;
    
    printf("Testing OpenACC/OpenMP partition mapping...\n");
    
    // Test all valid partition cases
    test_gang_redundant(n);
    test_gang_partitioned(n);
    test_worker_partitioned(n);
    test_gang_worker_partitioned(n);
    test_vector_partitioned(n);
    test_gang_vector_partitioned(n);
    test_worker_vector_partitioned(n);
    test_fully_partitioned(n);
    
    // Test OpenMP partitioning
    test_omp_partitioning(n);
    
    // Test potential edge cases
    for (int i = 8; i < 12; i++) {
        test_invalid_partition(i);
    }
    
    // Force runtime evaluation with volatile
    volatile int check = 0;
    #pragma acc parallel copy(check)
    {
        if (acc_gang_id == 0 && acc_worker_id == 0 && acc_vector_id == 0) {
            // This might generate partition mapping for diagnostics
            check = 1;
        }
    }
    
    printf("All partition tests completed.\n");
    printf("If compiled with -fdump-tree-omplower, check for partition string usage.\n");
    
    return 0;
}
