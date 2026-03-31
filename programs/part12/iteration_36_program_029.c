/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc, lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to force control flow */
extern int get_external_value(void);

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

/* Noinline function with goto-based control flow */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int skip_first) {
    double product = 1.0;
    int i = 0;
    
    /* Goto to interact with artificial label creation */
    if (skip_first) {
        goto skip_loop;
    }
    
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(4) reduction(*:product) aligned(data:32)
    for (i = 0; i < n; i++) {
        product *= data[i];
    }
    
skip_loop:
    /* Another SIMD loop that might be conditionally executed */
    if (!skip_first || use_simt_path) {
        #pragma omp simd simdlen(8) reduction(*:product)
        for (int j = 0; j < n/2; j++) {
            product *= data[j] * 0.5;
        }
    }
    
    return product;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int *results, int size) {
    /* Regular parallel for */
    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        results[i] = i * i;
    }
    
    /* Conditional SIMD inside parallel region */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        if (tid % 2 == 0) {
            #pragma omp simd simdlen(4)
            for (int i = 0; i < size; i++) {
                results[i] += tid;
            }
        }
    }
}

/* Main test function */
int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int loop_size = (argc > 1) ? atoi(argv[1]) : 1000;
    int use_simt = (argc > 2) ? atoi(argv[2]) : 0;
    int skip_double = (argc > 3) ? atoi(argv[3]) : 0;
    
    /* Set volatile control variables */
    use_simt_path = use_simt;
    loop_control = loop_size;
    
    /* Test 1: Integer SIMD reduction in main() with conditional */
    int *int_data = (int*)malloc(loop_size * sizeof(int));
    int int_sum = 0;
    
    for (int i = 0; i < loop_size; i++) {
        int_data[i] = i + 1;
    }
    
    /* Force conditional wrapper generation */
    if (simd_enabled && use_simt_path) {
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_control; i++) {
            int_sum += int_data[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < loop_control; i++) {
            int_sum += int_data[i] * 2;
        }
    }
    
    printf("Integer sum: %d\n", int_sum);
    
    /* Test 2: Float SIMD in static function */
    float *float_data = (float*)malloc(loop_size * sizeof(float));
    for (int i = 0; i < loop_size; i++) {
        float_data[i] = (float)i / 10.0f;
    }
    
    float float_result = static_simd_reduction(float_data, loop_size, use_simt);
    printf("Float reduction: %f\n", float_result);
    
    /* Test 3: Double SIMD in noinline function with goto */
    double *double_data = (double*)malloc(loop_size * sizeof(double));
    for (int i = 0; i < loop_size; i++) {
        double_data[i] = 1.0 + (double)i / 100.0;
    }
    
    double double_result = noinline_simd_reduction(double_data, loop_size, skip_double);
    printf("Double product: %e\n", double_result);
    
    /* Test 4: Mixed OpenMP constructs */
    int *results = (int*)malloc(loop_size * sizeof(int));
    mixed_omp_constructs(results, loop_size);
    
    /* Verify some results to prevent elimination */
    int check_sum = 0;
    #pragma omp simd simdlen(4) reduction(+:check_sum)
    for (int i = 0; i < loop_size; i++) {
        check_sum += results[i];
    }
    printf("Check sum: %d\n", check_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    free(results);
    
    return 0;
}
