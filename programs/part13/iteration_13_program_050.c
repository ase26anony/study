#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

// Helper function to verify results
int verify_result(const char* test_name, int expected, int actual) {
    if (expected != actual) {
        printf("FAIL: %s - expected %d, got %d\n", test_name, expected, actual);
        return 0;
    }
    printf("PASS: %s\n", test_name);
    return 1;
}

int main() {
    int i, j, k;
    int *a, *b, *c, *d;
    int sum = 0, max_val = 0;
    int success = 1;
    
    // Allocate and initialize arrays
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * M * sizeof(int));
    d = (int*)malloc(N * M * P * sizeof(int));
    
    for (i = 0; i < N; i++) {
        a[i] = i % 100;
        b[i] = (i + 1) % 100;
    }
    
    for (i = 0; i < N * M; i++) {
        c[i] = i % 50;
    }
    
    for (i = 0; i < N * M * P; i++) {
        d[i] = i % 25;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    // ============================================
    // Test 1: Gang redundant partitioning
    // Simple parallel region with gang-level redundancy
    // Likely maps to partition code 0
    // ============================================
    #pragma acc parallel copy(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(4) num_workers(1) vector_length(1)
    {
        int gang_id = __pgi_gangidx();
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (c[i] != (a[i] + b[i])) {
            printf("FAIL: Test 1 gang redundant\n");
            success = 0;
            break;
        }
    }
    if (success) printf("PASS: Test 1 gang redundant\n");
    
    // ============================================
    // Test 2: Gang partitioned
    // Using gang-level parallelism only
    // Likely maps to partition code 1
    // ============================================
    sum = 0;
    #pragma acc parallel copyin(a[0:N]) copy(sum) \
        num_gangs(8) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang reduction(+:sum)
        for (i = 0; i < N; i++) {
            sum += a[i];
        }
    }
    
    // Verify
    int expected_sum = 0;
    for (i = 0; i < N; i++) expected_sum += a[i];
    success &= verify_result("Test 2 gang partitioned", expected_sum, sum);
    
    // ============================================
    // Test 3: Worker partitioned
    // Using worker-level parallelism within gangs
    // Likely maps to partition code 2
    // ============================================
    #pragma acc parallel copy(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(2) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker
        for (i = 0; i < N; i++) {
            c[i] = a[i] * b[i];
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (c[i] != (a[i] * b[i])) {
            printf("FAIL: Test 3 worker partitioned\n");
            success = 0;
            break;
        }
    }
    if (success) printf("PASS: Test 3 worker partitioned\n");
    
    // ============================================
    // Test 4: Gang+worker partitioned
    // Nested parallelism with gang and worker levels
    // Likely maps to partition code 3
    // ============================================
    #pragma acc parallel copy(c[0:N*M]) \
        num_gangs(4) num_workers(4) vector_length(1)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                c[i * M + j] = i + j;
            }
        }
    }
    
    // Verify
    int correct = 1;
    for (i = 0; i < N && correct; i++) {
        for (j = 0; j < M && correct; j++) {
            if (c[i * M + j] != (i + j)) {
                printf("FAIL: Test 4 gang+worker partitioned\n");
                success = 0;
                correct = 0;
            }
        }
    }
    if (correct) printf("PASS: Test 4 gang+worker partitioned\n");
    
    // ============================================
    // Test 5: Vector partitioned
    // Using vector-level parallelism
    // Likely maps to partition code 4
    // ============================================
    #pragma acc parallel copy(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(1) num_workers(1) vector_length(256)
    {
        #pragma acc loop vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] - b[i];
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (c[i] != (a[i] - b[i])) {
            printf("FAIL: Test 5 vector partitioned\n");
            success = 0;
            break;
        }
    }
    if (success) printf("PASS: Test 5 vector partitioned\n");
    
    // ============================================
    // Test 6: Gang+vector partitioned
    // Gang and vector parallelism combined
    // Likely maps to partition code 5
    // ============================================
    max_val = 0;
    #pragma acc parallel copyin(a[0:N]) copy(max_val) \
        num_gangs(4) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang vector reduction(max:max_val)
        for (i = 0; i < N; i++) {
            if (a[i] > max_val) max_val = a[i];
        }
    }
    
    // Verify
    int expected_max = 0;
    for (i = 0; i < N; i++) {
        if (a[i] > expected_max) expected_max = a[i];
    }
    success &= verify_result("Test 6 gang+vector partitioned", expected_max, max_val);
    
    // ============================================
    // Test 7: Worker+vector partitioned
    // Worker and vector parallelism combined
    // Likely maps to partition code 6
    // ============================================
    #pragma acc parallel copy(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(1) num_workers(4) vector_length(64)
    {
        #pragma acc loop worker vector
        for (i = 0; i < N; i++) {
            c[i] = a[i] / (b[i] + 1);  // Avoid division by zero
        }
    }
    
    // Verify
    for (i = 0; i < N; i++) {
        if (c[i] != (a[i] / (b[i] + 1))) {
            printf("FAIL: Test 7 worker+vector partitioned\n");
            success = 0;
            break;
        }
    }
    if (success) printf("PASS: Test 7 worker+vector partitioned\n");
    
    // ============================================
    // Test 8: Fully partitioned
    // Using all three levels of parallelism
    // Likely maps to partition code 7
    // ============================================
    long long total_sum = 0;
    #pragma acc parallel copyin(d[0:N*M*P]) copy(total_sum) \
        num_gangs(8) num_workers(4) vector_length(32)
    {
        #pragma acc loop gang worker vector reduction(+:total_sum)
        for (i = 0; i < N * M * P; i++) {
            total_sum += d[i];
        }
    }
    
    // Verify
    long long expected_total = 0;
    for (i = 0; i < N * M * P; i++) {
        expected_total += d[i];
    }
    if (total_sum == expected_total) {
        printf("PASS: Test 8 fully partitioned\n");
    } else {
        printf("FAIL: Test 8 fully partitioned - expected %lld, got %lld\n", 
               expected_total, total_sum);
        success = 0;
    }
    
    // ============================================
    // Test 9: Runtime-determined partitioning
    // Using conditional clauses and async operations
    // ============================================
    int async_id = 1;
    int use_gang = 1;
    
    #pragma acc parallel copy(a[0:N], b[0:N]) copyout(c[0:N]) \
        async(async_id) if(use_gang)
    {
        #pragma acc loop
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 2;
        }
    }
    
    #pragma acc wait(async_id)
    
    // Verify
    for (i = 0; i < N; i++) {
        if (c[i] != (a[i] + b[i] * 2)) {
            printf("FAIL: Test 9 runtime-determined\n");
            success = 0;
            break;
        }
    }
    if (success) printf("PASS: Test 9 runtime-determined\n");
    
    // ============================================
    // Test 10: Multi-device and explicit management
    // Try to trigger different device paths
    // ============================================
    acc_device_t dev_type = acc_get_device_type();
    printf("Device type: %d\n", (int)dev_type);
    
    // Try to set device (may fail if only one device)
    int num_devices = acc_get_num_devices(dev_type);
    if (num_devices > 1) {
        acc_set_device_num(1, dev_type);
    }
    
    // Final parallel region with complex data clauses
    #pragma acc data copyin(a[0:N], b[0:N]) create(c[0:N]) copyout(d[0:100])
    {
        #pragma acc parallel present(a, b, c, d)
        {
            #pragma acc loop gang worker vector
            for (i = 0; i < 100; i++) {
                d[i] = a[i] + b[i] + c[i];
            }
        }
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    
    if (success) {
        printf("\nAll tests passed successfully!\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}
