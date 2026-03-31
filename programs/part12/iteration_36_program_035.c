/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to force runtime evaluation */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int condition) 
{
    float sum = 0.0f;
    
    /* Conditional wrapper to force SIMT transformation */
    if (condition) {
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
double noinline_simd_reduction(double *data, int n, int skip_first) 
{
    double product = 1.0;
    int i = 0;
    
    /* Goto statements to interact with artificial label creation */
    if (skip_first) {
        goto skip_loop;
    }
    
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(4) reduction(*:product) linear(i:1)
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
void mixed_omp_constructs(int *results, int size) 
{
    /* Parallel region to activate OMP infrastructure */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < size; i++) {
        results[i] = i * i;
    }
    
    /* Conditional SIMD with runtime-dependent bounds */
    int limit = loop_control % size;
    if (limit > 0) {
        #pragma omp simd simdlen(8)
        for (int i = 0; i < limit; i++) {
            results[i] += get_random() % 100;
        }
    }
}

/* Main test driver */
int main(int argc, char **argv) 
{
    /* Parse command line for runtime control */
    int array_size = 1024;
    if (argc > 1) {
        array_size = atoi(argv[1]);
        if (array_size <= 0) array_size = 1024;
    }
    
    /* Allocate and initialize arrays with different types */
    int *int_data = (int*)malloc(array_size * sizeof(int));
    float *float_data = (float*)malloc(array_size * sizeof(float));
    double *double_data = (double*)malloc(array_size * sizeof(double));
    
    for (int i = 0; i < array_size; i++) {
        int_data[i] = i + 1;
        float_data[i] = (float)(i + 1) * 0.5f;
        double_data[i] = (double)(i + 1) * 0.25;
    }
    
    /* Test 1: Integer SIMD reduction in main with volatile condition */
    int int_sum = 0;
    volatile int simd_flag = simd_enabled;
    
    /* This conditional wrapper should trigger the uncovered SIMT code */
    if (simd_flag || use_simt_path) {
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < array_size; i++) {
            int_sum += int_data[i];
        }
    } else {
        /* Alternative path to ensure both branches exist */
        for (int i = 0; i < array_size; i++) {
            int_sum += int_data[i] * 2;
        }
    }
    
    /* Test 2: Float SIMD in static function with ternary operator */
    int condition = (argc > 2) ? atoi(argv[2]) : 1;
    float float_sum = static_simd_reduction(float_data, array_size, condition);
    
    /* Test 3: Double SIMD in noinline function with goto */
    int skip = (argc > 3) ? atoi(argv[3]) : 0;
    double double_prod = noinline_simd_reduction(double_data, array_size, skip);
    
    /* Test 4: Mixed constructs */
    int *results = (int*)malloc(array_size * sizeof(int));
    mixed_omp_constructs(results, array_size);
    
    /* Print results to prevent elimination */
    printf("Integer sum: %d\n", int_sum);
    printf("Float sum: %f\n", float_sum);
    printf("Double product: %lf\n", double_prod);
    
    /* Verify some results */
    int check_sum = 0;
    for (int i = 0; i < array_size; i++) {
        check_sum += results[i];
    }
    printf("Results checksum: %d\n", check_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    free(results);
    
    return 0;
}

/* Dummy external function implementation */
int get_random(void) {
    static int seed = 12345;
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}
