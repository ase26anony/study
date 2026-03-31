/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;

/* External function to force runtime evaluation */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *arr, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional wrapper around SIMD loop - forces gbind creation */
    if (flag > 0) {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += arr[i];
        }
    } else {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i += 2) {
            sum += arr[i] * 2.0f;
        }
    }
    
    return sum;
}

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_reduction(double *arr, int n, int skip) {
    double product = 1.0;
    int i;
    
    /* Goto to interact with artificial label creation */
    if (skip) {
        goto skip_simd;
    }
    
    /* SIMD loop with different data type */
    #pragma omp simd simdlen(2) reduction(*:product)
    for (i = 0; i < n; i++) {
        product *= (arr[i] + 1.0);
    }
    
skip_simd:
    /* Another SIMD loop that might be skipped */
    if (!skip) {
        #pragma omp simd simdlen(4) reduction(*:product)
        for (i = 0; i < n/2; i++) {
            product *= arr[i];
        }
    }
    
    return product;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int *data = (int*)malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        data[i] = i % 100;
    }
    
    /* Conditional SIMD reduction */
    int sum = 0;
    
    /* Ternary operator to force conditional wrapper */
    (use_simt_path) ? 
    (
        #pragma omp simd simdlen(16) reduction(+:sum)
        for (int i = 0; i < size; i += 2) {
            sum += data[i];
        }
    ) :
    (
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 1; i < size; i += 2) {
            sum += data[i];
        }
    );
    
    printf("Mixed constructs sum: %d\n", sum);
    free(data);
}

/* Main function with command-line control */
int main(int argc, char **argv) {
    int loop_count = 1000;
    int use_simt = 0;
    
    /* Parse command-line arguments for runtime control */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    if (argc > 2) {
        use_simt = atoi(argv[2]);
        use_simt_path = use_simt;  /* Set volatile */
    }
    
    /* Force runtime evaluation */
    simd_enabled = get_random() % 2;
    
    /* 1. Integer SIMD reduction in main with volatile condition */
    int int_sum = 0;
    int *int_arr = (int*)malloc(loop_count * sizeof(int));
    
    for (int i = 0; i < loop_count; i++) {
        int_arr[i] = i + 1;
    }
    
    /* Conditional execution based on volatile */
    if (simd_enabled) {
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i++) {
            int_sum += int_arr[i];
        }
    } else {
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i += 2) {
            int_sum += int_arr[i] * 2;
        }
    }
    
    printf("Integer sum: %d\n", int_sum);
    free(int_arr);
    
    /* 2. Float SIMD in static function */
    float *float_arr = (float*)malloc(loop_count * sizeof(float));
    for (int i = 0; i < loop_count; i++) {
        float_arr[i] = (float)i / 10.0f;
    }
    
    float float_sum = static_simd_reduction(float_arr, loop_count, use_simt);
    printf("Float sum: %f\n", float_sum);
    free(float_arr);
    
    /* 3. Double SIMD in noinline function with goto */
    double *double_arr = (double*)malloc(loop_count * sizeof(double));
    for (int i = 0; i < loop_count; i++) {
        double_arr[i] = (double)i / 5.0;
    }
    
    double double_prod = noinline_simd_reduction(double_arr, loop_count, use_simt);
    printf("Double product: %e\n", double_prod);
    free(double_arr);
    
    /* 4. Mixed OpenMP constructs */
    mixed_omp_constructs(loop_count / 2);
    
    /* Additional complex conditional SIMD */
    {
        int a = 0, b = 0;
        /* Nested conditions to force complex control flow */
        if (argc > 3) {
            #pragma omp simd simdlen(2) reduction(+:a)
            for (int i = 0; i < 100; i++) {
                a += i;
            }
        }
        
        switch (use_simt) {
            case 0:
                #pragma omp simd simdlen(4) reduction(+:b)
                for (int i = 0; i < 50; i++) {
                    b += i * 2;
                }
                break;
            case 1:
                #pragma omp simd simdlen(8) reduction(+:b)
                for (int i = 0; i < 50; i++) {
                    b += i * 3;
                }
                break;
            default:
                #pragma omp simd simdlen(16) reduction(+:b)
                for (int i = 0; i < 50; i++) {
                    b += i * 4;
                }
        }
        
        printf("Final a=%d, b=%d\n", a, b);
    }
    
    return 0;
}

/* Dummy implementation of external function */
int get_random(void) {
    return rand();
}
