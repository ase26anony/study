/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc, lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to prevent inlining */
extern int parse_args(int argc, char **argv);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional wrapper to force SIMT transformation */
    if (enable) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        /* Alternative path without SIMD */
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    }
    return sum;
}

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int flag) {
    double result = 0.0;
    
    /* Complex control flow with goto to interact with artificial labels */
    if (flag > 0) {
        goto simd_path;
    } else {
        goto normal_path;
    }
    
simd_path:
    /* This SIMD loop should trigger the uncovered SIMT transformation */
    #pragma omp simd simdlen(4) reduction(+:result)
    for (int i = 0; i < n; i += 2) {  /* Non-unit stride to stress transformation */
        result += data[i];
    }
    goto finish;
    
normal_path:
    for (int i = 0; i < n; i++) {
        result += data[i];
    }
    
finish:
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int *array = (int*)malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd simdlen(16)
    for (int i = 0; i < size; i++) {
        array[i] = i % 100;
    }
    
    /* Regular parallel region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Thread %d initialized array\n", omp_get_thread_num());
        }
    }
    
    free(array);
}

/* Main test function with conditional SIMD */
int conditional_simd_reduction(int *data, int n, volatile int *flag) {
    int sum = 0;
    
    /* Ternary operator to force conditional wrapper generation */
    (*flag) ? 
    (
        #pragma omp simd simdlen(32) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i] * 2;
        }
    ) : 
    (
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    );
    
    return sum;
}

int main(int argc, char **argv) {
    const int N = 1024;
    int int_data[N];
    float float_data[N];
    double double_data[N];
    
    /* Parse command-line arguments for runtime control */
    int simd_flag = parse_args(argc, argv);
    if (simd_flag < 0) simd_flag = 0;
    
    /* Initialize data arrays */
    for (int i = 0; i < N; i++) {
        int_data[i] = (i * 3) % 97;
        float_data[i] = (float)(i * 0.1f);
        double_data[i] = (double)(i * 0.05);
    }
    
    /* Test 1: SIMD in main with volatile control */
    volatile int *ctrl = &use_simt_path;
    *ctrl = simd_flag;
    
    int result1 = conditional_simd_reduction(int_data, loop_control, ctrl);
    printf("Result 1 (int reduction): %d\n", result1);
    
    /* Test 2: SIMD in static function with runtime bounds */
    int bounds = (argc > 1) ? atoi(argv[1]) : 500;
    if (bounds > N) bounds = N;
    
    float result2 = static_simd_reduction(float_data, bounds, simd_enabled);
    printf("Result 2 (float reduction): %f\n", result2);
    
    /* Test 3: SIMD in noinline function with goto */
    double result3 = noinline_simd_reduction(double_data, N, simd_flag);
    printf("Result 3 (double reduction): %lf\n", result3);
    
    /* Test 4: Mixed OpenMP constructs */
    mixed_omp_constructs(256);
    
    /* Additional test with nested conditionals */
    for (int iter = 0; iter < 3; iter++) {
        int temp_sum = 0;
        
        /* Multiple conditional layers to stress transformation */
        if (iter % 2 == 0) {
            if (simd_flag > 0) {
                #pragma omp simd simdlen(8) reduction(+:temp_sum)
                for (int i = 0; i < 100; i++) {
                    temp_sum += int_data[i] * iter;
                }
            }
        }
        printf("Iteration %d: %d\n", iter, temp_sum);
    }
    
    return 0;
}

/* External function implementation */
int parse_args(int argc, char **argv) {
    if (argc > 2) {
        return atoi(argv[2]);
    }
    return 1; /* Default to SIMT path enabled */
}
