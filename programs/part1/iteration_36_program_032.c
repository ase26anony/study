#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define MAX_GANGS 8
#define MAX_WORKERS 4
#define MAX_VECTORS 16

// Force compiler to generate partition mapping logic
volatile int force_partition_code = 0;

// Function to prevent optimization
void use_result(int val) {
    asm volatile("" : : "r"(val));
}

// Function that could trigger partition string mapping
void debug_partition_info(int code) {
    // This mimics the internal compiler logic that maps codes to strings
    if (code < 0 || code > 7) {
        // Could trigger default case
        fprintf(stderr, "Partition error: Invalid code %d\n", code);
    }
}

// Test case 0: Gang redundant (single gang)
void test_gang_redundant() {
    int data[ARRAY_SIZE];
    int sum = 0;
    
    #pragma acc parallel copy(data) copyout(sum) num_gangs(1) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i;
        }
        
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            sum += data[i];
        }
    }
    
    use_result(sum);
    force_partition_code = 0; // Simulate gang redundant code
    debug_partition_info(force_partition_code);
}

// Test case 1: Gang partitioned (multiple gangs)
void test_gang_partitioned() {
    int data[ARRAY_SIZE];
    int partial_sums[MAX_GANGS];
    
    #pragma acc parallel copy(data) copyout(partial_sums) num_gangs(MAX_GANGS) num_workers(1) vector_length(1)
    {
        int gang_id = acc_gang_id();
        
        #pragma acc loop gang
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * (gang_id + 1);
        }
        
        int local_sum = 0;
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = gang_id; i < ARRAY_SIZE; i += MAX_GANGS) {
            local_sum += data[i];
        }
        
        if (gang_id < MAX_GANGS) {
            partial_sums[gang_id] = local_sum;
        }
    }
    
    force_partition_code = 1; // Simulate gang partitioned code
    debug_partition_info(force_partition_code);
}

// Test case 2: Worker partitioned (single gang, multiple workers)
void test_worker_partitioned() {
    float data[ARRAY_SIZE];
    
    #pragma acc parallel copy(data) num_gangs(1) num_workers(MAX_WORKERS) vector_length(1)
    {
        int worker_id = acc_worker_id();
        
        #pragma acc loop worker
        for (int i = worker_id; i < ARRAY_SIZE; i += MAX_WORKERS) {
            data[i] = (float)i / (worker_id + 1.0f);
        }
    }
    
    force_partition_code = 2; // Simulate worker partitioned code
    debug_partition_info(force_partition_code);
}

// Test case 3: Gang+worker partitioned
void test_gang_worker_partitioned() {
    double data[ARRAY_SIZE][ARRAY_SIZE/16];
    
    #pragma acc parallel copy(data) num_gangs(MAX_GANGS/2) num_workers(MAX_WORKERS) vector_length(1)
    {
        int gang_id = acc_gang_id();
        int worker_id = acc_worker_id();
        
        #pragma acc loop gang worker collapse(2)
        for (int i = gang_id; i < ARRAY_SIZE; i += MAX_GANGS/2) {
            for (int j = worker_id; j < ARRAY_SIZE/16; j += MAX_WORKERS) {
                data[i][j] = (double)(i * j) / ((gang_id + 1) * (worker_id + 1));
            }
        }
    }
    
    force_partition_code = 3; // Simulate gang+worker partitioned code
    debug_partition_info(force_partition_code);
}

// Test case 4: Vector partitioned
void test_vector_partitioned() {
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copy(data) num_gangs(1) num_workers(1) vector_length(MAX_VECTORS)
    {
        #pragma acc loop vector
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * acc_vector_id();
        }
    }
    
    force_partition_code = 4; // Simulate vector partitioned code
    debug_partition_info(force_partition_code);
}

// Test case 5: Gang+vector partitioned
void test_gang_vector_partitioned() {
    int data[ARRAY_SIZE];
    
    #pragma acc parallel copy(data) num_gangs(MAX_GANGS) num_workers(1) vector_length(MAX_VECTORS)
    {
        int gang_id = acc_gang_id();
        int vector_id = acc_vector_id();
        
        #pragma acc loop gang vector
        for (int i = gang_id * MAX_VECTORS + vector_id; 
             i < ARRAY_SIZE; 
             i += MAX_GANGS * MAX_VECTORS) {
            data[i] = gang_id * 1000 + vector_id;
        }
    }
    
    force_partition_code = 5; // Simulate gang+vector partitioned code
    debug_partition_info(force_partition_code);
}

// Test case 6: Worker+vector partitioned
void test_worker_vector_partitioned() {
    float data[ARRAY_SIZE];
    
    #pragma acc parallel copy(data) num_gangs(1) num_workers(MAX_WORKERS) vector_length(MAX_VECTORS)
    {
        int worker_id = acc_worker_id();
        int vector_id = acc_vector_id();
        
        #pragma acc loop worker vector
        for (int i = worker_id * MAX_VECTORS + vector_id;
             i < ARRAY_SIZE;
             i += MAX_WORKERS * MAX_VECTORS) {
            data[i] = (float)(worker_id * 100 + vector_id);
        }
    }
    
    force_partition_code = 6; // Simulate worker+vector partitioned code
    debug_partition_info(force_partition_code);
}

// Test case 7: Fully partitioned (gang+worker+vector)
void test_fully_partitioned() {
    double data[ARRAY_SIZE][ARRAY_SIZE/32];
    
    #pragma acc parallel copy(data) num_gangs(MAX_GANGS/2) num_workers(MAX_WORKERS) vector_length(MAX_VECTORS/2)
    {
        int gang_id = acc_gang_id();
        int worker_id = acc_worker_id();
        int vector_id = acc_vector_id();
        
        #pragma acc loop gang worker vector collapse(2)
        for (int i = gang_id; i < ARRAY_SIZE; i += MAX_GANGS/2) {
            for (int j = worker_id * (MAX_VECTORS/2) + vector_id;
                 j < ARRAY_SIZE/32;
                 j += MAX_WORKERS * (MAX_VECTORS/2)) {
                data[i][j] = (double)(i * j) / 
                            ((gang_id + 1) * (worker_id + 1) * (vector_id + 1));
            }
        }
    }
    
    force_partition_code = 7; // Simulate fully partitioned code
    debug_partition_info(force_partition_code);
}

// Test default case with invalid partition codes
void test_invalid_partitions() {
    // Force generation of default case through boundary violations
    int invalid_codes[] = {-1, 8, 100, -100};
    
    for (int i = 0; i < 4; i++) {
        debug_partition_info(invalid_codes[i]);
    }
    
    // Also test with OpenMP to trigger different partition mappings
    #pragma omp target teams distribute parallel for simd \
        num_teams(4) thread_limit(32) simdlen(8)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // Complex computation to prevent optimization
        volatile int temp = i * omp_get_team_num() + omp_get_thread_num();
        use_result(temp);
    }
}

// OpenMP version to trigger different partition logic
void test_omp_partitioning() {
    int data[ARRAY_SIZE];
    
    // OpenMP target with teams and distribute
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data) num_teams(4) thread_limit(64) simdlen(8)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * (omp_get_team_num() + 1) + omp_get_thread_num();
    }
    
    // Nested parallelism to trigger complex partition mapping
    #pragma omp target teams distribute parallel for collapse(2) \
        num_teams(2) thread_limit(128)
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < ARRAY_SIZE) {
                data[idx] += omp_get_num_teams() * omp_get_num_threads();
            }
        }
    }
}

int main() {
    printf("Testing OpenACC/OpenMP partition mapping coverage...\n");
    
    // Test all valid partition cases (0-7)
    test_gang_redundant();
    printf("Test 0 (gang redundant) completed\n");
    
    test_gang_partitioned();
    printf("Test 1 (gang partitioned) completed\n");
    
    test_worker_partitioned();
    printf("Test 2 (worker partitioned) completed\n");
    
    test_gang_worker_partitioned();
    printf("Test 3 (gang+worker partitioned) completed\n");
    
    test_vector_partitioned();
    printf("Test 4 (vector partitioned) completed\n");
    
    test_gang_vector_partitioned();
    printf("Test 5 (gang+vector partitioned) completed\n");
    
    test_worker_vector_partitioned();
    printf("Test 6 (worker+vector partitioned) completed\n");
    
    test_fully_partitioned();
    printf("Test 7 (fully partitioned) completed\n");
    
    // Test invalid/default cases
    test_invalid_partitions();
    printf("Invalid partition tests completed\n");
    
    // Test OpenMP partitioning
    test_omp_partitioning();
    printf("OpenMP partitioning tests completed\n");
    
    printf("All partition mapping tests completed successfully\n");
    
    return 0;
}
