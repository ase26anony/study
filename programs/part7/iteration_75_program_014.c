/* haifa-sched-coverage.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa-sched-coverage.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define MAX_DEPTH 32

/* Mixed data types with different alignments */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    float f;
    double d;
    char pad[3];
};

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile struct MixedData g_volatile_data;

/* Function pointer for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Small helper functions that may be inlined or not */
static inline int helper_mul(int a, int b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    return a * b + (a ^ b);
}

static int helper_div(int a, int b) {
    if (b == 0) b = 1;
    int result = a / b;
    g_volatile_counter += result;
    return result;
}

static double helper_fp(double a, double b) {
    volatile double temp = a * b;
    asm volatile("" : : "r"(temp) : "memory");
    return temp - a + b;
}

/* Non-inlineable function with side effects */
__attribute__((noinline)) 
int complex_helper(int *arr, int idx) {
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += arr[(idx + i) % ARRAY_SIZE];
        asm volatile("" : : "r"(sum) : "memory");
    }
    g_volatile_data.i = sum;
    return sum;
}

/* Pointer chasing through array */
int pointer_chase(int *array, int start, int steps) {
    int idx = start;
    int sum = 0;
    for (int i = 0; i < steps; i++) {
        idx = array[idx % ARRAY_SIZE];
        sum += idx;
        /* Memory barrier to prevent reordering */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Deep conditional chain */
int deep_conditional(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        result = helper_mul(x, y);
        if (y < 0) {
            result = helper_div(result, z);
            if (z == 0) {
                result ^= 0xABCD;
            } else if (z > 100) {
                result |= 0x1234;
            } else {
                result &= 0xFF;
            }
        } else if (y == 0) {
            result = x << 4;
        } else {
            result = y >> 2;
        }
    } else if (x < -10) {
        result = x * y * z;
        if (result > 1000) {
            result /= 3;
        } else if (result < -1000) {
            result *= -2;
        } else {
            result += 777;
        }
    } else {
        result = x + y + z;
        for (int i = 0; i < 4; i++) {
            result = helper_mul(result, i + 1);
        }
    }
    
    return result;
}

/* Switch-based computation kernel */
int switch_kernel(int val, int case_id) {
    int result = val;
    
    switch (case_id % 10) {
        case 0:
            result = helper_mul(result, 7);
            result ^= 0xDEADBEEF;
            break;
        case 1:
            result = result + (result << 2);
            result = helper_div(result, 3);
            break;
        case 2:
            for (int i = 0; i < 5; i++) {
                result = helper_mul(result, i + 2);
            }
            break;
        case 3:
            result = pointer_chase(&result, result & 0xFF, 8);
            break;
        case 4:
            result = deep_conditional(result, case_id, val);
            break;
        case 5:
            result = result * 3 / 2;
            asm volatile("" : : "r"(result) : "memory");
            result = result ^ (result >> 1);
            break;
        case 6:
            result = (result << 4) | (result >> 28);
            result = helper_mul(result, result);
            break;
        case 7:
            for (int i = 0; i < 3; i++) {
                result += complex_helper(&result, i);
            }
            break;
        case 8:
            result = result * 1103515245 + 12345;
            result = (result & 0x7FFFFFFF);
            break;
        case 9:
            result = helper_div(result * 2, result + 1);
            result = result | 0x80000000;
            break;
    }
    
    return result;
}

/* Main computation with complex control flow */
uint64_t compute_kernel(int iterations, int *int_array, double *double_array, 
                       float *float_array, struct MixedData *struct_array) {
    uint64_t total = 0;
    int temp_results[MAX_DEPTH];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFF;
        double_array[i] = (i * 0.6180339887);
        float_array[i] = (i * 0.3141592653f);
        struct_array[i].i = i;
        struct_array[i].f = i * 1.414f;
        struct_array[i].d = i * 2.71828;
    }
    
    /* Create linked-list structure in array */
    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        int_array[i] = (i + 1) % ARRAY_SIZE;
    }
    
    /* Main computation loop */
    for (int iter = 0; iter < iterations; iter++) {
        int idx = iter % ARRAY_SIZE;
        int chain_result = int_array[idx];
        
        /* Chain of dependent operations */
        chain_result = helper_mul(chain_result, iter);
        chain_result = chain_result + (chain_result << 3);
        chain_result = helper_div(chain_result, (iter & 0xFF) + 1);
        chain_result = chain_result ^ int_array[(iter + 1) % ARRAY_SIZE];
        
        /* Pointer chasing */
        int chase_result = pointer_chase(int_array, idx, 16);
        
        /* Mixed floating point operations */
        double fp_result = double_array[idx];
        for (int j = 0; j < 4; j++) {
            fp_result = helper_fp(fp_result, double_array[(idx + j) % ARRAY_SIZE]);
            fp_result = fp_result * 1.1 - 0.3;
        }
        float_array[idx] = (float)fp_result;
        
        /* Struct operations with volatile */
        struct_array[idx].i = chain_result;
        struct_array[idx].f = float_array[idx];
        g_volatile_data = struct_array[idx];
        
        /* Switch-based computation */
        int switch_result = switch_kernel(chain_result, iter);
        
        /* Deep conditional chain */
        int cond_result = deep_conditional(chain_result, chase_result, switch_result);
        
        /* Function pointer dispatch */
        ComputeFunc funcs[] = {helper_mul, helper_div};
        int func_result = funcs[iter & 1](cond_result, switch_result);
        
        /* Nested loop with loop-carried dependency */
        int loop_result = 0;
        for (int i = 0; i < 32; i++) {
            loop_result += int_array[(idx + i) % ARRAY_SIZE];
            loop_result = helper_mul(loop_result, i + 1);
            if (i & 1) {
                loop_result = complex_helper(int_array, loop_result & 0xFF);
            }
        }
        
        /* Store in temp array for reduction */
        temp_results[iter % MAX_DEPTH] = func_result + loop_result;
        
        /* Conditional with goto (non-linear control flow) */
        if ((iter & 0xF) == 0) {
            goto special_case;
        }
        
        /* Normal path */
        total += func_result;
        continue;
        
    special_case:
        /* Special computation path */
        int special_sum = 0;
        for (int i = 0; i < 8; i++) {
            special_sum ^= int_array[(iter + i) % ARRAY_SIZE];
            special_sum = special_sum * 0x9E3779B9;
        }
        total += special_sum;
        
        /* Another memory barrier */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    /* Final reduction across all data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        total ^= int_array[i];
        total += (uint64_t)(double_array[i] * 1000);
        total += struct_array[i].i;
        
        /* Mix in volatile data */
        total += g_volatile_counter;
        total ^= g_volatile_data.i;
    }
    
    /* Reduce temp results */
    for (int i = 0; i < MAX_DEPTH; i++) {
        total += temp_results[i];
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    
    /* Allocate arrays with different alignments */
    int *int_array = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)aligned_alloc(64, ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    struct MixedData *struct_array = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    if (!int_array || !double_array || !float_array || !struct_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize volatile data */
    g_volatile_counter = 0;
    g_volatile_data.i = 0x12345678;
    g_volatile_data.f = 3.14159f;
    g_volatile_data.d = 2.71828;
    
    /* Run computation */
    clock_t start = clock();
    uint64_t result = compute_kernel(iterations, int_array, double_array, 
                                    float_array, struct_array);
    clock_t end = clock();
    
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016llX\n", (unsigned long long)result);
    printf("Iterations: %d, Time: %.3f seconds\n", iterations, elapsed);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(struct_array);
    
    return 0;
}
