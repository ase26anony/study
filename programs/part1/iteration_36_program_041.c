#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

// Function to prevent optimization
void use_int(int x) {
    asm volatile("" : : "r"(x));
}

// Function to prevent dead code elimination
void use_ptr(void* p) {
    asm volatile("" : : "r"(p));
}

// Test case 0: Gang redundant (single gang)
void test_gang_redundant(int n) {
    volatile int force_runtime = n;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang num_gangs(1) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    // Use results to prevent optimization
    for (int i = 0; i < n; i++) {
        use_int(data[i]);
    }
    
    free(data);
}

// Test case 1: Gang partitioned (multiple gangs)
void test_gang_partitioned(int n) {
    volatile int gangs = 4;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang num_gangs(gangs) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i + (i % gangs);
    }
    
    use_ptr(data);
    free(data);
}

// Test case 2: Worker partitioned (single gang, multiple workers)
void test_worker_partitioned(int n) {
    volatile int workers = 4;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang num_gangs(1) num_workers(workers) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 3;
    }
    
    use_ptr(data);
    free(data);
}

// Test case 3: Gang+Worker partitioned
void test_gang_worker_partitioned(int n) {
    volatile int gangs = 2;
    volatile int workers = 4;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang num_gangs(gangs) num_workers(workers) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = (i % gangs) + (i % workers);
    }
    
    use_ptr(data);
    free(data);
}

// Test case 4: Vector partitioned
void test_vector_partitioned(int n) {
    volatile int vector_len = 32;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop vector vector_length(vector_len) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i << 2;
    }
    
    use_ptr(data);
    free(data);
}

// Test case 5: Gang+Vector partitioned
void test_gang_vector_partitioned(int n) {
    volatile int gangs = 4;
    volatile int vector_len = 16;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang vector num_gangs(gangs) vector_length(vector_len) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = (i % gangs) * vector_len;
    }
    
    use_ptr(data);
    free(data);
}

// Test case 6: Worker+Vector partitioned
void test_worker_vector_partitioned(int n) {
    volatile int workers = 4;
    volatile int vector_len = 8;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop worker vector num_workers(workers) vector_length(vector_len) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = (i % workers) + (i % vector_len);
    }
    
    use_ptr(data);
    free(data);
}

// Test case 7: Fully partitioned (Gang+Worker+Vector)
void test_fully_partitioned(int n) {
    volatile int gangs = 2;
    volatile int workers = 2;
    volatile int vector_len = 4;
    int* data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang worker vector \
        num_gangs(gangs) num_workers(workers) vector_length(vector_len) \
        copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        int gang_id = i % gangs;
        int worker_id = (i / gangs) % workers;
        int vector_id = i % vector_len;
        data[i] = gang_id * 100 + worker_id * 10 + vector_id;
    }
    
    use_ptr(data);
    free(data);
}

// OpenMP equivalents to trigger similar partitioning logic
void test_omp_partitioning(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    
    // OpenMP target with teams and distribute
    #pragma omp target teams distribute parallel for \
        num_teams(4) thread_limit(32) map(from:data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * omp_get_team_num() + omp_get_thread_num();
    }
    
    // Nested parallelism for combined partitioning
    #pragma omp target teams distribute parallel for simd \
        num_teams(2) thread_limit(64) simdlen(8) map(from:data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = (i % 8) + (omp_get_team_num() * 10);
    }
    
    use_ptr(data);
    free(data);
}

// Function to test invalid partition codes (default case)
void test_invalid_partition(int n) {
    // This function uses runtime values to potentially generate
    // invalid partition codes through variable clauses
    volatile int invalid_gangs = -1;
    volatile int invalid_workers = 0;
    int* data = (int*)malloc(n * sizeof(int));
    
    // Use conditional compilation to create edge cases
    #ifdef __OPTIMIZE__
    #pragma acc parallel loop gang num_gangs(invalid_gangs) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i;
    }
    #endif
    
    // Another potential invalid case
    #pragma acc parallel loop vector vector_length(0) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    use_ptr(data);
    free(data);
}

// Template function to generate different partition types at compile time
template<int PartitionType>
void test_template_partition(int n) {
    int* data = (int*)malloc(n * sizeof(int));
    
    if constexpr (PartitionType == 0) {
        #pragma acc parallel loop gang num_gangs(1) copyout(data[0:n])
        for (int i = 0; i < n; i++) data[i] = i;
    } else if constexpr (PartitionType == 1) {
        #pragma acc parallel loop gang num_gangs(4) copyout(data[0:n])
        for (int i = 0; i < n; i++) data[i] = i * 2;
    } else if constexpr (PartitionType == 2) {
        #pragma acc parallel loop worker num_workers(4) copyout(data[0:n])
        for (int i = 0; i < n; i++) data[i] = i * 3;
    }
    // ... more cases
    
    use_ptr(data);
    free(data);
}

int main() {
    const int N = 1024;
    volatile int runtime_n = N;  // Force runtime evaluation
    
    printf("Testing OpenACC/OpenMP partition mapping coverage...\n");
    
    // Test all valid partition cases
    test_gang_redundant(runtime_n);
    printf("Case 0 tested\n");
    
    test_gang_partitioned(runtime_n);
    printf("Case 1 tested\n");
    
    test_worker_partitioned(runtime_n);
    printf("Case 2 tested\n");
    
    test_gang_worker_partitioned(runtime_n);
    printf("Case 3 tested\n");
    
    test_vector_partitioned(runtime_n);
    printf("Case 4 tested\n");
    
    test_gang_vector_partitioned(runtime_n);
    printf("Case 5 tested\n");
    
    test_worker_vector_partitioned(runtime_n);
    printf("Case 6 tested\n");
    
    test_fully_partitioned(runtime_n);
    printf("Case 7 tested\n");
    
    // Test OpenMP partitioning
    test_omp_partitioning(runtime_n);
    printf("OpenMP partitioning tested\n");
    
    // Test template instantiations
    test_template_partition<0>(runtime_n);
    test_template_partition<1>(runtime_n);
    test_template_partition<2>(runtime_n);
    
    // Test invalid partition codes (for default case)
    test_invalid_partition(runtime_n);
    printf("Invalid partition cases tested\n");
    
    printf("All partition tests completed.\n");
    
    // Force compiler to consider all paths
    volatile int check = 0;
    if (check) {
        // This unreachable code ensures compiler considers all switch cases
        const char* dummy;
        switch (check) {
            case 0: dummy = "gang redundant"; break;
            case 1: dummy = "gang partitioned"; break;
            case 2: dummy = "worker partitioned"; break;
            case 3: dummy = "gang+worker partitioned"; break;
            case 4: dummy = "vector partitioned"; break;
            case 5: dummy = "gang+vector partitioned"; break;
            case 6: dummy = "worker+vector partitioned"; break;
            case 7: dummy = "fully partitioned"; break;
            default: dummy = "<illegal>"; break;
        }
        use_ptr((void*)dummy);
    }
    
    return 0;
}
