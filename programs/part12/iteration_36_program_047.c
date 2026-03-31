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
static float static_simd_reduction(float *data, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional execution to force gbind creation */
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

/* Noinline function to test DECL_CONTEXT setting */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int skip) {
    double result = 0.0;
    
    /* Use goto to interact with artificial labels */
    if (skip) {
        goto skip_simd;
    }
    
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(16) reduction(+:result) aligned(data:32)
    for (int i = 0; i < n; i++) {
        result += data[i] * data[i];
    }
    
skip_simd:
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int *array = (int*)malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Conditional SIMD reduction */
    int sum = 0;
    volatile int *ptr = &simd_enabled;
    
    /* Complex condition to prevent static elimination */
    if ((*ptr > 0) || (get_random() % 2)) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < size; i++) {
            sum += array[i];
        }
    }
    
    printf("Integer sum: %d\n", sum);
    free(array);
}

/* Main function with diverse SIMD constructs */
int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int loop_size = (argc > 1) ? atoi(argv[1]) : 1000;
    int use_simt = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* Set volatile control variables */
    simd_enabled = use_simt;
    use_simt_path = (get_random() % 100) > 50;
    
    /* Test 1: Integer SIMD reduction in main() */
    int int_data[1000];
    int int_sum = 0;
    
    /* Initialize with simple pattern */
    for (int i = 0; i < 1000; i++) {
        int_data[i] = i % 100;
    }
    
    /* Conditional SIMD with runtime-dependent condition */
    if (simd_enabled && (loop_size > 500 || use_simt_path)) {
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < 1000; i++) {
            int_sum += int_data[i];
        }
    } else {
        #pragma omp simd simdlen(2) reduction(+:int_sum)
        for (int i = 0; i < 1000; i++) {
            int_sum += int_data[i] * 2;
        }
    }
    
    printf("Test 1 - Integer reduction: %d\n", int_sum);
    
    /* Test 2: Float SIMD in static function */
    float float_data[2000];
    for (int i = 0; i < 2000; i++) {
        float_data[i] = (float)i / 10.0f;
    }
    
    float float_sum = static_simd_reduction(float_data, 2000, use_simt);
    printf("Test 2 - Float reduction: %f\n", float_sum);
    
    /* Test 3: Double SIMD in noinline function with goto */
    double double_data[500];
    for (int i = 0; i < 500; i++) {
        double_data[i] = (double)i / 3.0;
    }
    
    double double_sum = noinline_simd_reduction(double_data, 500, 
                                                (get_random() % 3) == 0);
    printf("Test 3 - Double reduction: %lf\n", double_sum);
    
    /* Test 4: Mixed OpenMP constructs */
    mixed_omp_constructs(loop_size);
    
    /* Test 5: Nested conditional SIMD with ternary-like structure */
    int final_result = 0;
    volatile int *control = &loop_control;
    
    /* Force complex control flow */
    switch (*control % 3) {
        case 0:
            #pragma omp simd simdlen(8) reduction(+:final_result)
            for (int i = 0; i < 500; i++) {
                final_result += i * 2;
            }
            break;
        case 1:
            if (use_simt_path) {
                #pragma omp simd simdlen(16) reduction(+:final_result)
                for (int i = 0; i < 500; i++) {
                    final_result += i * 3;
                }
            }
            break;
        default:
            /* Empty to test fallthrough */
            break;
    }
    
    printf("Test 5 - Final result: %d\n", final_result);
    
    return 0;
}

/* Dummy implementation of external function */
int get_random(void) {
    static int seed = 12345;
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}
