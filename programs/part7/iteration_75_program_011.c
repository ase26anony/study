/* haifa-sched-coverage.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fsched-pressure -funroll-loops haifa-sched-coverage.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Volatile variables to prevent optimization and create scheduling hazards */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) misaligned_data {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that create scheduling boundaries */
static int helper1(int a, int b) {
    asm volatile("" ::: "memory"); /* Compiler barrier */
    return (a * b) + (a ^ b) - (a & b);
}

static float helper2(float a, float b) {
    volatile float temp = a * b;
    return temp + a - b;
}

static double helper3(double *arr, int idx) {
    /* Pointer chasing with volatile */
    volatile double *ptr = &arr[idx];
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += *ptr;
        ptr = &arr[(idx + i) % 256];
    }
    return sum;
}

/* Non-inlineable function (due to complexity) */
__attribute__((noinline)) 
int complex_switch(int val, int *arr, float *farr, double *darr) {
    int result = 0;
    
    /* Deep switch with many cases */
    switch (val % 12) {
        case 0: {
            /* Chain of dependent operations */
            int t1 = arr[val] * 3;
            int t2 = t1 + arr[val + 1];
            int t3 = t2 ^ arr[val + 2];
            int t4 = t3 * 1103515245;
            result = t4 % 65536;
            break;
        }
        case 1: {
            /* Mixed float/int operations */
            float f1 = farr[val] * 2.5f;
            int i1 = (int)f1;
            float f2 = f1 + farr[val + 1];
            result = i1 * (int)f2;
            break;
        }
        case 2: {
            /* Double precision chain */
            double d1 = darr[val] * 1.41421356;
            double d2 = d1 + darr[val + 1];
            double d3 = d2 * darr[val + 2];
            result = (int)(d3 * 1000.0);
            break;
        }
        case 3: {
            /* Memory intensive */
            for (int i = 0; i < 8; i++) {
                arr[(val + i) % 256] = arr[(val + i + 1) % 256] * 2;
            }
            result = arr[val % 256];
            break;
        }
        case 4: {
            /* Function call in switch */
            result = helper1(val, arr[val % 256]);
            break;
        }
        case 5: {
            /* Volatile operations */
            g_volatile_counter++;
            result = g_volatile_counter * val;
            break;
        }
        case 6: {
            /* Nested conditionals */
            if (val & 1) {
                result = arr[val % 256] * 3;
            } else if (val & 2) {
                result = arr[val % 256] + 5;
            } else if (val & 4) {
                result = arr[val % 256] ^ 0xFF;
            } else {
                result = ~arr[val % 256];
            }
            break;
        }
        case 7: {
            /* Loop with carried dependency */
            int sum = 0;
            for (int i = 0; i < 16; i++) {
                sum += arr[(val + i) % 256] * arr[(val + i - 1) % 256];
            }
            result = sum;
            break;
        }
        case 8: {
            /* Mixed operations */
            float f = helper2((float)val, (float)arr[val % 256]);
            double d = helper3(darr, val % 256);
            result = (int)(f * d);
            break;
        }
        case 9: {
            /* Pointer arithmetic chain */
            int *ptr = arr + (val % 256);
            for (int i = 0; i < 4; i++) {
                *ptr = *ptr * 2 + i;
                ptr = arr + ((val + i * 7) % 256);
            }
            result = arr[val % 256];
            break;
        }
        case 10: {
            /* Bit manipulation chain */
            int x = val;
            x = (x >> 1) ^ (x << 3);
            x = x * 0x9e3779b9;
            x = x ^ (x >> 16);
            result = x;
            break;
        }
        case 11: {
            /* Array initialization with no dependencies between iterations */
            for (int i = 0; i < 32; i++) {
                arr[(val + i * 8) % 256] = i * 1103515245;
            }
            result = arr[val % 256];
            break;
        }
    }
    
    return result;
}

/* Function using computed goto via function pointers */
static int computed_jump_computation(int x, int y, compute_func_t *funcs) {
    int idx = (x ^ y) % 6;
    return funcs[idx](x, y);
}

int main(int argc, char **argv) {
    /* Parse iteration count */
    int N = 10000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 10000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)aligned_alloc(64, 256 * sizeof(int));
    float *float_array = (float*)malloc(257 * sizeof(float)); /* Misaligned */
    double *double_array = (double*)aligned_alloc(32, 256 * sizeof(double));
    struct misaligned_data *struct_array = 
        (struct misaligned_data*)malloc(100 * sizeof(struct misaligned_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        int_array[i] = i * 1103515245;
        float_array[i] = (float)(i * 1.6180339887);
        double_array[i] = (double)(i * 3.1415926535);
    }
    
    for (int i = 0; i < 100; i++) {
        struct_array[i].c = (char)(i & 0xFF);
        struct_array[i].i = i * 7;
        struct_array[i].d = (double)i / 3.0;
        struct_array[i].s = (short)(i * 13);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[6] = {
        helper1,
        (compute_func_t)helper2, /* Cast for variety */
        (compute_func_t)helper3,
        NULL, NULL, NULL
    };
    funcs[3] = (compute_func_t)complex_switch;
    funcs[4] = (compute_func_t)computed_jump_computation;
    funcs[5] = (compute_func_t)helper1;
    
    /* Main computation loop - designed to create complex scheduling */
    uint64_t final_result = 0;
    
    for (int iter = 0; iter < N; iter++) {
        int base_idx = iter % 256;
        
        /* 1. Pointer chasing simulation */
        int *chase_ptr = &int_array[base_idx];
        int chase_sum = 0;
        for (int i = 0; i < 32; i++) {
            chase_sum += *chase_ptr;
            int next_idx = (*chase_ptr) % 256;
            chase_ptr = &int_array[next_idx];
            asm volatile("" ::: "memory"); /* Barrier */
        }
        
        /* 2. Chain of dependent arithmetic operations */
        int a = int_array[base_idx];
        int b = int_array[(base_idx + 1) % 256];
        float c = float_array[base_idx];
        double d = double_array[base_idx];
        
        /* Complex dependency chain */
        int t1 = a * b + chase_sum;
        float t2 = c * (float)t1;
        double t3 = d + (double)t2;
        int t4 = (int)(t3 * 100.0) ^ t1;
        float t5 = t2 + (float)t4;
        double t6 = t3 * (double)t5;
        int t7 = t4 * (int)t6;
        
        /* 3. Switch statement with many branches */
        int switch_result = complex_switch(iter, int_array, float_array, double_array);
        
        /* 4. Conditional with function call */
        int func_result;
        if (iter & 1) {
            func_result = helper1(t7, switch_result);
        } else if (iter & 2) {
            func_result = (int)helper2((float)t7, (float)switch_result);
        } else {
            func_result = computed_jump_computation(t7, switch_result, funcs);
        }
        
        /* 5. Mixed data type accesses with varying alignments */
        struct misaligned_data *sptr = &struct_array[iter % 100];
        int struct_val = sptr->i + (int)sptr->d + sptr->s;
        
        /* 6. Large basic block with independent operations */
        /* This should fill the instruction queue */
        int independent_results[8];
        for (int i = 0; i < 8; i++) {
            independent_results[i] = int_array[(base_idx + i * 7) % 256] * 
                                     int_array[(base_idx + i * 13) % 256];
        }
        
        /* 7. Reduction across independent results */
        int block_result = 0;
        for (int i = 0; i < 8; i++) {
            block_result ^= independent_results[i];
        }
        
        /* 8. Final combination with volatile */
        g_volatile_double += (double)(t7 + switch_result + func_result + 
                                     struct_val + block_result);
        
        /* Update final result */
        final_result ^= (uint64_t)t7;
        final_result += (uint64_t)switch_result;
        final_result ^= (uint64_t)func_result << 16;
        final_result += (uint64_t)struct_val;
        final_result ^= (uint64_t)block_result << 32;
        
        /* Modify arrays to create loop-carried dependencies */
        int_array[base_idx] = (int_array[base_idx] * 3 + 1) ^ final_result;
        float_array[base_idx] = float_array[base_idx] * 1.1f + (float)final_result;
        double_array[base_idx] = double_array[base_idx] * 0.9 + (double)final_result;
    }
    
    /* Final reduction across arrays */
    uint64_t array_sum = 0;
    for (int i = 0; i < 256; i++) {
        array_sum += int_array[i];
        array_sum ^= (uint64_t)(float_array[i] * 1000.0f);
        array_sum += (uint64_t)(double_array[i] * 1000.0);
    }
    
    final_result ^= array_sum;
    final_result += (uint64_t)g_volatile_double;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %lu\n", (unsigned long)final_result);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(struct_array);
    
    return 0;
}
