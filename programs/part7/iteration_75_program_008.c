/* haifa_sched_trigger.c
 * Program designed to trigger GCC's HAIFA scheduler state save/restore
 * and ensure free_state() is called with populated scheduler context.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent reordering optimizations */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) packed_data {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that create scheduling boundaries */
static int helper1(int a, int b) {
    volatile int barrier;
    barrier = a * b;
    asm volatile("" ::: "memory");
    return barrier + (a ^ b);
}

static int helper2(int a, int b) {
    volatile int barrier;
    barrier = a / (b | 1);
    asm volatile("" ::: "memory");
    return barrier - (a & b);
}

static double helper3(double a, double b) {
    volatile double barrier;
    barrier = a * b;
    asm volatile("" ::: "memory");
    return barrier + sin(a) * cos(b);
}

/* Non-inlineable function (due to complexity) to force scheduling boundaries */
__attribute__((noinline)) 
int complex_calculation(int *arr, double *darr, int idx, int mod) {
    int result = 0;
    double dresult = 0.0;
    
    /* Mixed data type operations */
    for (int i = 0; i < 8; i++) {
        result += arr[(idx + i) % 256] * (mod + i);
        dresult += darr[(idx + i) % 256] * (mod * 0.1 + i);
        asm volatile("" ::: "memory"); /* Compiler barrier */
    }
    
    /* Volatile store to force memory ordering */
    g_volatile_counter = result;
    g_volatile_double = dresult;
    
    return result + (int)dresult;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_kernel(int value, int *arr, double *darr) {
    int result = 0;
    
    switch (value % 10) {
        case 0:
            result = arr[value] * 3 + arr[value + 1];
            darr[value] = result * 0.5;
            break;
        case 1:
            result = arr[value] - arr[value * 2 % 256];
            darr[value] = sqrt(fabs(result));
            break;
        case 2:
            result = helper1(arr[value], arr[value + 1]);
            darr[value] = helper3(darr[value], darr[value + 1]);
            break;
        case 3:
            for (int i = 0; i < 4; i++) {
                result += arr[(value + i * 7) % 256];
            }
            darr[value] = result * 0.25;
            break;
        case 4:
            result = arr[value] * arr[value + 1] - arr[value + 2];
            darr[value] = result / 3.14159;
            break;
        case 5:
            result = helper2(arr[value], arr[value + 3]);
            darr[value] = darr[value] * 2.0 - 1.0;
            break;
        case 6:
            result = (arr[value] << 3) | (arr[value] >> 5);
            darr[value] = fmod(darr[value], 100.0);
            break;
        case 7:
            result = complex_calculation(arr, darr, value, 7);
            break;
        case 8:
            result = arr[value] ^ arr[value + 1] ^ arr[value + 2];
            darr[value] = pow(darr[value], 1.5);
            break;
        case 9:
            result = arr[value] % (arr[value + 1] + 1);
            darr[value] = log(fabs(darr[value]) + 1.0);
            break;
    }
    
    return result;
}

/* Pointer chasing through array (simulated linked list) */
static int pointer_chase(int *arr, int start, int steps) {
    int idx = start;
    int sum = 0;
    
    for (int i = 0; i < steps; i++) {
        sum += arr[idx];
        idx = arr[idx] % 256; /* Next "node" */
        asm volatile("" ::: "memory"); /* Barrier prevents optimization */
    }
    
    return sum;
}

/* Large basic block with many independent operations */
static void large_basic_block(int *arr, double *darr, struct packed_data *pdata, 
                              int base, int iterations) {
    /* Many independent operations to fill instruction queue */
    for (int i = 0; i < iterations; i++) {
        int idx = (base + i) % 256;
        
        /* Independent arithmetic operations */
        int a = arr[idx] * 3;
        int b = arr[idx + 1] + 7;
        int c = a - b;
        int d = c * arr[idx + 2];
        int e = d >> 2;
        int f = e | arr[idx + 3];
        
        /* Independent FP operations */
        double da = darr[idx] * 1.5;
        double db = darr[idx + 1] + 2.5;
        double dc = da - db;
        double dd = dc * darr[idx + 2];
        double de = dd / 3.0;
        double df = de + darr[idx + 3];
        
        /* Packed struct accesses (misaligned) */
        pdata[idx].c = (char)(f & 0xFF);
        pdata[idx].i = f;
        pdata[idx].d = df;
        pdata[idx].s = (short)(f >> 8);
        
        /* Store results back */
        arr[idx] = f;
        darr[idx] = df;
    }
}

int main(int argc, char **argv) {
    int N = 1000; /* Default iteration count */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(256 * sizeof(int));
    double *double_array = (double*)malloc(256 * sizeof(double));
    float *float_array = (float*)malloc(256 * sizeof(float));
    struct packed_data *packed_array = 
        (struct packed_data*)malloc(256 * sizeof(struct packed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        double_array[i] = (double)(int_array[i]) / 1000.0;
        float_array[i] = (float)(int_array[i]) / 500.0f;
        packed_array[i].c = (char)(i & 0xFF);
        packed_array[i].i = int_array[i];
        packed_array[i].d = double_array[i];
        packed_array[i].s = (short)(i & 0xFFFF);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {helper1, helper2};
    
    int final_result = 0;
    double final_double = 0.0;
    
    /* Primary computation loop with complex control flow */
    for (int iter = 0; iter < N; iter++) {
        int idx = iter % 256;
        
        /* Deeply nested conditional chain */
        if (iter & 1) {
            if (iter & 2) {
                if (iter & 4) {
                    if (iter & 8) {
                        /* Pointer chasing with loop-carried dependency */
                        int chase_result = pointer_chase(int_array, idx, 16);
                        final_result ^= chase_result;
                    } else {
                        /* Chain of dependent arithmetic operations */
                        int a = int_array[idx] * 3;
                        int b = a + int_array[idx + 1];
                        int c = b / (int_array[idx + 2] | 1);
                        int d = c - int_array[idx + 3];
                        int e = d * 7;
                        int f = e >> 2;
                        final_result += f;
                    }
                } else {
                    /* Call helper function in conditional branch */
                    final_result += helper1(int_array[idx], int_array[idx + 1]);
                }
            } else {
                /* Computed jump via function pointer */
                int func_idx = (iter >> 2) & 1;
                final_result += funcs[func_idx](int_array[idx], int_array[idx + 2]);
            }
        } else {
            /* Switch statement with multiple basic blocks */
            final_result += switch_kernel(iter, int_array, double_array);
        }
        
        /* Large basic block every 8 iterations to fill instruction queue */
        if ((iter & 7) == 0) {
            large_basic_block(int_array, double_array, packed_array, idx, 32);
        }
        
        /* Mixed integer/float calculations */
        if (iter & 16) {
            double temp = double_array[idx] * 2.0;
            temp = temp + sin(double_array[idx + 1]);
            temp = temp * cos(double_array[idx + 2]);
            final_double += temp;
            
            /* Convert back to int with barrier */
            asm volatile("" ::: "memory");
            final_result += (int)temp;
        }
        
        /* Memory barrier every 32 iterations */
        if ((iter & 31) == 0) {
            asm volatile("" ::: "memory");
            g_volatile_counter = final_result;
            g_volatile_double = final_double;
        }
        
        /* Data-dependent loop bound */
        int inner_loop = (int_array[idx] % 8) + 1;
        for (int j = 0; j < inner_loop; j++) {
            int_array[(idx + j) % 256] += j * final_result;
            double_array[(idx + j) % 256] += j * final_double;
        }
    }
    
    /* Reduction across all arrays */
    int reduction = 0;
    double double_reduction = 0.0;
    
    for (int i = 0; i < 256; i++) {
        reduction ^= int_array[i];
        reduction += packed_array[i].i;
        double_reduction += double_array[i];
        double_reduction += float_array[i];
    }
    
    final_result ^= reduction;
    final_double += double_reduction;
    
    /* Final volatile stores to prevent dead code elimination */
    g_volatile_counter = final_result;
    g_volatile_double = final_double;
    
    /* Print result to prevent optimization */
    printf("Result: %d (double: %f)\n", final_result, final_double);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(packed_array);
    
    return 0;
}
