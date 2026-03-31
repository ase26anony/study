#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

extern void test_reductions(int n, int *checksum);
extern void test_scans(int n, int *checksum);
extern void test_conditionals(volatile int cond, int *checksum);
extern void test_enter_data(int n, int *checksum);

void test_reductions(int n, int *checksum) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    long lsum = 0;
    
    // Multiple reduction variables of different types
    #pragma omp parallel reduction(+:sum, fsum, dsum, lsum)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
            fsum += i * 0.5f;
            dsum += i * 0.25;
            lsum += i * 2L;
        }
        
        // Nested reduction with taskloop
        #pragma omp taskloop reduction(*:sum)
        for (int i = 1; i <= 10; i++) {
            sum *= i;
        }
    }
    
    // Combined parallel for simd with reduction
    int arr[100];
    for (int i = 0; i < 100; i++) arr[i] = i;
    
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    *checksum += sum + (int)fsum + (int)dsum + (int)lsum;
}

void test_scans(int n, int *checksum) {
    int sum = 0;
    int partial_sums[100];
    
    // SIMD with inscan reduction
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan inclusive(sum)
        partial_sums[i] = sum;
    }
    
    // For loop with scan directive
    int scan_sum = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        scan_sum += i * 2;
        #pragma omp scan exclusive(scan_sum)
        partial_sums[i] += scan_sum;
    }
    
    for (int i = 0; i < n && i < 100; i++) {
        *checksum += partial_sums[i];
    }
}

void test_conditionals(volatile int cond, int *checksum) {
    int local_sum = 0;
    
    // Parallel with non-constant if clause
    #pragma omp parallel if(cond > 0) reduction(+:local_sum)
    {
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            local_sum += i;
        }
    }
    
    // Another conditional with function argument
    #pragma omp parallel if(cond < 100) 
    {
        #pragma omp for
        for (int i = 0; i < 50; i++) {
            local_sum += i * 2;
        }
    }
    
    // Combined directive with if clause
    #pragma omp parallel for if(cond != 0)
    for (int i = 0; i < 100; i++) {
        local_sum += i * 3;
    }
    
    *checksum += local_sum;
}

void test_enter_data(int n, int *checksum) {
    int *dynamic_array = (int*)malloc(n * sizeof(int));
    
    // Enter data with to mapper
    #pragma omp enter data map(to:dynamic_array[0:n])
    
    // Initialize array in parallel
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        dynamic_array[i] = i;
    }
    
    // Compute sum
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += dynamic_array[i];
    }
    
    // Exit data
    #pragma omp exit data map(from:dynamic_array[0:n])
    
    *checksum += sum;
    
    free(dynamic_array);
}
