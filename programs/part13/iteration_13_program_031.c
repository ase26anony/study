#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>

#define N 1024
#define M 256
#define P 128

// Helper function to verify results
int verify_array(int *arr, int size, int expected) {
    for (int i = 0; i < size; i++) {
        if (arr[i] != expected) {
            printf("Verification failed at index %d: got %d, expected %d\n", 
                   i, arr[i], expected);
            return 0;
        }
    }
    return 1;
}

int main() {
    int *a, *b, *c, *d;
    int scalar = 0;
    int reduction_result = 0;
    int success = 1;
    
    // Allocate and initialize arrays
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * M * sizeof(int));
    d = (int*)malloc(N * M * P * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
    }
    
    for (int i = 0; i < N * M; i++) {
        c[i] = i % 100;
    }
    
    for (int i = 0; i < N * M * P; i++) {
        d[i] = i % 50;
    }
    
    printf("Starting OpenACC tests to cover partition mapping...\n");
    
    // Test 1: Gang redundant partitioning (likely case 0)
    // Simple parallel region with scalar reduction
    printf("\nTest 1: Gang redundant partitioning\n");
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
        num_gangs(4) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    // Verify Test 1
    for (int i = 0; i < N; i++) {
        if (c[i] != N) {
            printf("Test 1 failed at index %d: %d != %d\n", i, c[i], N);
            success = 0;
            break;
        }
    }
    
    // Test 2: Gang partitioned (likely case 1)
    printf("\nTest 2: Gang partitioned\n");
    int *e = (int*)malloc(M * sizeof(int));
    for (int i = 0; i < M; i++) e[i] = i;
    
    #pragma acc parallel copy(e[0:M]) num_gangs(8) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            e[i] = e[i] * 2;
        }
    }
    
    // Test 3: Worker partitioned (likely case 2)
    printf("\nTest 3: Worker partitioned\n");
    int *f = (int*)malloc(M * sizeof(int));
    for (int i = 0; i < M; i++) f[i] = i;
    
    #pragma acc parallel copy(f[0:M]) num_gangs(1) num_workers(4) vector_length(1)
    {
        #pragma acc loop worker
        for (int i = 0; i < M; i++) {
            f[i] = f[i] + 100;
        }
    }
    
    // Test 4: Gang+worker partitioned (likely case 3)
    printf("\nTest 4: Gang+worker partitioned\n");
    int *g = (int*)malloc(N * M * sizeof(int));
    for (int i = 0; i < N * M; i++) g[i] = i;
    
    #pragma acc parallel copy(g[0:N*M]) num_gangs(4) num_workers(4) vector_length(1)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N * M; i++) {
            g[i] = g[i] * 3;
        }
    }
    
    // Test 5: Vector partitioned (likely case 4)
    printf("\nTest 5: Vector partitioned\n");
    int *h = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) h[i] = i;
    
    #pragma acc parallel copy(h[0:N]) num_gangs(1) num_workers(1) vector_length(256)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            h[i] = h[i] + 500;
        }
    }
    
    // Test 6: Gang+vector partitioned (likely case 5)
    printf("\nTest 6: Gang+vector partitioned\n");
    int *k = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) k[i] = i;
    
    #pragma acc parallel copy(k[0:N]) num_gangs(4) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            k[i] = k[i] * 2 + 1;
        }
    }
    
    // Test 7: Worker+vector partitioned (likely case 6)
    printf("\nTest 7: Worker+vector partitioned\n");
    int *l = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) l[i] = i;
    
    #pragma acc parallel copy(l[0:N]) num_gangs(1) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            l[i] = l[i] - 100;
        }
    }
    
    // Test 8: Fully partitioned (likely case 7)
    printf("\nTest 8: Fully partitioned\n");
    int *m = (int*)malloc(N * M * P * sizeof(int));
    for (int i = 0; i < N * M * P; i++) m[i] = i % 100;
    
    #pragma acc parallel copy(m[0:N*M*P]) \
        num_gangs(8) num_workers(4) vector_length(32)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N * M * P; i++) {
            m[i] = m[i] * 2 + 1;
        }
    }
    
    // Test 9: Complex nested parallelism with reductions
    printf("\nTest 9: Complex nested parallelism\n");
    int outer_sum = 0;
    int inner_sum = 0;
    
    #pragma acc parallel copyin(a[0:N], b[0:N]) \
        copy(outer_sum, inner_sum) reduction(+:outer_sum, inner_sum) \
        num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang reduction(+:outer_sum)
        for (int i = 0; i < N; i++) {
            int local_sum = 0;
            
            #pragma acc loop worker vector reduction(+:local_sum)
            for (int j = 0; j < M; j++) {
                local_sum += a[i] + b[j % N];
            }
            
            outer_sum += local_sum;
            inner_sum += i;
        }
    }
    
    // Test 10: Async operations with data movement
    printf("\nTest 10: Async operations\n");
    int async_id = 1;
    int *n = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) n[i] = i;
    
    #pragma acc data copy(n[0:N])
    {
        #pragma acc parallel async(async_id) num_gangs(4) num_workers(2) vector_length(32)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                n[i] = n[i] * 3;
            }
        }
        #pragma acc wait(async_id)
    }
    
    // Test 11: Multi-dimensional arrays with tile clauses
    printf("\nTest 11: Multi-dimensional tiling\n");
    int matrix[128][128];
    
    #pragma acc parallel copy(matrix) num_gangs(8) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker tile(16, 16)
        for (int i = 0; i < 128; i++) {
            for (int j = 0; j < 128; j++) {
                matrix[i][j] = i * 1000 + j;
            }
        }
    }
    
    // Test 12: Runtime API calls that might trigger different paths
    printf("\nTest 12: Runtime API usage\n");
    acc_device_t dev_type = acc_get_device_type();
    printf("Device type: %d\n", dev_type);
    
    // Try to set device (might trigger different internal paths)
    if (acc_get_num_devices(dev_type) > 0) {
        acc_set_device_num(0, dev_type);
    }
    
    // Test 13: Firstprivate and private clauses
    printf("\nTest 13: Firstprivate/private clauses\n");
    int private_var = 42;
    int firstprivate_var = 100;
    
    #pragma acc parallel copyout(c[0:10]) \
        private(private_var) firstprivate(firstprivate_var) \
        num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < 10; i++) {
            private_var = i;
            c[i] = private_var + firstprivate_var;
        }
    }
    
    // Test 14: Conditional parallel regions
    printf("\nTest 14: Conditional execution\n");
    int condition = 1;
    int *cond_arr = (int*)malloc(100 * sizeof(int));
    
    #pragma acc parallel copy(cond_arr[0:100]) if(condition) \
        num_gangs(4) num_workers(1) vector_length(64)
    {
        #pragma acc loop gang
        for (int i = 0; i < 100; i++) {
            cond_arr[i] = i * i;
        }
    }
    
    // Test 15: Multiple data clauses with different mappings
    printf("\nTest 15: Mixed data clauses\n");
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    int *z = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        x[i] = i;
        y[i] = 2 * i;
    }
    
    #pragma acc data copyin(x[0:N]) copyout(z[0:N]) create(y[0:N])
    {
        #pragma acc parallel present(x, y, z) \
            num_gangs(8) num_workers(2) vector_length(16)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                y[i] = x[i] * 2;
                z[i] = y[i] + x[i];
            }
        }
    }
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
    free(g);
    free(h);
    free(k);
    free(l);
    free(m);
    free(n);
    free(x);
    free(y);
    free(z);
    free(cond_arr);
    
    if (success) {
        printf("\nAll tests completed successfully!\n");
        return 0;
    } else {
        printf("\nSome tests failed!\n");
        return 1;
    }
}
