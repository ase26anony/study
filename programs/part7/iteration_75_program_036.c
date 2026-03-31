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
#define LINKED_LIST_SIZE 256

/* Mixed data types with different alignments */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    float f;
    char padding[3];
};

/* Volatile variables to create scheduling hazards */
volatile int volatile_counter = 0;
volatile double volatile_double = 0.0;

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that may be inlined or not */
static __attribute__((noinline)) int helper_func1(int a, int b) {
    asm volatile("" ::: "memory");  /* Memory barrier */
    int result = (a * b) + (a ^ b) - (a & b);
    volatile_counter++;
    return result;
}

static __attribute__((noinline)) int helper_func2(int a, int b) {
    asm volatile("" ::: "memory");
    int result = (a << 3) | (b >> 2);
    volatile_double += 0.5;
    return result;
}

static __attribute__((noinline)) double fp_helper(double a, double b) {
    asm volatile("" ::: "memory");
    double result = sin(a) * cos(b) + tan(a + b);
    volatile_counter += 2;
    return result;
}

/* Complex computation kernel with many dependencies */
static int compute_kernel(int *arr, double *darr, float *farr, 
                         struct MixedData *marr, int idx) {
    int result = 0;
    
    /* Create complex dependency chain */
    int a = arr[idx];
    int b = arr[(idx + 1) % ARRAY_SIZE];
    int c = arr[(idx + 2) % ARRAY_SIZE];
    
    /* Long dependency chain */
    int d = a * b + c;
    int e = d ^ (a << 2);
    int f = e * 3 - b;
    int g = f / (c + 1);
    int h = g | (d & e);
    
    /* Floating point mixed with integer */
    double da = darr[idx];
    double db = darr[(idx + 3) % ARRAY_SIZE];
    double dc = fp_helper(da, db);
    
    float fa = farr[idx];
    float fb = farr[(idx + 4) % ARRAY_SIZE];
    float fc = fa * fb - (float)dc;
    
    /* Packed struct access */
    marr[idx].i = h;
    marr[idx].f = fc;
    marr[idx].d = dc;
    
    /* More dependencies */
    result = h + (int)fc + (int)(dc * 100);
    
    /* Conditional with memory barrier */
    if (result & 1) {
        asm volatile("" ::: "memory");
        result = helper_func1(result, idx);
    } else {
        asm volatile("" ::: "memory");
        result = helper_func2(result, idx);
    }
    
    return result;
}

/* Pointer chasing through array simulating linked list */
static int pointer_chase(int *array, int start, int steps) {
    int current = start;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        current = array[current % ARRAY_SIZE];
        sum += current ^ i;
        
        /* Create loop-carried dependency */
        if (i > 0) {
            sum += array[(current + i - 1) % ARRAY_SIZE];
        }
        
        /* Memory barrier every 8 steps */
        if ((i & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return sum;
}

/* Switch-based computation with many cases */
static int switch_computation(int value, int *arr, double *darr) {
    int result = 0;
    
    switch (value % 12) {
        case 0:
            result = arr[value % ARRAY_SIZE] * 3;
            darr[value % ARRAY_SIZE] += result * 0.1;
            break;
        case 1:
            result = arr[value % ARRAY_SIZE] + arr[(value + 1) % ARRAY_SIZE];
            result ^= 0x55AA55AA;
            break;
        case 2:
            result = arr[value % ARRAY_SIZE] << (value & 7);
            darr[value % ARRAY_SIZE] = sin(result);
            break;
        case 3:
            result = pointer_chase(arr, value % ARRAY_SIZE, 16);
            break;
        case 4:
            result = helper_func1(value, arr[value % ARRAY_SIZE]);
            darr[value % ARRAY_SIZE] = cos(result);
            break;
        case 5:
            result = arr[value % ARRAY_SIZE] / (1 + (value & 31));
            result |= 0x80000000;
            break;
        case 6:
            result = arr[value % ARRAY_SIZE] * arr[(value + 2) % ARRAY_SIZE];
            result -= arr[(value + 3) % ARRAY_SIZE];
            break;
        case 7:
            result = (arr[value % ARRAY_SIZE] ^ 0x12345678) * 7;
            darr[value % ARRAY_SIZE] = sqrt(fabs(result));
            break;
        case 8:
            result = helper_func2(value, arr[value % ARRAY_SIZE]);
            break;
        case 9:
            result = arr[value % ARRAY_SIZE] + (value << 3);
            result &= 0x0F0F0F0F;
            break;
        case 10:
            result = arr[value % ARRAY_SIZE] * value;
            result = (result >> 4) | (result << 28);
            break;
        case 11:
            result = arr[value % ARRAY_SIZE] - arr[(value + 4) % ARRAY_SIZE];
            result = abs(result);
            break;
    }
    
    return result;
}

/* Large basic block with many independent instructions */
static void init_arrays(int *arr, double *darr, float *farr, 
                       struct MixedData *marr, int size) {
    for (int i = 0; i < size; i++) {
        /* Many independent stores - fills instruction queue */
        arr[i] = (i * 1103515245) & 0x7FFFFFFF;
        darr[i] = sin(i * 0.01) * 100.0;
        farr[i] = (float)(cos(i * 0.02) * 50.0);
        marr[i].c = (char)(i & 0xFF);
        marr[i].i = i * i;
        marr[i].d = darr[i] * 2.0;
        marr[i].f = farr[i] * 1.5f;
        
        /* Every 32nd iteration, add memory barrier */
        if ((i & 31) == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

/* Deeply nested conditionals */
static int nested_conditional(int a, int b, int c, int *arr) {
    int result = 0;
    
    if (a > 0) {
        if (b > a) {
            if (c > b) {
                result = arr[a % ARRAY_SIZE] + arr[b % ARRAY_SIZE];
            } else if (c == b) {
                result = arr[a % ARRAY_SIZE] * arr[c % ARRAY_SIZE];
            } else {
                result = helper_func1(a, b);
            }
            
            if (result & 1) {
                result ^= arr[c % ARRAY_SIZE];
            } else {
                result |= 0xAAAAAAAA;
            }
        } else if (b == a) {
            result = arr[b % ARRAY_SIZE] << 2;
            
            if (c & 1) {
                result = helper_func2(result, c);
            } else {
                result = pointer_chase(arr, c % ARRAY_SIZE, 8);
            }
        } else {
            result = arr[a % ARRAY_SIZE] - arr[b % ARRAY_SIZE];
            
            for (int i = 0; i < 4; i++) {
                result += arr[(c + i) % ARRAY_SIZE];
            }
        }
    } else if (a == 0) {
        result = b * c;
        
        if (result > 1000) {
            result /= 3;
        } else if (result > 100) {
            result *= 2;
        } else {
            result += 50;
        }
    } else {
        result = abs(a) + abs(b) + abs(c);
        
        switch (result % 5) {
            case 0: result ^= 0xFF00FF00; break;
            case 1: result = result << 3; break;
            case 2: result = result >> 2; break;
            case 3: result = ~result; break;
            case 4: result = result * 7; break;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    struct MixedData *mixed_array = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random values */
    init_arrays(int_array, double_array, float_array, mixed_array, ARRAY_SIZE);
    
    /* Create linked list structure within int_array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i + 1) % ARRAY_SIZE;
    }
    
    int final_result = 0;
    
    /* Main computation loop - complex enough to trigger scheduler state saves */
    for (int iter = 0; iter < N; iter++) {
        int idx = iter % ARRAY_SIZE;
        
        /* 1. Pointer chasing */
        int chase_result = pointer_chase(int_array, idx, 32);
        
        /* 2. Complex computation kernel */
        int kernel_result = compute_kernel(int_array, double_array, float_array, 
                                          mixed_array, idx);
        
        /* 3. Switch-based computation */
        int switch_result = switch_computation(iter, int_array, double_array);
        
        /* 4. Nested conditional */
        int cond_result = nested_conditional(iter, chase_result, kernel_result, int_array);
        
        /* 5. Mixed floating point and integer */
        double fp_val = fp_helper(double_array[idx], double_array[(idx + 5) % ARRAY_SIZE]);
        float_array[idx] = (float)fp_val * 0.5f;
        
        /* 6. Function pointer dispatch (computed jump) */
        compute_func_t funcs[] = {helper_func1, helper_func2};
        int func_idx = iter & 1;
        int func_result = funcs[func_idx](chase_result, switch_result);
        
        /* 7. Update arrays with dependencies */
        int_array[idx] = (chase_result + kernel_result + switch_result + 
                         cond_result + func_result) & 0x7FFFFFFF;
        
        double_array[idx] = sin(int_array[idx] * 0.001) + cos(fp_val);
        
        /* 8. Packed struct operations */
        mixed_array[idx].i = int_array[idx];
        mixed_array[idx].d = double_array[idx];
        mixed_array[idx].f = float_array[idx];
        mixed_array[idx].c = (char)(iter & 0xFF);
        
        /* Accumulate final result */
        final_result ^= int_array[idx];
        final_result += (int)(double_array[idx] * 1000);
        final_result ^= mixed_array[idx].i;
        
        /* Memory barrier every 16 iterations */
        if ((iter & 15) == 0) {
            asm volatile("" ::: "memory");
        }
        
        /* Conditional function call */
        if (iter & 3) {
            volatile_counter += helper_func1(iter, final_result & 0xFF);
        } else {
            volatile_double += fp_helper(double_array[idx], 0.5);
        }
    }
    
    /* Final reduction across arrays */
    int reduction = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        reduction ^= int_array[i];
        reduction += (int)(double_array[i] * 100);
        reduction ^= mixed_array[i].i;
        
        /* Process in chunks with barriers */
        if ((i & 63) == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    final_result ^= reduction;
    final_result += volatile_counter;
    final_result ^= (int)volatile_double;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(mixed_array);
    
    return final_result & 0xFF;
}
