/* haifa-sched-coverage.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa-sched-coverage.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile float g_volatile_float = 1.0f;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) mixed_data {
    char c;
    int i;
    double d;
    float f;
    char trailing;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions with different computation patterns */
static int helper_mul_chain(int a, int b) {
    int t1 = a * 3;
    int t2 = b * 7;
    int t3 = t1 * t2;
    int t4 = t3 * 11;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return t4 ^ (t1 + t2);
}

static int helper_add_chain(int a, int b) {
    int sum = a;
    for (int i = 0; i < 8; i++) {
        sum += b + i;
        sum ^= (sum << 3);
    }
    return sum;
}

static float helper_float_ops(float a, float b) {
    float t = a * 2.5f;
    t += b * 1.7f;
    t /= 3.2f;
    t = t * t - 1.0f;
    asm volatile("" ::: "memory");
    return t;
}

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) static double complex_reduction(double *arr, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] * (i & 1 ? 0.5 : 2.0);
        sum = sum / (1.0 + (i % 3));
    }
    return sum;
}

/* Main computation with complex control flow */
static uint64_t compute_kernel(int iterations, int array_size) {
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(array_size * sizeof(int));
    float *float_array = (float*)malloc(array_size * sizeof(float));
    double *double_array = (double*)malloc(array_size * sizeof(double));
    struct mixed_data *mixed = (struct mixed_data*)malloc(array_size * sizeof(struct mixed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        float_array[i] = (float)int_array[i] / 1000.0f;
        double_array[i] = (double)int_array[i] / 500.0;
        mixed[i].c = (char)(i & 0xFF);
        mixed[i].i = int_array[i];
        mixed[i].d = double_array[i];
        mixed[i].f = float_array[i];
        mixed[i].trailing = (char)((i + 1) & 0xFF);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {helper_mul_chain, helper_add_chain};
    
    uint64_t accumulator = 0;
    int *chase_ptr = int_array;
    
    /* Primary computation loop with complex dependencies */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pointer chasing through array (simulated linked list) */
        int chase_sum = 0;
        for (int i = 0; i < 100; i++) {
            chase_sum += *chase_ptr;
            chase_ptr = &int_array[*chase_ptr % array_size];
            asm volatile("" ::: "memory");  /* Prevent optimization */
        }
        
        /* Chain of dependent arithmetic operations */
        int a = chase_sum;
        int b = a * 3 + iter;
        int c = b / 2 - a;
        int d = c * 7 ^ b;
        int e = d + (a << 2);
        int f = e * 11 - d;
        
        /* Mixed integer/float operations */
        float fa = (float)a * 1.5f;
        float fb = fa + (float)b * 0.7f;
        double dc = (double)c * 2.3;
        double dd = dc / (1.0 + (double)d * 0.01);
        
        /* Deeply nested conditional chain */
        if (iter & 1) {
            if (iter & 2) {
                if (iter & 4) {
                    f = helper_mul_chain(f, e);
                } else {
                    f = helper_add_chain(f, e);
                }
                fa = helper_float_ops(fa, fb);
            } else {
                dc = dd * 1.7 - dc;
            }
            
            /* Access packed struct with misaligned fields */
            int mixed_idx = iter % array_size;
            accumulator += mixed[mixed_idx].i;
            accumulator ^= (uint64_t)mixed[mixed_idx].d;
            fa += mixed[mixed_idx].f;
        }
        
        /* Switch statement with many cases */
        switch (iter % 10) {
            case 0: {
                /* Kernel 0: Integer multiply-accumulate */
                int t = 1;
                for (int j = 0; j < 20; j++) {
                    t = t * (int_array[j % array_size] & 0xFF) + j;
                }
                accumulator += t;
                break;
            }
            case 1: {
                /* Kernel 1: Floating point operations */
                float ft = 1.0f;
                for (int j = 0; j < 15; j++) {
                    ft = ft * float_array[j % array_size] - (float)j * 0.1f;
                }
                g_volatile_float = ft;  /* Volatile store */
                break;
            }
            case 2: {
                /* Kernel 2: Memory-intensive */
                for (int j = 0; j < 50; j++) {
                    int idx = (iter + j) % array_size;
                    double_array[idx] = double_array[idx] * 1.01 + (double)j;
                }
                break;
            }
            case 3: {
                /* Kernel 3: Function pointer call */
                int idx = iter % 2;
                f = funcs[idx](f, e);
                break;
            }
            case 4: {
                /* Kernel 4: Reduction chain */
                double sum = 0.0;
                for (int j = 0; j < 30; j++) {
                    sum += double_array[j % array_size] * (j & 1 ? -1.0 : 1.0);
                }
                accumulator ^= (uint64_t)sum;
                break;
            }
            case 5: {
                /* Kernel 5: Integer bit manipulation */
                int bits = f;
                for (int j = 0; j < 16; j++) {
                    bits = (bits << 1) | ((bits >> 31) & 1);
                    bits ^= (1 << (j % 32));
                }
                f = bits;
                break;
            }
            case 6: {
                /* Kernel 6: Mixed type conversions */
                for (int j = 0; j < 25; j++) {
                    float_array[j % array_size] = (float)((double)int_array[j % array_size] * 0.3);
                }
                break;
            }
            case 7: {
                /* Kernel 7: Volatile access pattern */
                g_volatile_counter++;
                for (int j = 0; j < 10; j++) {
                    int_array[j % array_size] += g_volatile_counter;
                }
                break;
            }
            case 8: {
                /* Kernel 8: Dependent chain with barriers */
                int chain = iter;
                for (int j = 0; j < 40; j++) {
                    chain = chain * 3 + j;
                    asm volatile("" ::: "memory");
                    chain = chain ^ (chain >> 3);
                }
                accumulator += chain;
                break;
            }
            case 9: {
                /* Kernel 9: Call to non-inlineable function */
                double red = complex_reduction(double_array, array_size > 50 ? 50 : array_size);
                accumulator ^= (uint64_t)(red * 1000.0);
                break;
            }
        }
        
        /* Final reduction across computed values */
        accumulator += (uint64_t)f;
        accumulator ^= (uint64_t)(fa * 100.0f);
        accumulator += (uint64_t)(dc + dd);
        
        /* Update arrays with data-dependent indices */
        int write_idx = accumulator % array_size;
        int_array[write_idx] = f;
        float_array[write_idx] = fa;
        double_array[write_idx] = dc;
    }
    
    /* Final reduction across all arrays */
    uint64_t final_result = 0;
    for (int i = 0; i < array_size; i++) {
        final_result ^= (uint64_t)int_array[i];
        final_result += (uint64_t)(float_array[i] * 1000.0f);
        final_result ^= (uint64_t)double_array[i];
        if (i % 4 == 0) {
            final_result += mixed[i].i;
        }
    }
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed);
    
    return final_result;
}

int main(int argc, char **argv) {
    /* Parse iteration count from command line */
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Use different array sizes to vary memory access patterns */
    int array_size = 1024;
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size < 100) array_size = 100;
        if (array_size > 10000) array_size = 10000;
    }
    
    printf("Running HAIFA scheduler test: iterations=%d, array_size=%d\n", 
           iterations, array_size);
    
    /* Run computation kernel multiple times to increase coverage probability */
    uint64_t total_result = 0;
    for (int run = 0; run < 3; run++) {
        uint64_t result = compute_kernel(iterations, array_size);
        total_result ^= result;
        printf("Run %d: result = 0x%016llx\n", run, (unsigned long long)result);
    }
    
    printf("Final result: 0x%016llx\n", (unsigned long long)total_result);
    
    /* Use result to prevent dead code elimination */
    if (total_result == 0x123456789ABCDEF0ULL) {
        printf("Impossible condition\n");
    }
    
    return 0;
}
