#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Complex struct with mixed types and packing */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    float f;
    char arr[3];
};

/* Volatile variables to create scheduling hazards */
volatile int vol_counter = 0;
volatile double vol_double = 1.0;

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that won't be inlined easily */
static __attribute__((noinline)) int helper1(int a, int b) {
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return (a * b) + (a ^ b) - (a & b);
}

static __attribute__((noinline)) float helper2(float a, float b) {
    asm volatile("" ::: "memory");
    return a * b + a / (b + 1.0f);
}

static __attribute__((noinline)) double helper3(double a, double b) {
    asm volatile("" ::: "memory");
    return sqrt(a * a + b * b) * sin(a) * cos(b);
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int i, int *arr, float *farr, double *darr) {
    int result = 0;
    
    switch (i % 10) {
        case 0:
            result = arr[i] * arr[i+1] + arr[i-1];
            farr[i % 100] = helper2(farr[i % 100], farr[(i+1) % 100]);
            break;
        case 1:
            result = arr[i] ^ arr[i+1] | arr[i-1];
            darr[i % 100] = helper3(darr[i % 100], darr[(i+1) % 100]);
            break;
        case 2:
            result = (arr[i] << 3) | (arr[i+1] >> 2);
            farr[i % 100] = farr[i % 100] * 1.1f + 0.5f;
            break;
        case 3:
            result = arr[i] * 1103515245 + 12345;
            darr[i % 100] = darr[i % 100] * 0.99 + 0.01;
            break;
        case 4:
            result = ~arr[i] & arr[i+1];
            farr[i % 100] = sqrtf(farr[i % 100] * farr[i % 100] + 1.0f);
            break;
        case 5:
            result = arr[i] % (arr[i+1] + 1);
            darr[i % 100] = log(darr[i % 100] + 1.0);
            break;
        case 6:
            result = arr[i] + (arr[i+1] << 1) - (arr[i-1] >> 1);
            farr[i % 100] = farr[i % 100] + vol_double;
            break;
        case 7:
            result = arr[i] * arr[i] - arr[i+1] * arr[i+1];
            darr[i % 100] = darr[i % 100] * vol_double;
            break;
        case 8:
            result = (arr[i] & 0xFF) | ((arr[i+1] & 0xFF) << 8);
            farr[i % 100] = helper2(farr[i % 100], vol_double);
            break;
        case 9:
            result = arr[i] + helper1(arr[i+1], arr[i-1]);
            darr[i % 100] = helper3(darr[i % 100], vol_double);
            break;
    }
    
    return result;
}

/* Deeply nested conditional chain */
static int nested_conditional(int i, int *arr, struct MixedData *mdata) {
    int result = 0;
    
    if (i & 1) {
        result = helper1(arr[i], arr[i+1]);
        if (i & 2) {
            mdata[i % 50].i = result;
            if (i & 4) {
                result ^= mdata[i % 50].c;
                if (i & 8) {
                    mdata[i % 50].d = helper3(result, i);
                    if (i & 16) {
                        result += (int)mdata[i % 50].f;
                        if (i & 32) {
                            mdata[i % 50].arr[0] = result & 0xFF;
                        }
                    }
                }
            }
        }
    } else {
        result = arr[i] * 3;
        if (!(i & 2)) {
            mdata[i % 50].f = helper2(result, i);
            if (!(i & 4)) {
                result -= (int)mdata[i % 50].d;
                if (!(i & 8)) {
                    mdata[i % 50].i = result ^ i;
                }
            }
        }
    }
    
    return result;
}

/* Pointer chasing simulation */
static int pointer_chase(int *arr, int size, int start) {
    int index = start;
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        sum += arr[index];
        /* Create loop-carried dependency */
        index = (arr[index] * 1103515245 + 12345) % size;
        if (index < 0) index = -index;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

/* Large basic block with many independent operations */
static void large_basic_block(int *arr, float *farr, double *darr, 
                             struct MixedData *mdata, int idx) {
    /* Many independent operations to fill instruction queue */
    int t1 = arr[idx] * 2;
    int t2 = arr[idx+1] + 3;
    int t3 = t1 ^ t2;
    float f1 = farr[idx % 100] * 1.5f;
    float f2 = farr[(idx+1) % 100] / 2.0f;
    double d1 = darr[idx % 100] * 0.75;
    double d2 = darr[(idx+1) % 100] + 0.25;
    
    arr[idx] = t3 + t1 - t2;
    farr[idx % 100] = f1 + f2 * 0.3f;
    darr[idx % 100] = d1 * d2 - 0.1;
    
    mdata[idx % 50].i = t3;
    mdata[idx % 50].f = f1;
    mdata[idx % 50].d = d1;
    
    /* More independent operations */
    int t4 = arr[idx+2] << 2;
    int t5 = arr[idx+3] >> 1;
    float f3 = sqrtf(farr[idx % 100]);
    double d3 = log(darr[idx % 100] + 1.0);
    
    arr[idx+1] = t4 | t5;
    farr[(idx+1) % 100] = f3 * 2.0f;
    darr[(idx+1) % 100] = d3 * 0.5;
    
    /* Even more operations */
    int t6 = arr[idx+4] % 17;
    int t7 = arr[idx+5] & 0xFF;
    float f4 = sinf(farr[idx % 100]);
    double d4 = cos(darr[idx % 100]);
    
    arr[idx+2] = t6 * t7;
    farr[(idx+2) % 100] = f4 + 1.0f;
    darr[(idx+2) % 100] = d4 * 2.0;
    
    vol_counter++;  /* Volatile access creates scheduling hazard */
}

int main(int argc, char *argv[]) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(sizeof(int) * 1000);
    float *float_array = (float*)malloc(sizeof(float) * 100);
    double *double_array = (double*)malloc(sizeof(double) * 100);
    struct MixedData *mixed_array = (struct MixedData*)malloc(sizeof(struct MixedData) * 50);
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 1000; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    for (int i = 0; i < 100; i++) {
        float_array[i] = (i * 1103515245) * 0.001f;
        double_array[i] = (i * 1103515245) * 0.000001;
    }
    
    for (int i = 0; i < 50; i++) {
        mixed_array[i].c = i & 0xFF;
        mixed_array[i].i = i * 1000;
        mixed_array[i].f = i * 0.1f;
        mixed_array[i].d = i * 0.01;
        mixed_array[i].arr[0] = i & 0xFF;
        mixed_array[i].arr[1] = (i >> 8) & 0xFF;
        mixed_array[i].arr[2] = (i >> 16) & 0xFF;
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {helper1, helper1, helper1, helper1, helper1};
    
    long long total_sum = 0;
    
    /* Main computation loop with complex control flow */
    for (int i = 1; i < N; i++) {
        int idx = i % 900;  /* Keep within bounds */
        
        /* 1. Pointer chasing with loop-carried dependency */
        int chase_result = pointer_chase(int_array, 1000, idx);
        total_sum += chase_result;
        
        /* 2. Switch statement with multiple basic blocks */
        int switch_result = switch_computation(i, int_array, float_array, double_array);
        total_sum += switch_result;
        
        /* 3. Nested conditional chain */
        int nested_result = nested_conditional(i, int_array, mixed_array);
        total_sum += nested_result;
        
        /* 4. Large basic block with many independent operations */
        if (i % 3 == 0) {
            large_basic_block(int_array, float_array, double_array, mixed_array, idx);
        }
        
        /* 5. Computed jump via function pointer */
        int func_idx = i % 5;
        int func_result = funcs[func_idx](int_array[idx], int_array[idx+1]);
        total_sum += func_result;
        
        /* 6. Mixed data type operations with dependencies */
        double temp_double = double_array[i % 100];
        float temp_float = float_array[i % 100];
        int temp_int = int_array[i];
        
        /* Chain of dependent operations */
        temp_double = temp_double * temp_float + temp_int;
        temp_float = sqrtf(fabsf(temp_float)) * 2.0f;
        temp_int = (temp_int * 3 + (int)temp_double) ^ (int)temp_float;
        
        double_array[i % 100] = temp_double;
        float_array[i % 100] = temp_float;
        int_array[i] = temp_int;
        
        /* 7. Memory barrier to force scheduling constraints */
        asm volatile("" ::: "memory");
        
        /* 8. Volatile access */
        total_sum += vol_counter;
        vol_double = total_sum * 0.000001;
    }
    
    /* Final reduction to prevent dead code elimination */
    long long final_sum = total_sum;
    
    /* Reduce across all arrays */
    for (int i = 0; i < 1000; i++) {
        final_sum ^= int_array[i];
    }
    
    for (int i = 0; i < 100; i++) {
        final_sum += (long long)float_array[i];
        final_sum ^= (long long)double_array[i];
    }
    
    for (int i = 0; i < 50; i++) {
        final_sum += mixed_array[i].i;
        final_sum += (long long)mixed_array[i].f;
        final_sum ^= (long long)mixed_array[i].d;
    }
    
    printf("Result: %lld\n", final_sum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed_array);
    
    return 0;
}
