/* haifa_scheduler_test.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa_scheduler_test.c -o haifa_test
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
static volatile float g_volatile_float = 0.0f;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    double d;
    char c2;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Helper functions with different computation patterns */
static int compute_chain_a(int a, int b) {
    int t1 = a * 1103515245 + 12345;
    int t2 = b * 1664525 + 1013904223;
    int t3 = t1 ^ t2;
    int t4 = t3 * 1103515245;
    return t4 ^ (t4 >> 16);
}

static int compute_chain_b(int a, int b) {
    float f1 = (float)a * 3.14159f;
    float f2 = (float)b * 2.71828f;
    float f3 = f1 * f2;
    float f4 = f3 / (fabs(f1) + 1.0f);
    return (int)(f4 * 1000.0f);
}

static double compute_chain_c(double a, double b) {
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += a * b;
        a = sin(a * 0.1);
        b = cos(b * 0.1);
    }
    return sum;
}

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
static int external_computation(int x, int y) {
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += (x << i) ^ (y >> i);
    }
    
    g_volatile_counter++;
    return result;
}

/* Complex conditional chain */
static int deep_conditional_chain(int val, int *array, int idx) {
    int result = val;
    
    if (val & 1) {
        result = compute_chain_a(val, array[idx % ARRAY_SIZE]);
        if (result > 1000) {
            result = external_computation(result, idx);
            if (result & 0x10) {
                float ftmp = (float)result * g_volatile_float;
                result = (int)(ftmp * 100.0f);
            } else {
                result = compute_chain_b(result, idx);
            }
        } else if (result < -1000) {
            result = -result;
            for (int i = 0; i < 3; i++) {
                result ^= array[(idx + i) % ARRAY_SIZE];
            }
        }
    } else if (val & 2) {
        double dtmp = compute_chain_c((double)val, (double)idx);
        result = (int)(dtmp * 1000.0);
        if (result & 4) {
            result = result * 3 - 7;
        }
    } else if (val & 4) {
        result = val * val - val;
        for (int j = 0; j < 5; j++) {
            result += array[(idx + j * 17) % ARRAY_SIZE];
        }
    } else if (val & 8) {
        result = 0;
        for (int k = 0; k < 8; k++) {
            result = (result << 3) | (val & 0x7);
            val >>= 3;
        }
    } else {
        result = external_computation(val, result);
    }
    
    return result;
}

/* Pointer chasing simulation */
static int pointer_chase(int *array, int start_idx, int steps) {
    int idx = start_idx;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Create loop-carried dependency */
        idx = array[idx % ARRAY_SIZE] % ARRAY_SIZE;
        sum += array[idx];
        
        /* Mixed operations to use different functional units */
        float fsum = (float)sum * 0.5f;
        sum = (int)fsum + (idx * 3);
        
        /* Memory barrier every 8 steps */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return sum;
}

/* Large basic block generator */
static void fill_large_block(int *dest, int *src1, int *src2, int size) {
    /* Many independent instructions to fill instruction queue */
    for (int i = 0; i < size; i++) {
        /* Independent operations - scheduler can reorder these */
        int a = src1[i] * 3;
        int b = src2[i] * 7;
        int c = a + b;
        int d = a - b;
        int e = c ^ d;
        int f = (c * d) >> 4;
        int g = e & f;
        dest[i] = g + (i * 11);
        
        /* Volatile access to prevent complete reordering */
        if (i % 16 == 0) {
            g_volatile_float += 0.1f;
        }
    }
}

/* Switch statement with many cases */
static int switch_computation(int val, int case_id) {
    int result = val;
    
    switch (case_id % 10) {
        case 0:
            result = result * 3 + 7;
            result = result ^ 0x55AA55AA;
            break;
        case 1:
            result = (result << 4) | (result >> 28);
            result = result + 0x12345678;
            break;
        case 2:
            result = compute_chain_a(result, case_id);
            result = result % 10007;
            break;
        case 3:
            result = external_computation(result, case_id);
            result = result * 2 - 1;
            break;
        case 4:
            result = deep_conditional_chain(result, &val, case_id);
            break;
        case 5:
            result = result * result;
            result = result - (case_id * case_id);
            break;
        case 6:
            result = (result & 0xFF) << 24 |
                    (result & 0xFF00) << 8 |
                    (result & 0xFF0000) >> 8 |
                    (result & 0xFF000000) >> 24;
            break;
        case 7:
            result = compute_chain_b(result, case_id);
            break;
        case 8:
            result = pointer_chase(&val, result % ARRAY_SIZE, 8);
            break;
        case 9:
            result = result + case_id * 3;
            result = result * 1103515245 + 12345;
            break;
        default:
            result = 0;
    }
    
    return result;
}

/* Main computation kernel */
static uint64_t complex_kernel(int iterations, int init_val) {
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int *int_array2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    struct misaligned_data *struct_array = 
        (struct misaligned_data*)malloc(ARRAY_SIZE * sizeof(struct misaligned_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) % 1000;
        float_array[i] = (float)(i * 1664525 + 1013904223) / 1000000.0f;
        double_array[i] = (double)(i * 1103515245) / 1000000000.0;
        int_array2[i] = (i * 1664525) % 1000;
        
        struct_array[i].c = (char)(i & 0xFF);
        struct_array[i].i = i * 3 - 7;
        struct_array[i].d = (double)i * 1.23456789;
        struct_array[i].c2 = (char)((i >> 8) & 0xFF);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {
        compute_chain_a,
        compute_chain_b,
        NULL, /* Will be filled */
        NULL
    };
    funcs[2] = (compute_func_t)external_computation;
    
    uint64_t accumulator = (uint64_t)init_val;
    int state = init_val;
    
    /* Main computation loop */
    for (int iter = 0; iter < iterations; iter++) {
        /* 1. Pointer chasing with loop-carried dependencies */
        int chase_result = pointer_chase(int_array, state % ARRAY_SIZE, 16);
        state = (state * 1103515245 + 12345) ^ chase_result;
        
        /* 2. Chain of dependent arithmetic operations */
        int a = state * 3;
        int b = a + int_array[iter % ARRAY_SIZE];
        int c = b * 7 - 5;
        float d = (float)c * float_array[iter % ARRAY_SIZE];
        double e = (double)d * double_array[iter % ARRAY_SIZE];
        int f = (int)(e * 1000.0);
        
        /* 3. Switch statement for control flow complexity */
        int switch_result = switch_computation(f, iter);
        
        /* 4. Conditional with function call */
        if (iter & 1) {
            switch_result = external_computation(switch_result, iter);
            
            /* Nested conditional */
            if (switch_result > 1000000) {
                switch_result = deep_conditional_chain(switch_result, int_array, iter);
            }
        }
        
        /* 5. Computed jump via function pointer */
        int func_idx = iter % 3;
        if (funcs[func_idx]) {
            switch_result = funcs[func_idx](switch_result, iter);
        }
        
        /* 6. Mixed data type accesses with misaligned struct */
        struct misaligned_data *s = &struct_array[iter % ARRAY_SIZE];
        switch_result += s->i + (int)(s->d * 100.0);
        
        /* 7. Large basic block generation periodically */
        if (iter % 100 == 0) {
            fill_large_block(int_array2, int_array, &state, 64);
        }
        
        /* 8. Memory barrier every 32 iterations */
        if (iter % 32 == 0) {
            asm volatile("" ::: "memory");
            g_volatile_counter += iter;
        }
        
        /* Accumulate results */
        accumulator ^= (uint64_t)switch_result << 32;
        accumulator += (uint64_t)state;
        accumulator = (accumulator << 13) | (accumulator >> 51);
        
        /* Update volatile */
        g_volatile_float += float_array[iter % ARRAY_SIZE] * 0.01f;
    }
    
    /* Reduction across arrays */
    for (int i = 0; i < ARRAY_SIZE; i += 4) {
        accumulator ^= (uint64_t)int_array[i];
        accumulator += (uint64_t)(float_array[i] * 1000.0f);
        accumulator ^= (uint64_t)(double_array[i] * 1000000.0);
        accumulator += (uint64_t)int_array2[i];
    }
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(int_array2);
    free(struct_array);
    
    return accumulator;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 1000;
        }
    }
    
    printf("Running HAIFA scheduler test with %d iterations...\n", iterations);
    
    /* Run multiple passes to increase scheduling opportunities */
    uint64_t final_result = 0;
    for (int pass = 0; pass < 3; pass++) {
        uint64_t pass_result = complex_kernel(iterations / 3, pass * 1000 + 123);
        final_result ^= pass_result;
        
        printf("Pass %d result: 0x%016llx\n", pass, (unsigned long long)pass_result);
        
        /* Force different scheduling patterns each pass */
        g_volatile_counter += pass * 1000;
        g_volatile_float = (float)pass * 0.5f;
    }
    
    printf("Final result: 0x%016llx\n", (unsigned long long)final_result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Volatile float: %f\n", g_volatile_float);
    
    return (final_result & 0xFFFFFFFF) == 0 ? 0 : 1;
}
