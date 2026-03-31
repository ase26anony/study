#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

extern void test_reduction_temporaries(int n, int* results);
extern void test_scan_temporaries(int n, float* results);
extern void test_conditional_temporaries(int n, volatile int cond, int* results);
extern void test_enter_data(int n, float* data);

int main(int argc, char** argv) {
    const int N = 1000;
    int reduction_results[3] = {0};
    float scan_results[2] = {0.0f};
    int cond_results[2] = {0};
    float* data_array = (float*)malloc(N * sizeof(float));
    
    // Initialize data
    for (int i = 0; i < N; i++) {
        data_array[i] = (float)i / 10.0f;
    }
    
    // Use argv to create non-constant conditions
    volatile int condition = (argc > 1) ? atoi(argv[1]) : 1;
    
    // Test 1: Multiple reduction temporaries
    test_reduction_temporaries(N, reduction_results);
    
    // Test 2: Scan temporaries with inscan clause
    test_scan_temporaries(N, scan_results);
    
    // Test 3: Conditional temporaries with volatile
    test_conditional_temporaries(N, condition, cond_results);
    
    // Test 4: Enter data with 'to' mapper
    test_enter_data(N, data_array);
    
    // Compute checksum to prevent optimization
    int checksum = 0;
    checksum += reduction_results[0] + reduction_results[1] + reduction_results[2];
    checksum += (int)(scan_results[0] + scan_results[1]);
    checksum += cond_results[0] + cond_results[1];
    checksum += (int)data_array[N/2];
    
    printf("Checksum: %d\n", checksum);
    
    free(data_array);
    return 0;
}
