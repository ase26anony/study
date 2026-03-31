/* haifa_scheduler_test.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -funroll-loops haifa_scheduler_test.c -o scheduler_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

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

/* Small helper functions that create scheduling boundaries */
static int helper1(int a, int b) {
    volatile int barrier;
    barrier = a * b;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return barrier + (a ^ b);
}

static int helper2(int a, int b) {
    volatile int temp = a;
    for (int i = 0; i < 3; i++) {
        temp = (temp * 1103515245 + 12345) & 0x7fffffff;
    }
    asm volatile("" ::: "memory");
    return temp % (b + 1);
}

static float helper3(float a, float b) {
    volatile float result = a;
    result = result * b - a / (b + 1.0f);
    asm volatile("" ::: "memory");
    return result;
}

/* Complex computation kernel with many dependencies */
static int complex_kernel(int *arr, float *farr, double *darr, 
                         struct misaligned_data *marr, int idx) {
    int result = 0;
    volatile int dep1, dep2, dep3;
    
    /* Chain of dependent operations */
    dep1 = arr[idx] * 1103515245;
    dep2 = dep1 ^ (idx * 12345);
    dep3 = dep2 + (int)(farr[idx] * 1000.0f);
    
    /* Pointer chasing through array */
    int next_idx = dep3 % 256;
    if (next_idx >= 0 && next_idx < 256) {
        result = arr[next_idx] + (int)darr[idx];
    }
    
    /* Mixed type operations */
    float ftemp = (float)dep3 * 0.5f;
    double dtemp = (double)ftemp * marr[idx].d;
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    /* More dependencies */
    result += (int)(dtemp * 100.0) + marr[idx].i;
    
    /* Update volatile globals */
    g_volatile_counter++;
    g_volatile_float += ftemp;
    
    return result;
}

/* Switch-based dispatcher with different computation patterns */
static int switch_dispatcher(int i, int *arr, float *farr, double *darr) {
    int result = 0;
    
    switch (i % 10) {
        case 0:
            /* Long dependency chain */
            result = arr[i];
            for (int j = 0; j < 8; j++) {
                result = (result * 1664525 + 1013904223) & 0x7fffffff;
                result ^= (int)farr[j];
            }
            break;
            
        case 1:
            /* Memory intensive */
            for (int j = 0; j < 4; j++) {
                arr[(i + j) % 256] += arr[(i - j + 256) % 256];
                farr[(i + j) % 256] *= 1.01f;
            }
            result = arr[i % 256];
            break;
            
        case 2:
            /* Function call mix */
            result = helper1(arr[i], arr[(i + 1) % 256]);
            result += helper2(result, i);
            break;
            
        case 3:
            /* Floating point chain */
            float ftemp = farr[i];
            for (int j = 0; j < 6; j++) {
                ftemp = ftemp * 1.5f - ftemp / 2.0f;
                ftemp = helper3(ftemp, (float)j);
            }
            result = (int)ftemp;
            break;
            
        case 4:
            /* Nested loops with dependencies */
            for (int j = 0; j < 3; j++) {
                int sum = 0;
                for (int k = 0; k < 3; k++) {
                    sum += arr[(i + j + k) % 256] * arr[(i + j - k + 256) % 256];
                }
                result += sum;
            }
            break;
            
        case 5:
            /* Conditional chain */
            if (i & 1) {
                result = helper1(arr[i], i);
            } else if (i & 2) {
                result = helper2(arr[i], i);
            } else if (i & 4) {
                result = arr[i] * arr[(i + 128) % 256];
            } else {
                result = arr[i] ^ arr[(i + 64) % 256];
            }
            break;
            
        case 6:
            /* Mixed computations */
            result = (int)(sin((double)i) * 1000.0);
            result += (int)(cos((double)arr[i]) * 1000.0);
            break;
            
        case 7:
            /* Pointer arithmetic */
            int *ptr = arr + (i % 128);
            for (int j = 0; j < 4; j++) {
                result += *ptr;
                ptr += (j * 31) % 64;
                if (ptr >= arr + 256) ptr = arr;
            }
            break;
            
        case 8:
            /* Bit manipulation chain */
            result = arr[i];
            result = ((result << 5) | (result >> 27)) ^ 0x9e3779b9;
            result = ((result << 13) | (result >> 19)) + i;
            result = ((result << 17) | (result >> 15)) ^ arr[(i + 1) % 256];
            break;
            
        case 9:
            /* All of the above mixed */
            result = switch_dispatcher(i + 1, arr, farr, darr);
            result += complex_kernel(arr, farr, darr, 
                                    (struct misaligned_data *)arr, i);
            break;
    }
    
    return result;
}

/* Main computation with deep nesting and complex control flow */
static uint64_t compute(int iterations) {
    /* Allocate arrays with different types and alignments */
    int *int_array = (int *)aligned_alloc(64, 256 * sizeof(int));
    float *float_array = (float *)aligned_alloc(32, 256 * sizeof(float));
    double *double_array = (double *)aligned_alloc(64, 256 * sizeof(double));
    struct misaligned_data *misaligned_array = 
        (struct misaligned_data *)malloc(256 * sizeof(struct misaligned_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        float_array[i] = (float)int_array[i] / 1000.0f;
        double_array[i] = (double)int_array[i] / 10000.0;
        
        misaligned_array[i].c = (char)(i & 0xff);
        misaligned_array[i].i = int_array[i];
        misaligned_array[i].d = double_array[i];
        misaligned_array[i].c2 = (char)((i >> 8) & 0xff);
    }
    
    uint64_t total_result = 0;
    int branch_counter = 0;
    
    /* Main computation loop with complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* Large basic block with many independent operations */
        int temp_results[32];
        for (int i = 0; i < 32; i++) {
            /* Independent computations to fill instruction queue */
            temp_results[i] = int_array[(iter + i * 7) % 256] * 
                             int_array[(iter + i * 13) % 256];
            float_array[(iter + i * 5) % 256] += 0.1f;
            double_array[(iter + i * 11) % 256] *= 1.0001;
        }
        
        /* Process results with dependencies */
        int chain_result = 0;
        for (int i = 0; i < 32; i++) {
            chain_result = chain_result * 31 + temp_results[i];
        }
        
        /* Deeply nested conditionals */
        if (iter & 1) {
            if (chain_result & 2) {
                if ((iter ^ chain_result) & 4) {
                    /* Call helper in deepest branch */
                    chain_result = helper1(chain_result, iter);
                } else {
                    chain_result = helper2(chain_result, iter);
                }
            } else {
                /* Inline computation */
                for (int j = 0; j < 8; j++) {
                    chain_result = (chain_result << 3) | (chain_result >> 29);
                    chain_result ^= int_array[(iter + j) % 256];
                }
            }
        }
        
        /* Switch statement with many cases */
        int switch_result = switch_dispatcher(iter, int_array, 
                                            float_array, double_array);
        
        /* Pointer chasing simulation */
        int chase_idx = iter % 256;
        int chase_sum = 0;
        for (int i = 0; i < 16; i++) {
            chase_sum += int_array[chase_idx];
            chase_idx = (chase_idx * 13 + 17) % 256;
            if (chase_idx < 0) chase_idx += 256;
        }
        
        /* Computed goto simulation using function pointers */
        compute_func_t funcs[] = {helper1, helper2};
        int func_result = funcs[iter & 1](chase_sum, switch_result);
        
        /* Combine all results */
        int final_iter_result = chain_result + switch_result + 
                               chase_sum + func_result;
        
        /* Update arrays creating loop-carried dependencies */
        int_array[iter % 256] = (int_array[iter % 256] + final_iter_result) & 0x7fffffff;
        float_array[iter % 256] = fmod(float_array[iter % 256] + 
                                      (float)final_iter_result / 1000.0f, 1000.0f);
        
        /* Accumulate to total with non-linear operation */
        total_result = (total_result * 6364136223846793005ULL + 
                       (uint64_t)final_iter_result);
        
        /* Branch counter for volatile access */
        branch_counter++;
        if (branch_counter >= 100) {
            g_volatile_counter += branch_counter;
            branch_counter = 0;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Final reduction across all arrays */
    uint64_t final_check = total_result;
    for (int i = 0; i < 256; i++) {
        final_check ^= (uint64_t)int_array[i];
        final_check += (uint64_t)(float_array[i] * 1000.0f);
        final_check = (final_check << 13) | (final_check >> 51);
    }
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(misaligned_array);
    
    return final_check;
}

int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    printf("Running HAIFA scheduler test with %d iterations...\n", iterations);
    
    uint64_t result = compute(iterations);
    
    /* Mix in volatile globals */
    result ^= (uint64_t)g_volatile_counter;
    result += (uint64_t)(g_volatile_float * 1000.0f);
    
    printf("Result: 0x%016llx\n", (unsigned long long)result);
    
    return 0;
}
