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

/* External function to force control flow */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional wrapper - forces compiler to generate bind/label structure */
    if (enable) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i] * data[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += sqrtf(data[i]);
        }
    }
    
    return sum;
}

/* Noinline function with goto-based control flow */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int flag) {
    double product = 1.0;
    
    /* Complex control flow with goto to interact with artificial labels */
    if (flag > 0) {
        goto compute;
    } else {
        goto skip;
    }
    
compute:
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(16) reduction(*:product) aligned(data:32)
    for (int i = 0; i < n; i++) {
        product *= (data[i] + 1.0);
    }
    goto finish;
    
skip:
    /* Alternative SIMD loop */
    #pragma omp simd simdlen(8) reduction(*:product)
    for (int i = 0; i < n; i++) {
        product *= (1.0 - data[i]);
    }
    
finish:
    return product;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int *array = (int*)malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < size; i++) {
        array[i] = i * 2;
    }
    
    /* Conditional SIMD reduction */
    int sum = 0;
    
    /* Ternary operator to force conditional wrapper generation */
    (use_simt_path) ? 
    (
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < size; i += 2) {
            sum += array[i];
        }
    ) : (
        #pragma omp simd simdlen(16) reduction(+:sum)
        for (int i = 1; i < size; i += 2) {
            sum += array[i];
        }
    );
    
    printf("Mixed constructs sum: %d\n", sum);
    free(array);
}

/* Main function with diverse SIMD constructs */
int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int loop_size = 1000;
    int use_simt = 0;
    
    if (argc > 1) {
        loop_size = atoi(argv[1]);
        if (argc > 2) {
            use_simt = atoi(argv[2]);
        }
    }
    
    /* Force volatile updates */
    simd_enabled = (loop_size > 0);
    use_simt_path = use_simt;
    loop_control = loop_size;
    
    /* 1. Integer SIMD reduction in main() */
    int *int_data = (int*)malloc(loop_size * sizeof(int));
    int int_sum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < loop_size; i++) {
        int_data[i] = i % 100;
    }
    
    /* Conditional SIMD loop - triggers SIMT transformation */
    if (simd_enabled && (loop_size % 2 == 0)) {
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_control; i++) {
            int_sum += int_data[i] * 3;
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < loop_control; i++) {
            int_sum += int_data[i] * 2;
        }
    }
    
    printf("Integer reduction result: %d\n", int_sum);
    
    /* 2. Float SIMD in static function */
    float *float_data = (float*)malloc(loop_size * sizeof(float));
    for (int i = 0; i < loop_size; i++) {
        float_data[i] = (float)i / 100.0f;
    }
    
    float float_result = static_simd_reduction(float_data, loop_size, use_simt);
    printf("Float reduction result: %f\n", float_result);
    
    /* 3. Double SIMD in noinline function with goto */
    double *double_data = (double*)malloc(loop_size * sizeof(double));
    for (int i = 0; i < loop_size; i++) {
        double_data[i] = sin((double)i / 50.0);
    }
    
    double double_result = noinline_simd_reduction(double_data, loop_size, use_simt);
    printf("Double reduction result: %f\n", double_result);
    
    /* 4. Mixed OpenMP constructs */
    mixed_omp_constructs(loop_size / 2);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
