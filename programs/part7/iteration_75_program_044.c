/* haifa_sched_coverage.c
 * Program to trigger GCC's HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa_sched_coverage.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile double g_volatile_double = 1.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that create scheduling boundaries */
static int helper_mul_chain(int a, int b) {
    volatile int barrier;
    int t1 = a * 3;
    int t2 = b * 5;
    barrier = t1 + t2;
    int t3 = t1 * t2;
    int t4 = t3 * 7;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return t4 ^ barrier;
}

static float helper_float_ops(float x, float y) {
    float t1 = x * 0.5f;
    float t2 = y * 1.5f;
    float t3 = t1 + t2;
    float t4 = t3 * t3;
    volatile float v = t4;  /* Volatile store */
    return v - t1;
}

static double helper_double_chain(double a, double b) {
    double sum = a;
    for (int i = 0; i < 4; i++) {
        sum = sum * 1.1 + b;
        b = b * 0.9;
    }
    return sum;
}

/* Non-inlineable function (due to complexity) */
__attribute__((noinline)) 
int complex_branching(int x, int *arr, int n) {
    int result = x;
    if (x & 1) {
        for (int i = 0; i < n; i += 2) {
            result += arr[i] * arr[i + 1];
            arr[i] = result ^ 0x55AA55AA;
        }
    } else {
        for (int i = n - 1; i >= 0; i--) {
            result -= arr[i] * i;
            arr[i] = result & 0x00FF00FF;
        }
    }
    return result;
}

/* Main computation with dense instruction mix */
static uint64_t compute_kernel(int iterations, int array_size) {
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)aligned_alloc(64, array_size * sizeof(int));
    float *float_array = (float*)malloc(array_size * sizeof(float));
    double *double_array = (double*)malloc(array_size * sizeof(double));
    struct PackedData *packed_array = (struct PackedData*)malloc(array_size * sizeof(struct PackedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        float_array[i] = (float)(i * 0.123456789);
        double_array[i] = (double)(i * 0.987654321);
        packed_array[i].c = (char)(i & 0xFF);
        packed_array[i].i = i * 3;
        packed_array[i].d = (double)i * 2.5;
        packed_array[i].s = (short)(i * 7);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {
        helper_mul_chain,
        [](int a, int b) { return a + b * 2; },
        [](int a, int b) { return (a << 3) | (b & 0xF); },
        [](int a, int b) { return a * a - b * b; },
        [](int a, int b) { return (a ^ b) * 13; }
    };
    
    uint64_t accumulator = 0;
    int ptr_chase_index = 0;
    
    /* Main computation loop with complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* 1. Pointer chasing through array (simulated linked list) */
        int chase_sum = 0;
        for (int j = 0; j < 100; j++) {
            ptr_chase_index = int_array[ptr_chase_index] % array_size;
            chase_sum += ptr_chase_index;
            /* Memory barrier to prevent reordering */
            asm volatile("" ::: "memory");
        }
        accumulator ^= chase_sum;
        
        /* 2. Dense arithmetic chain with mixed types */
        double d1 = double_array[iter % array_size];
        float f1 = float_array[(iter + 1) % array_size];
        int i1 = int_array[(iter + 2) % array_size];
        
        for (int k = 0; k < 20; k++) {
            d1 = d1 * 1.01 + sin((double)k * 0.1);
            f1 = f1 * 1.5f - cosf((float)k * 0.2f);
            i1 = i1 * 3 + k * 7;
            
            /* Volatile access to force scheduler work */
            g_volatile_double = d1;
            g_volatile_counter = i1;
        }
        
        /* 3. Large basic block with independent operations */
        int temp_results[32];
        for (int k = 0; k < 32; k++) {
            /* Independent computations to fill instruction queue */
            temp_results[k] = (int_array[k] * k) + (int)(float_array[k] * 100.0f);
            double_array[k] = double_array[k] * 0.99 + k * 0.01;
            packed_array[k].i = packed_array[k].i * 2 + k;
            packed_array[k].d = packed_array[k].d * 0.95;
        }
        
        /* 4. Switch statement with many cases (10 branches) */
        switch (iter % 10) {
            case 0: {
                int t = helper_mul_chain(iter, iter + 1);
                accumulator += t * 3;
                break;
            }
            case 1: {
                float f = helper_float_ops((float)iter, (float)(iter * 2));
                accumulator += (int)(f * 1000.0f);
                break;
            }
            case 2: {
                double d = helper_double_chain((double)iter, (double)(iter * 3));
                accumulator ^= (uint64_t)d;
                break;
            }
            case 3: {
                /* Nested loop with loop-carried dependency */
                int sum = 0;
                for (int k = 1; k < 50; k++) {
                    sum += int_array[k] * int_array[k-1];
                    int_array[k-1] = sum & 0xFFF;
                }
                accumulator += sum;
                break;
            }
            case 4: {
                /* Function pointer call */
                int r = funcs[iter % 5](iter, iter * 2);
                accumulator += r;
                break;
            }
            case 5: {
                /* Conditional with volatile */
                if (g_volatile_counter & 1) {
                    accumulator <<= 3;
                } else {
                    accumulator >>= 2;
                }
                break;
            }
            case 6: {
                /* Mixed integer/float operations */
                int i_val = int_array[iter % array_size];
                float f_val = float_array[iter % array_size];
                double d_val = (double)i_val * f_val;
                accumulator += (uint64_t)d_val;
                break;
            }
            case 7: {
                /* Memory-intensive operations */
                memmove(&int_array[10], &int_array[0], 100 * sizeof(int));
                break;
            }
            case 8: {
                /* Call non-inlineable function */
                int r = complex_branching(iter, int_array, array_size / 2);
                accumulator ^= r;
                break;
            }
            case 9: {
                /* Deep conditional chain */
                int x = iter;
                if (x < 100) accumulator += 1;
                else if (x < 200) accumulator += 2;
                else if (x < 300) accumulator += 3;
                else if (x < 400) accumulator += 4;
                else if (x < 500) accumulator += 5;
                else if (x < 600) accumulator += 6;
                else if (x < 700) accumulator += 7;
                else if (x < 800) accumulator += 8;
                else if (x < 900) accumulator += 9;
                else accumulator += 10;
                break;
            }
        }
        
        /* 5. Conditional with helper call */
        if (iter & 0x1) {
            float f = helper_float_ops((float)accumulator, (float)(iter));
            accumulator += (uint64_t)(f * 100.0f);
        }
        
        /* 6. Reduction across arrays */
        if ((iter % 100) == 0) {
            int sum = 0;
            for (int k = 0; k < array_size; k += 4) {
                sum += int_array[k] + int_array[k+1] + 
                       int_array[k+2] + int_array[k+3];
            }
            accumulator ^= sum;
        }
    }
    
    /* Final reduction across all modified data */
    uint64_t final_result = accumulator;
    for (int i = 0; i < array_size; i++) {
        final_result ^= int_array[i];
        final_result += (uint64_t)(float_array[i] * 100.0f);
        final_result ^= (uint64_t)double_array[i];
        final_result += packed_array[i].i;
    }
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(packed_array);
    
    return final_result;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    int array_size = 1024;
    
    /* Parse command line arguments */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 1024;
    }
    
    printf("Running HAIFA scheduler test with %d iterations, array size %d\n", 
           iterations, array_size);
    
    /* Run computation kernel */
    uint64_t result = compute_kernel(iterations, array_size);
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016llX\n", (unsigned long long)result);
    
    /* Additional test with different optimization patterns */
    if (result & 1) {
        /* Run again with different parameters to explore more scheduler states */
        result ^= compute_kernel(iterations / 2, array_size * 2);
        printf("Final result: 0x%016llX\n", (unsigned long long)result);
    }
    
    return 0;
}
