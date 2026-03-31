#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1000

#pragma omp declare target
float device_compute(float a, float b) {
    return a * b + (a - b);
}
#pragma omp end declare target

int main(int argc, char *argv[]) {
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float result = 0.0f;
    
    // Initialize arrays with non-constant values
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i + argc);  // Use argc to make trip count non-constant
        b[i] = (float)(i * 2);
    }
    
    // Use environment variable or argc to decide execution path
    int use_offload = 0;
    if (argc > 1) {
        use_offload = atoi(argv[1]);
    } else {
        char *env = getenv("USE_OFFLOAD");
        use_offload = (env != NULL && atoi(env) > 0);
    }
    
    if (use_offload) {
        // Complex nested OpenMP construct for SIMT transformation
        #pragma omp target teams distribute parallel for simd \
            map(to: a[0:N], b[0:N]) map(from: result) \
            reduction(+:result) num_teams(4) thread_limit(128)
        for (int i = 0; i < N; i++) {
            // Non-trivial loop body with device function call
            float temp = device_compute(a[i], b[i]);
            result += temp;
            
            // Additional arithmetic to create substantial GIMPLE
            result += (float)i / (float)N;
            if (i % 2 == 0) {
                result -= 0.5f;
            }
        }
        
        printf("Offloaded computation result: %f\n", result);
    } else {
        // Host fallback path - still using OpenMP but no offloading
        #pragma omp parallel for simd reduction(+:result)
        for (int i = 0; i < N; i++) {
            result += a[i] * b[i] + (a[i] - b[i]);
            result += (float)i / (float)N;
            if (i % 2 == 0) {
                result -= 0.5f;
            }
        }
        
        printf("Host computation result: %f\n", result);
    }
    
    // Additional test with volatile variable to prevent optimization
    volatile int dynamic_bound = N;
    float checksum = 0.0f;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N]) map(tofrom: checksum) \
        if(use_offload)  // Conditional offloading
    for (int i = 0; i < dynamic_bound; i++) {
        checksum += a[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    free(a);
    free(b);
    
    return 0;
}
