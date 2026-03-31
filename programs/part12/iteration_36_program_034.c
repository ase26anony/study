/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to force runtime evaluation */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional wrapper - forces compiler to generate control flow */
    if (enable) {
        /* This SIMD loop should trigger expand_omp_simt_simd */
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i] * data[i];
        }
    } else {
        /* Fallback non-SIMD path */
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    }
    
    return sum;
}

/* Noinline function to ensure separate context */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int flag) {
    double product = 1.0;
    int use_simd = 0;
    
    /* Complex conditional with goto to interact with artificial labels */
    if (flag > 0) {
        use_simd = 1;
        goto simd_path;
    } else {
        goto scalar_path;
    }
    
simd_path:
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(4) reduction(*:product) aligned(data:32)
    for (int i = 0; i < n; i++) {
        product *= (data[i] + 1.0);
    }
    goto end;
    
scalar_path:
    for (int i = 0; i < n; i++) {
        product *= data[i];
    }
    
end:
    return product;
}

/* Function with nested control flow around SIMD */
void helper_function(int *results, int size, volatile int *control) {
    int local_sum = 0;
    
    /* Ternary operator to force conditional SIMD generation */
    int limit = (*control > 0) ? size : size / 2;
    
    /* This should trigger the SIMT transformation with gbind */
    if (simd_enabled && use_simt_path) {
        /* SIMD with explicit simdlen */
        #pragma omp simd simdlen(16) reduction(+:local_sum)
        for (int i = 0; i < limit; i++) {
            local_sum += results[i] * (i % 10);
        }
    } else {
        /* Different SIMD variant */
        #pragma omp simd simdlen(8) reduction(+:local_sum)
        for (int i = 0; i < limit; i++) {
            local_sum += results[i];
        }
    }
    
    results[0] = local_sum;
}

/* Mixed OpenMP constructs to activate full OMP lowering */
void parallel_region(int *data, int n) {
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < n; i++) {
        data[i] = i * 2;
    }
}

int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int array_size = 1024;
    int use_simt = 0;
    
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size <= 0) array_size = 1024;
    }
    if (argc > 2) {
        use_simt = atoi(argv[2]);
        use_simt_path = use_simt;  /* Set volatile */
    }
    
    /* Allocate and initialize arrays with different data types */
    int *int_data = (int*)malloc(array_size * sizeof(int));
    float *float_data = (float*)malloc(array_size * sizeof(float));
    double *double_data = (double*)malloc(array_size * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < array_size; i++) {
        int_data[i] = (i % 100) + 1;
        float_data[i] = (float)(i % 100) * 0.1f;
        double_data[i] = (double)(i % 100) * 0.01;
    }
    
    /* 1. Integer SIMD reduction in helper function with volatile control */
    printf("Test 1: Integer SIMD reduction\n");
    helper_function(int_data, array_size, &loop_control);
    printf("  Result: %d\n", int_data[0]);
    
    /* 2. Float SIMD reduction in static function with runtime condition */
    printf("Test 2: Float SIMD reduction\n");
    float float_result = static_simd_reduction(float_data, array_size, 
                                              (array_size > 500) ? 1 : 0);
    printf("  Result: %f\n", float_result);
    
    /* 3. Double SIMD reduction in noinline function with goto */
    printf("Test 3: Double SIMD reduction\n");
    double double_result = noinline_simd_reduction(double_data, array_size, 
                                                   use_simt);
    printf("  Result: %lf\n", double_result);
    
    /* 4. Mixed OpenMP parallel+SIMD to ensure infrastructure is active */
    printf("Test 4: Mixed OpenMP parallel for simd\n");
    parallel_region(int_data, array_size);
    printf("  First element: %d\n", int_data[0]);
    
    /* 5. Additional SIMD loop with varying parameters */
    printf("Test 5: Additional SIMD with different simdlen\n");
    {
        long long big_sum = 0;
        /* Varying SIMD length based on runtime */
        int simd_len = (use_simt) ? 32 : 16;
        
        #pragma omp simd simdlen(simd_len) reduction(+:big_sum)
        for (int i = 0; i < array_size; i++) {
            big_sum += (long long)int_data[i] * i;
        }
        printf("  Big sum: %lld\n", big_sum);
    }
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
