/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc lines 2941-2975 */
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
    
    /* Conditional wrapper to force SIMT transformation */
    if (flag > 0) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum -= data[i];
        }
    }
    
    return sum;
}

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_compute(double *arr, int size, int skip) {
    double result = 0.0;
    int i;
    
    /* Goto to interact with artificial label creation */
    if (skip) {
        goto skip_simd;
    }
    
    /* SIMD loop with multiple data types in expressions */
    #pragma omp simd simdlen(16) reduction(+:result) linear(i:1)
    for (i = 0; i < size; i++) {
        result += arr[i] * (i % 2 ? 1.5 : 2.5);
    }
    
skip_simd:
    /* Another SIMD loop that might be skipped */
    if (!skip) {
        #pragma omp simd simdlen(8) reduction(+:result)
        for (int j = 0; j < size/2; j++) {
            result += arr[j] * 0.5;
        }
    }
    
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int *output, int n) {
    int i;
    
    /* Regular parallel for */
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        output[i] = i * 2;
    }
    
    /* Conditional SIMD with volatile control */
    int local_flag = simd_enabled;
    
    /* This conditional wrapper should trigger the uncovered code */
    if (local_flag) {
        #pragma omp simd simdlen(4)
        for (i = 0; i < n; i++) {
            output[i] += (i % 3);
        }
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    const int N = 1024;
    int *int_data = (int*)malloc(N * sizeof(int));
    float *float_data = (float*)malloc(N * sizeof(float));
    double *double_data = (double*)malloc(N * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        int_data[i] = i + 1;
        float_data[i] = (i + 1) * 0.5f;
        double_data[i] = (i + 1) * 0.25;
    }
    
    /* Parse command line for runtime control */
    int use_simt = 0;
    int loop_count = N;
    
    if (argc > 1) {
        use_simt = atoi(argv[1]);
        use_simt_path = use_simt;  /* Set volatile */
    }
    if (argc > 2) {
        loop_count = atoi(argv[2]);
        loop_control = loop_count;  /* Set volatile */
    }
    
    /* Test 1: SIMD in main with conditional execution */
    int sum_int = 0;
    
    /* Complex condition to prevent static elimination */
    int condition = (use_simt > 0) ? 1 : (get_random() % 2);
    
    if (condition) {
        /* This should trigger the SIMT transformation */
        #pragma omp simd simdlen(8) reduction(+:sum_int)
        for (int i = 0; i < loop_count; i++) {
            sum_int += int_data[i];
        }
    } else {
        /* Alternative path */
        #pragma omp simd simdlen(4) reduction(+:sum_int)
        for (int i = 0; i < loop_count/2; i++) {
            sum_int -= int_data[i];
        }
    }
    
    printf("Integer sum: %d\n", sum_int);
    
    /* Test 2: Static function with SIMD */
    float sum_float = static_simd_reduction(float_data, loop_count, use_simt);
    printf("Float sum: %f\n", sum_float);
    
    /* Test 3: Noinline function with goto */
    double sum_double = noinline_simd_compute(double_data, loop_count, use_simt == 0);
    printf("Double sum: %lf\n", sum_double);
    
    /* Test 4: Mixed constructs */
    mixed_omp_constructs(int_data, loop_count < N ? loop_count : N/2);
    
    /* Verify results aren't optimized away */
    volatile int check = int_data[0] + (int)sum_float + (int)sum_double;
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* Dummy external function implementation */
int get_random(void) {
    return rand();
}
