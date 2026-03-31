/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc, lines 2941-2975. This creates conditional wrappers with
   artificial labels and bind expressions for SIMD loops targeting GPU-like SIMT execution. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Volatile variables to prevent optimization and control flow */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to prevent inlining */
extern int parse_arg(const char *arg);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional wrapper around SIMD loop - forces gbind creation */
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
    double product = 1.0;
    
    /* Goto statements to interact with artificial label generation */
    if (skip) goto skip_simd;
    
    /* SIMD loop with different data type */
    #pragma omp simd simdlen(4) reduction(*:product)
    for (int i = 0; i < n; i++) {
        product *= (data[i] + 1.0);
    }
    
skip_simd:
    /* Another SIMD loop that might be skipped */
    if (!skip) {
        #pragma omp simd simdlen(8) reduction(*:product)
        for (int i = 0; i < n; i += 2) {
            product *= data[i];
        }
    }
    
    return product;
}

/* Function with ternary operator controlling SIMD execution */
int conditional_simd_sum(int *arr, int size, int threshold) {
    int total = 0;
    
    /* Ternary operator creates conditional execution path */
    (threshold > 0) ? 
    (
        #pragma omp simd simdlen(16) reduction(+:total)
        for (int i = 0; i < size; i++) {
            total += arr[i];
        }
    ) :
    (
        #pragma omp simd simdlen(8) reduction(+:total)
        for (int i = 0; i < size; i++) {
            total -= arr[i];
        }
    );
    
    return total;
}

/* Mixed OpenMP constructs to activate full OMP lowering */
void parallel_region_test(int n) {
    int *parallel_data = (int *)malloc(n * sizeof(int));
    
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < n; i++) {
        parallel_data[i] = i * omp_get_thread_num();
    }
    
    /* Nested conditional SIMD */
    if (simd_enabled) {
        #pragma omp simd simdlen(8)
        for (int i = 0; i < n; i++) {
            parallel_data[i] += 1;
        }
    }
    
    free(parallel_data);
}

int main(int argc, char *argv[]) {
    /* Parse command-line arguments for runtime control */
    int loop_size = 1000;
    int use_simt = 0;
    int skip_double = 0;
    
    if (argc > 1) loop_size = atoi(argv[1]);
    if (argc > 2) use_simt = atoi(argv[2]);
    if (argc > 3) skip_double = atoi(argv[3]);
    
    /* Set volatile control variables */
    use_simt_path = use_simt;
    loop_control = loop_size;
    
    /* Test 1: Integer SIMD reduction in main with conditional */
    int *int_data = (int *)malloc(loop_size * sizeof(int));
    for (int i = 0; i < loop_size; i++) {
        int_data[i] = i + 1;
    }
    
    int int_sum = 0;
    /* Complex condition to prevent static elimination */
    if ((use_simt_path || simd_enabled) && (loop_control > 100)) {
        int_sum = conditional_simd_sum(int_data, loop_size, use_simt);
    }
    printf("Integer sum: %d\n", int_sum);
    
    /* Test 2: Float SIMD in static function */
    float *float_data = (float *)malloc(loop_size * sizeof(float));
    for (int i = 0; i < loop_size; i++) {
        float_data[i] = (float)i / 10.0f;
    }
    
    float float_sum = static_simd_reduction(float_data, loop_size, use_simt);
    printf("Float sum: %f\n", float_sum);
    
    /* Test 3: Double SIMD in noinline function with goto */
    double *double_data = (double *)malloc(loop_size * sizeof(double));
    for (int i = 0; i < loop_size; i++) {
        double_data[i] = (double)i / 5.0;
    }
    
    double double_prod = noinline_simd_reduction(double_data, loop_size, skip_double);
    printf("Double product: %f\n", double_prod);
    
    /* Test 4: Mixed OpenMP constructs */
    parallel_region_test(loop_size / 2);
    
    /* Additional complex conditional with nested SIMD */
    for (int outer = 0; outer < 3; outer++) {
        /* Volatile-dependent condition */
        if (simd_enabled ^ (outer % 2)) {
            #pragma omp simd simdlen(4)
            for (int i = 0; i < loop_size; i++) {
                int_data[i] += outer;
            }
        } else {
            #pragma omp simd simdlen(8)
            for (int i = 0; i < loop_size; i++) {
                int_data[i] -= outer;
            }
        }
    }
    
    /* Final reduction with SIMD */
    int final_sum = 0;
    #pragma omp simd simdlen(16) reduction(+:final_sum)
    for (int i = 0; i < loop_size; i++) {
        final_sum += int_data[i];
    }
    printf("Final sum: %d\n", final_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* External function definition */
int parse_arg(const char *arg) {
    return atoi(arg);
}
