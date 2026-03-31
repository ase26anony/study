#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 1;
volatile int loop_control = 1000;

/* External function to prevent inlining */
extern int parse_args(int argc, char **argv);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional execution to force SIMT wrapper */
    if (enable) {
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

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int skip) {
    double result = 0.0;
    
    /* Use goto to create complex control flow */
    if (skip) {
        goto skip_simd;
    }
    
    /* SIMD loop with double precision */
    #pragma omp simd simdlen(4) reduction(+:result)
    for (int i = 0; i < n; i += 2) {
        result += data[i] * data[i + 1];
    }
    
skip_simd:
    /* Another SIMD loop that might be skipped */
    if (!skip) {
        #pragma omp simd simdlen(8) reduction(+:result)
        for (int i = 0; i < n; i++) {
            result += data[i] / (i + 1);
        }
    }
    
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int *array = malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < size; i++) {
        array[i] = i * 2;
    }
    
    /* Conditional SIMD reduction */
    int sum = 0;
    if (simd_enabled) {
        #pragma omp simd simdlen(16) reduction(+:sum)
        for (int i = 0; i < size; i++) {
            sum += array[i];
        }
    }
    
    printf("Mixed constructs sum: %d\n", sum);
    free(array);
}

/* Main function with command-line control */
int main(int argc, char **argv) {
    /* Parse command line for runtime control */
    int loop_count = parse_args(argc, argv);
    if (loop_count <= 0) loop_count = 100;
    
    /* Array initializations */
    int *int_data = malloc(loop_count * sizeof(int));
    float *float_data = malloc(loop_count * sizeof(float));
    double *double_data = malloc(loop_count * sizeof(double));
    
    for (int i = 0; i < loop_count; i++) {
        int_data[i] = i + 1;
        float_data[i] = (float)i * 0.5f;
        double_data[i] = (double)i * 0.25;
    }
    
    /* 1. Integer SIMD reduction in main with volatile condition */
    int int_sum = 0;
    volatile int *volatile_ptr = &simd_enabled;
    
    /* Ternary operator to force conditional SIMD */
    (*volatile_ptr) ? 
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i++) {
            int_sum += int_data[i];
        }
    :
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i++) {
            int_sum += int_data[i] * 2;
        };
    
    printf("Integer sum: %d\n", int_sum);
    
    /* 2. Float SIMD in static function with runtime bounds */
    float float_sum = static_simd_reduction(float_data, 
                                          loop_control % loop_count,
                                          use_simt_path);
    printf("Float sum: %f\n", float_sum);
    
    /* 3. Double SIMD in noinline function with goto */
    double double_sum = noinline_simd_reduction(double_data, 
                                               loop_count,
                                               argc > 2);
    printf("Double result: %lf\n", double_sum);
    
    /* 4. Mixed OpenMP constructs */
    mixed_omp_constructs(loop_count / 2);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* External function implementation */
int parse_args(int argc, char **argv) {
    if (argc > 1) {
        return atoi(argv[1]);
    }
    return 1000;
}
