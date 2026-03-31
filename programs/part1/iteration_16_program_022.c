#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3"), noinline))
static float hot_loop_scheduler(float *data, int size) {
    volatile float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    for (int i = 0; i < size; i++) {
        /* RAW hazard: read after write */
        float temp = data[i] * 2.0f;
        
        /* WAR hazard: write after read */
        data[i] = temp + 1.0f;
        
        /* WAW hazard: write after write */
        float temp2 = data[i] * 3.0f;
        data[i] = temp2;
        
        /* Complex expression with mixed operations */
        sum += data[i] * sinf((float)i) * cosf((float)i);
    }
    
    /* Memory barrier forcing scheduler decisions */
    asm volatile("" ::: "memory");
    
    return sum;
}

__attribute__((cold, optimize("sched-pressure"), noinline))
static int cold_region_scheduler(int *arr, int n) {
    int result = 0;
    
    /* Nested loops with pointer chasing */
    for (int i = 0; i < n; i++) {
        int *ptr = &arr[i];
        
        /* Pointer chasing with dependencies */
        for (int j = 0; j < 8; j++) {
            /* Load/store sequence */
            int val = *ptr;
            
            /* Assembly with register clobber */
            asm volatile("" : "+r"(val) : : "eax", "memory");
            
            *ptr = val + j;
            ptr = &arr[(i + j) % n];
        }
        
        result += arr[i];
    }
    
    return result;
}

__attribute__((optimize("O3")))
static void vectorized_unrolled_loop(float *a, float *b, float *c, int size) {
    int i;
    
    /* SIMD-friendly loop with unroll pragma */
    #pragma GCC unroll 4
    for (i = 0; i < size; i++) {
        /* Operations that encourage vectorization */
        a[i] = b[i] * c[i] + sinf(b[i]) * cosf(c[i]);
        
        /* Conditional operation */
        a[i] = (a[i] > 0.0f) ? a[i] * 2.0f : a[i] * 0.5f;
    }
    
    /* Another unrolled section */
    #pragma GCC unroll 2
    for (i = 0; i < size - 1; i++) {
        /* Cross-iteration dependency */
        b[i] = a[i] + a[i + 1];
    }
}

__attribute__((noinline, optimize("O3")))
static int complex_control_flow(int x) {
    int result = 0;
    
    /* Switch with sparse cases */
    switch (x % 7) {
        case 0:
            result = x * 2;
            /* Fall through */
        case 1:
            result += x / 3;
            break;
        case 2:
            /* Conditional move */
            result = (x > 100) ? x - 50 : x + 50;
            break;
        case 3:
            for (int i = 0; i < 10; i++) {
                if (i == x % 5) continue;
                result += i * x;
                if (result > 1000) break;
            }
            break;
        case 4:
            result = x | (x << 8);
            break;
        case 5:
            /* Nested conditionals */
            if (x < 0) {
                result = -x;
            } else if (x < 50) {
                result = x * x;
            } else {
                result = x / 2;
            }
            break;
        default:
            result = ~x;
    }
    
    /* Assembly barrier splitting scheduling regions */
    asm volatile("" ::: "memory", "eax", "ebx");
    
    return result;
}

__attribute__((optimize("O3")))
static void mixed_precision_ops(double *dbl_arr, float *flt_arr, int size) {
    /* Mixed precision operations */
    for (int i = 0; i < size; i++) {
        /* Convert between precisions */
        float f = (float)dbl_arr[i];
        
        /* Mixed operations */
        dbl_arr[i] = dbl_arr[i] * 1.5 + (double)f * 0.25;
        
        /* More complex expression */
        flt_arr[i] = f * 2.0f + sinf(f) * 0.5f;
        
        /* Dependency chain */
        if (i > 0) {
            flt_arr[i] += flt_arr[i - 1] * 0.1f;
        }
    }
}

__attribute__((optimize("O3"), noinline))
static int scheduling_test_1(void) {
    static float fdata[ARRAY_SIZE];
    static int idata[ARRAY_SIZE];
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fdata[i] = (float)i * 0.1f;
        idata[i] = i;
    }
    
    float fsum = 0.0f;
    int isum = 0;
    
    /* Call various scheduling-intensive functions */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        fsum += hot_loop_scheduler(fdata, ARRAY_SIZE);
        isum += cold_region_scheduler(idata, ARRAY_SIZE);
        
        /* Modify data to prevent optimization */
        fdata[iter % ARRAY_SIZE] += 0.01f;
        idata[iter % ARRAY_SIZE] += 1;
    }
    
    return (int)fsum + isum;
}

__attribute__((optimize("O3"), noinline))
static int scheduling_test_2(void) {
    static float array_a[ARRAY_SIZE];
    static float array_b[ARRAY_SIZE];
    static float array_c[ARRAY_SIZE];
    static double dbl_arr[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (float)i * 0.25f;
        array_b[i] = (float)i * 0.33f;
        array_c[i] = (float)i * 0.5f;
        dbl_arr[i] = (double)i * 0.1;
    }
    
    int result = 0;
    
    /* Multiple vectorized/unrolled operations */
    for (int iter = 0; iter < ITERATIONS / 2; iter++) {
        vectorized_unrolled_loop(array_a, array_b, array_c, ARRAY_SIZE);
        
        /* Complex control flow */
        result += complex_control_flow(iter);
        
        /* Mixed precision operations */
        mixed_precision_ops(dbl_arr, array_a, ARRAY_SIZE);
        
        /* Prevent dead code elimination */
        array_b[iter % ARRAY_SIZE] += 0.001f;
    }
    
    /* Final computation using results */
    float final_sum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array_a[i] + array_b[i] + (float)dbl_arr[i];
    }
    
    return result + (int)final_sum;
}

__attribute__((optimize("O3")))
static void pointer_chasing_pattern(int *data, int size) {
    /* Complex pointer chasing with dependencies */
    int *ptr1 = data;
    int *ptr2 = data + size / 2;
    
    for (int i = 0; i < size / 2; i++) {
        /* Read from two pointers */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Assembly with constraints */
        asm volatile("addl %%ebx, %%eax\n\t"
                     : "=a"(val1)
                     : "a"(val1), "b"(val2)
                     : "cc");
        
        /* Write back with offset */
        *ptr1 = val1 + i;
        *ptr2 = val2 - i;
        
        /* Update pointers with wrapping */
        ptr1 = &data[(i * 3) % size];
        ptr2 = &data[(i * 7) % size];
        
        /* Memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

int main(void) {
    int total_result = 0;
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Allocate dynamic memory for additional scheduling complexity */
    int *dynamic_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *dynamic_floats = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!dynamic_data || !dynamic_floats) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize dynamic arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        dynamic_data[i] = i * 2;
        dynamic_floats[i] = (float)i * 0.333f;
    }
    
    /* Run scheduling tests */
    total_result += scheduling_test_1();
    total_result += scheduling_test_2();
    
    /* Additional pointer chasing test */
    pointer_chasing_pattern(dynamic_data, ARRAY_SIZE);
    
    /* Use results to prevent optimization */
    int dynamic_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        dynamic_sum += dynamic_data[i];
    }
    total_result += dynamic_sum;
    
    /* Clean up */
    free(dynamic_data);
    free(dynamic_floats);
    
    printf("Test completed. Result checksum: %d\n", total_result);
    printf("(This value varies based on architecture and optimization)\n");
    
    return 0;
}
