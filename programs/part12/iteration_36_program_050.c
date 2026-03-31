/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc, lines 2941-2975. This creates conditional wrappers with
   artificial labels and bind expressions for SIMD loops. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to force runtime evaluation */
extern int get_random(void);

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
        /* Non-SIMD fallback */
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    }
    return sum;
}

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int skip) {
    double product = 1.0;
    
    /* Goto to interact with artificial labels */
    if (skip) goto skip_simd;
    
    /* SIMD loop with conditional wrapper */
    #pragma omp simd simdlen(4) reduction(*:product)
    for (int i = 0; i < n; i++) {
        product *= data[i];
    }
    
    goto end;
    
skip_simd:
    /* Alternative computation */
    for (int i = 0; i < n; i++) {
        product *= 0.5;
    }
    
end:
    return product;
}

/* Main function with multiple SIMD constructs */
int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int loop_size = 1024;
    int use_simd = 1;
    
    if (argc > 1) loop_size = atoi(argv[1]);
    if (argc > 2) use_simd = atoi(argv[2]);
    
    /* Force runtime evaluation */
    use_simt_path = get_random() & 1;
    
    /* Array 1: Integer reduction */
    int *int_data = (int*)malloc(loop_size * sizeof(int));
    for (int i = 0; i < loop_size; i++) {
        int_data[i] = i + 1;
    }
    
    int int_sum = 0;
    
    /* Conditional SIMD with volatile control */
    if (simd_enabled && use_simd) {
        /* This should trigger SIMT transformation */
        #pragma omp simd simdlen(16) reduction(+:int_sum)
        for (int i = 0; i < loop_control && i < loop_size; i++) {
            int_sum += int_data[i];
        }
    } else {
        for (int i = 0; i < loop_size; i++) {
            int_sum += int_data[i];
        }
    }
    
    printf("Integer sum: %d\n", int_sum);
    
    /* Array 2: Float reduction in static function */
    float *float_data = (float*)malloc(loop_size * sizeof(float));
    for (int i = 0; i < loop_size; i++) {
        float_data[i] = (float)i * 0.5f;
    }
    
    float float_sum = static_simd_reduction(float_data, loop_size, use_simt_path);
    printf("Float sum: %f\n", float_sum);
    
    /* Array 3: Double reduction in noinline function */
    double *double_data = (double*)malloc(loop_size * sizeof(double));
    for (int i = 0; i < loop_size; i++) {
        double_data[i] = (double)i * 0.25;
    }
    
    /* Ternary operator for conditional execution */
    int skip_double = (use_simt_path == 0) ? 1 : 0;
    double double_product = noinline_simd_reduction(double_data, loop_size, skip_double);
    printf("Double product: %f\n", double_product);
    
    /* Mixed OpenMP construct to activate lowering infrastructure */
    #pragma omp parallel for simd
    for (int i = 0; i < loop_size; i++) {
        int_data[i] *= 2;
    }
    
    /* Additional SIMD with different parameters */
    long long long_sum = 0;
    #pragma omp simd simdlen(2) reduction(+:long_sum)
    for (int i = 0; i < loop_size; i += 2) {
        long_sum += (long long)int_data[i];
    }
    printf("Long long sum: %lld\n", long_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* Dummy external function implementation */
int get_random(void) {
    return rand();
}
