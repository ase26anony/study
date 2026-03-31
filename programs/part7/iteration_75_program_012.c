/* haifa_sched_trigger.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa_sched_trigger.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 32

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile float g_volatile_float = 1.0f;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    double d;
    char c2;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that create scheduling boundaries */
static int helper_mul_chain(int a, int b) {
    asm volatile("" ::: "memory");  /* Compiler barrier */
    int t1 = a * b;
    int t2 = t1 * (b + 1);
    int t3 = t2 * (a - 1);
    g_volatile_counter++;
    return t3 & 0xFF;
}

static float helper_float_ops(float x, float y) {
    volatile float v = x;  /* Volatile access */
    float r1 = v * y;
    float r2 = r1 + v;
    float r3 = r2 / (y + 1.0f);
    g_volatile_float = r3;
    return r3;
}

static double helper_double_chain(double *arr, int idx) {
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += arr[(idx + i) % ARRAY_SIZE] * (i + 1);
        asm volatile("" ::: "memory");  /* Barrier between iterations */
    }
    return sum;
}

/* Complex computation with many dependencies */
static int complex_dependency_chain(int seed) {
    int a = seed * 1103515245 + 12345;
    int b = (a >> 16) & 0x7FFF;
    int c = b * a;
    int d = c + (a % 17);
    int e = d ^ (b << 3);
    int f = e * 3;
    int g = f / (seed + 1);
    int h = g | (d & 0xF0F0F0F0);
    
    /* Create loop-carried dependency */
    static int carry = 0;
    h += carry;
    carry = h % 256;
    
    return h;
}

/* Pointer chasing through array */
static int pointer_chase(int *array, int start_idx, int steps) {
    int idx = start_idx;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        sum += array[idx];
        idx = array[idx] % ARRAY_SIZE;  /* Next index depends on current value */
        asm volatile("" ::: "memory");  /* Prevent reordering */
    }
    
    return sum;
}

/* Switch-based computation with different kernels */
static int switch_kernel(int value, int case_num) {
    int result = value;
    
    switch (case_num % 10) {
        case 0:
            result = result * 3 + 7;
            result = result ^ 0x55AA55AA;
            result = result >> (value & 0x7);
            break;
        case 1:
            result = helper_mul_chain(result, value);
            result = result + (value % 19);
            break;
        case 2:
            result = (result << 4) | (result >> 28);
            result = result & 0x0F0F0F0F;
            break;
        case 3:
            for (int i = 0; i < 8; i++) {
                result = (result * 13 + i) % 0x7FFF;
            }
            break;
        case 4:
            result = result ^ (result << 16);
            result = result ^ (result >> 16);
            result = result ^ (result << 8);
            break;
        case 5:
            result = complex_dependency_chain(result);
            break;
        case 6:
            result = (result & 0x55555555) << 1 | (result & 0xAAAAAAAA) >> 1;
            result = result * 0x9E3779B9;
            break;
        case 7:
            result = result + (result / 3) - (result % 5);
            break;
        case 8:
            result = ~result;
            result = result * 0xCCCCCCCD;
            break;
        case 9:
            result = (result | 0x80000000) & 0x7FFFFFFF;
            result = result * 0x24924925;
            break;
    }
    
    return result;
}

/* Main computation with mixed operations */
static uint64_t compute_kernel(int iterations, int *int_array, 
                               float *float_array, double *double_array,
                               struct misaligned_data *struct_array) {
    uint64_t total = 0;
    int chase_idx = 0;
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {
        helper_mul_chain,
        (compute_func_t)helper_float_ops,
        NULL
    };
    
    for (int i = 0; i < iterations; i++) {
        int base_val = i * 1103515245;
        
        /* 1. Pointer chasing with dependencies */
        chase_idx = pointer_chase(int_array, chase_idx, 8);
        
        /* 2. Mixed arithmetic chain with dependencies */
        int a = int_array[i % ARRAY_SIZE];
        int b = int_array[(i + 1) % ARRAY_SIZE];
        int c = a * b + chase_idx;
        int d = c / (a + 1);
        int e = d ^ b;
        int f = e << (i & 0x3);
        int g = f % 997;
        
        /* 3. Floating-point operations */
        float fa = float_array[i % ARRAY_SIZE];
        float fb = float_array[(i + 2) % ARRAY_SIZE];
        float fc = fa * fb + (float)g;
        float fd = fc / (fb + 1.0f);
        float fe = sinf(fd) * cosf(fa);
        
        /* 4. Double precision chain */
        double da = double_array[i % ARRAY_SIZE];
        double db = double_array[(i + 3) % ARRAY_SIZE];
        double dc = helper_double_chain(double_array, i);
        double dd = da * db + dc;
        double de = sqrt(fabs(dd)) + log1p(fabs(da));
        
        /* 5. Packed struct access (misaligned) */
        struct_array[i % MAX_DEPTH].c = (char)(g & 0xFF);
        struct_array[i % MAX_DEPTH].i = e;
        struct_array[i % MAX_DEPTH].d = dd;
        
        /* 6. Switch-based computation (10 different kernels) */
        int switch_result = switch_kernel(g, i);
        
        /* 7. Conditional with function call */
        if (i & 1) {
            switch_result = helper_mul_chain(switch_result, g);
        }
        
        /* 8. Computed jump (function pointer call) */
        if (funcs[0] != NULL && (i % 3) == 0) {
            switch_result = funcs[0](switch_result, e);
        }
        
        /* 9. Nested loop with carried dependency */
        int nested_sum = 0;
        for (int j = 0; j < 4; j++) {
            nested_sum += int_array[(i + j) % ARRAY_SIZE] * 
                         int_array[(i + j + 1) % ARRAY_SIZE];
            /* Loop-carried dependency */
            int_array[(i + j) % ARRAY_SIZE] = nested_sum % 1000;
        }
        
        /* 10. Reduction across all computed values */
        total += (uint64_t)g + (uint64_t)(fe * 1000) + 
                (uint64_t)fabs(de * 100) + (uint64_t)switch_result + 
                nested_sum;
        
        /* Update arrays with computed values */
        float_array[i % ARRAY_SIZE] = fe;
        double_array[i % ARRAY_SIZE] = de;
        
        /* Memory barrier every 16 iterations */
        if ((i & 0xF) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return total;
}

int main(int argc, char **argv) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000;
        }
    }
    
    printf("Running HAIFA scheduler test with %d iterations\n", iterations);
    
    /* Allocate and initialize arrays with different data types */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct misaligned_data *struct_array = 
        (struct misaligned_data*)malloc(MAX_DEPTH * sizeof(struct misaligned_data));
    
    if (!int_array || !float_array || !double_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245) % 10007;
        float_array[i] = (float)((i * 1103515245) % 10007) / 100.0f;
        double_array[i] = (double)((i * 1103515245) % 10007) / 1000.0;
    }
    
    for (int i = 0; i < MAX_DEPTH; i++) {
        struct_array[i].c = (char)(i & 0xFF);
        struct_array[i].i = i * 3;
        struct_array[i].d = (double)i / 3.0;
        struct_array[i].c2 = (char)((i + 1) & 0xFF);
    }
    
    /* Perform main computation */
    uint64_t result = compute_kernel(iterations, int_array, float_array, 
                                    double_array, struct_array);
    
    /* Additional reduction to prevent optimization */
    uint64_t final_check = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_check ^= (uint64_t)int_array[i];
        final_check += (uint64_t)(float_array[i] * 1000);
        final_check ^= (uint64_t)(double_array[i] * 10000);
    }
    
    for (int i = 0; i < MAX_DEPTH; i++) {
        final_check += struct_array[i].i;
        final_check ^= (uint64_t)struct_array[i].d;
    }
    
    result ^= final_check;
    
    printf("Result: %lu\n", (unsigned long)result);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
