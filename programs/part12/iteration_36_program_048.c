/* Test program specifically designed to trigger the uncovered SIMT transformation
   code in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;

/* External function to force runtime evaluation */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional execution to force SIMT wrapper generation */
    if (flag > 0) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i] * 2.0f;
        }
    }
    
    return sum;
}

/* Noinline function to ensure separate function context */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int skip) {
    double result = 0.0;
    
    /* Use goto to create complex control flow */
    if (skip) {
        goto skip_simd;
    }
    
    /* This SIMD loop should trigger the uncovered code */
    #pragma omp simd simdlen(4) reduction(+:result)
    for (int i = 0; i < n; i++) {
        result += data[i] * (i % 2 ? -1.0 : 1.0);
    }
    
    goto end;
    
skip_simd:
    /* Alternative path without SIMD */
    for (int i = 0; i < n; i++) {
        result += data[i];
    }
    
end:
    return result;
}

/* Helper function with conditional SIMD execution */
int helper_simd_reduction(int *arr, int size, volatile int enable) {
    int sum = 0;
    
    /* Complex condition to prevent static elimination */
    int should_simd = (enable && (size > 0) && (simd_enabled || use_simt_path));
    
    /* This conditional wrapper should trigger the gbind creation */
    if (should_simd) {
        /* SIMD loop with multiple clauses */
        #pragma omp simd simdlen(16) reduction(+:sum) aligned(arr:32)
        for (int i = 0; i < size; i++) {
            sum += arr[i] * (i % 3);
        }
    } else {
        /* Fallback non-SIMD path */
        for (int i = 0; i < size; i++) {
            sum += arr[i];
        }
    }
    
    return sum;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int n) {
    int i;
    
    /* Regular parallel for - activates OMP infrastructure */
    #pragma omp parallel for num_threads(2)
    for (i = 0; i < n; i++) {
        /* Just some work */
        int x = i * i;
        (void)x;
    }
    
    /* Conditional SIMD inside parallel region */
    #pragma omp parallel
    {
        int local_sum = 0;
        
        /* Ternary operator as another form of conditional execution */
        int use_simd = (get_random() % 2) ? 1 : 0;
        
        if (use_simd) {
            #pragma omp simd simdlen(8) reduction(+:local_sum)
            for (int j = 0; j < 100; j++) {
                local_sum += j * omp_get_thread_num();
            }
        }
    }
}

int main(int argc, char **argv) {
    const int SIZE = 1024;
    int *int_data = (int*)malloc(SIZE * sizeof(int));
    float *float_data = (float*)malloc(SIZE * sizeof(float));
    double *double_data = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = i + 1;
        float_data[i] = (float)(i + 1) * 0.5f;
        double_data[i] = (double)(i + 1) * 0.25;
    }
    
    /* Parse command line arguments for runtime control */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 100;
    use_simt_path = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* 1. Helper function with conditional SIMD */
    int result1 = helper_simd_reduction(int_data, loop_count, simd_enabled);
    printf("Result 1 (int reduction): %d\n", result1);
    
    /* 2. Static function with SIMD */
    float result2 = static_simd_reduction(float_data, loop_count, use_simt_path);
    printf("Result 2 (float reduction): %f\n", result2);
    
    /* 3. Noinline function with goto-controlled SIMD */
    int skip_simd = (get_random() % 3 == 0) ? 1 : 0;
    double result3 = noinline_simd_reduction(double_data, loop_count, skip_simd);
    printf("Result 3 (double reduction): %lf\n", result3);
    
    /* 4. Mixed OpenMP constructs */
    mixed_omp_constructs(loop_count);
    
    /* Additional test: Nested conditional SIMD */
    for (int iter = 0; iter < 3; iter++) {
        int temp_sum = 0;
        
        /* Switch statement with SIMD in different cases */
        switch (iter) {
            case 0:
                #pragma omp simd simdlen(2) reduction(+:temp_sum)
                for (int i = 0; i < 50; i++) {
                    temp_sum += int_data[i];
                }
                break;
            case 1:
                #pragma omp simd simdlen(4) reduction(+:temp_sum)
                for (int i = 0; i < 50; i++) {
                    temp_sum += int_data[i] * 2;
                }
                break;
            default:
                #pragma omp simd simdlen(8) reduction(+:temp_sum)
                for (int i = 0; i < 50; i++) {
                    temp_sum += int_data[i] * 3;
                }
        }
        printf("Iteration %d: %d\n", iter, temp_sum);
    }
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* Dummy implementation of external function */
int get_random(void) {
    static int seed = 12345;
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}
