#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int dynamic_control = 0;

/* External function to force control flow */
extern int get_external_value(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional wrapper around SIMD loop */
    if (flag || simd_enabled) {
        #pragma omp simd simdlen(8) reduction(+:sum) aligned(data:32)
        for (int i = 0; i < n; i++) {
            sum += data[i] * data[i];
        }
    } else {
        /* Alternative path without SIMD */
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    }
    return sum;
}

/* Noinline function with goto-based control flow */
__attribute__((noinline)) 
double noinline_simd_compute(double *arr, int size, int threshold) {
    double result = 0.0;
    int use_simd = 0;
    
    /* Complex conditional to force label creation */
    if (threshold > 100) {
        use_simd = 1;
        goto simd_section;
    } else if (threshold > 50) {
        use_simd = (get_external_value() % 2);
        if (use_simd) goto simd_section;
    }
    
    /* Non-SIMD fallback */
    for (int i = 0; i < size; i++) {
        result += arr[i];
    }
    goto end_computation;
    
simd_section:
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(4) reduction(+:result) linear(i:1) \
            safelen(8) aligned(arr:64)
    for (int i = 0; i < size; i++) {
        result += arr[i] * 2.0;
    }
    
end_computation:
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_operations(int *output, int n) {
    int i;
    
    /* Regular parallel for */
    #pragma omp parallel for private(i) schedule(static)
    for (i = 0; i < n; i++) {
        output[i] = i * i;
    }
    
    /* Conditional SIMD with volatile control */
    volatile int *ptr = &use_simt_path;
    if (*ptr || dynamic_control) {
        int sum = 0;
        #pragma omp simd simdlen(16) reduction(+:sum)
        for (i = 0; i < n; i++) {
            sum += output[i];
        }
        output[0] = sum;
    }
}

/* Main function with diverse SIMD constructs */
int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <size> <threshold>\n", argv[0]);
        return 1;
    }
    
    int size = atoi(argv[1]);
    int threshold = atoi(argv[2]);
    
    /* Set control variables from command line */
    simd_enabled = (threshold > 0);
    use_simt_path = (size > 1000);
    dynamic_control = get_external_value();
    
    /* Array allocations with different alignments */
    int *int_data = (int*)aligned_alloc(32, size * sizeof(int));
    float *float_data = (float*)aligned_alloc(64, size * sizeof(float));
    double *double_data = (double*)aligned_alloc(128, size * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        int_data[i] = i + 1;
        float_data[i] = (float)(i + 1) * 0.5f;
        double_data[i] = (double)(i + 1) * 0.25;
    }
    
    /* 1. SIMD in main with conditional execution */
    int int_sum = 0;
    
    /* Ternary operator to force conditional wrapper */
    (threshold % 2) ? 
    (
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < size; i += 2) {
            int_sum += int_data[i];
        }
    ) : 
    (
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 1; i < size; i += 2) {
            int_sum += int_data[i] * 2;
        }
    );
    
    /* 2. Call static function with SIMD */
    float float_result = static_simd_reduction(float_data, size, threshold);
    
    /* 3. Call noinline function with goto-based SIMD */
    double double_result = noinline_simd_compute(double_data, size, threshold);
    
    /* 4. Mixed OpenMP operations */
    mixed_omp_operations(int_data, size);
    
    /* Print results to prevent elimination */
    printf("Results:\n");
    printf("  Integer sum: %d\n", int_sum);
    printf("  Float reduction: %f\n", float_result);
    printf("  Double compute: %lf\n", double_result);
    printf("  First element: %d\n", int_data[0]);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* External function implementation */
int get_external_value(void) {
    static int counter = 0;
    return counter++ % 3;
}
