/* haifa_sched_coverage.c
 * Program designed to trigger GCC's HAIFA scheduler state save/restore
 * and ensure free_state() is called with populated data structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Volatile variables to prevent optimization and create scheduling hazards */
volatile int g_volatile_counter = 0;
volatile int g_volatile_switch = 0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    char c2;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions with different characteristics */
static int helper1(int a, int b) {
    volatile int barrier;
    barrier = a * b;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return barrier + (a ^ b);
}

static int helper2(int a, int b) {
    volatile int result = 0;
    for (int i = 0; i < 4; i++) {
        result += (a << i) | (b >> i);
    }
    asm volatile("" ::: "memory");
    return result;
}

static double helper3(double a, double b) {
    volatile double tmp = a;
    for (int i = 0; i < 3; i++) {
        tmp = tmp * b + i;
    }
    return tmp;
}

/* Non-inlineable function (due to complexity) to create scheduling boundaries */
__attribute__((noinline)) 
int complex_calculation(int *arr, int n, int seed) {
    volatile int sum = seed;
    struct PackedData pd;
    
    /* Mixed data type operations */
    pd.c = (char)(seed & 0xFF);
    pd.i = seed * 1103515245;
    pd.d = (double)seed * 3.14159;
    pd.c2 = (char)((seed >> 8) & 0xFF);
    
    /* Pointer chasing with dependencies */
    int *ptr = arr;
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr = arr + ((*ptr) % n);  /* Data-dependent pointer chase */
        if (ptr < arr || ptr >= arr + n) {
            ptr = arr;
        }
    }
    
    /* Chain of dependent operations */
    double dsum = (double)sum;
    dsum = dsum * pd.d;
    dsum = helper3(dsum, pd.d * 2.0);
    dsum = dsum / (pd.i + 1);
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    return (int)dsum + pd.c + pd.c2;
}

int main(int argc, char *argv[]) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(N * sizeof(int));
    double *double_array = (double*)malloc(N * sizeof(double));
    float *float_array = (float*)malloc(N * sizeof(float));
    struct PackedData *packed_array = (struct PackedData*)malloc(N * sizeof(struct PackedData));
    
    if (!int_array || !double_array || !float_array || !packed_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        int_array[i] = i * 1103515245;
        double_array[i] = (double)i * 3.14159;
        float_array[i] = (float)i * 2.71828f;
        packed_array[i].c = (char)(i & 0xFF);
        packed_array[i].i = int_array[i];
        packed_array[i].d = double_array[i];
        packed_array[i].c2 = (char)((i >> 8) & 0xFF);
    }
    
    /* Function pointer array for computed jumps */
    compute_func_t funcs[] = {helper1, helper2, helper1, helper2, helper1};
    
    /* Main computation loop with complex control flow */
    int result = 0;
    double dresult = 0.0;
    
    for (int outer = 0; outer < 100; outer++) {
        g_volatile_counter = outer;
        
        /* Large basic block with many independent operations */
        for (int i = 0; i < N; i++) {
            /* Independent array initializations to fill instruction queue */
            int_array[i] += i;
            double_array[i] *= 1.0001;
            float_array[i] -= 0.5f;
            packed_array[i].c ^= (char)i;
            
            /* Memory barrier every 16 iterations */
            if ((i & 0xF) == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Nested loops with loop-carried dependencies */
        int carry = 0;
        for (int i = 1; i < N; i++) {
            /* Data-dependent computation chain */
            carry = int_array[i] * int_array[i-1] + carry;
            int_array[i] = carry & 0x7FFFFFFF;
            
            /* Mixed floating-point operations */
            dresult += double_array[i] * float_array[i];
            
            /* Volatile access */
            g_volatile_switch = i & 0xFF;
        }
        
        /* Deeply nested conditional chain */
        int temp = int_array[outer % N];
        if (temp & 0x01) {
            temp = complex_calculation(int_array, N / 10, temp);
            if (temp > 1000) {
                dresult += helper3((double)temp, 2.0);
                if (temp & 0x02) {
                    temp = helper1(temp, int_array[(temp + 1) % N]);
                } else {
                    temp = helper2(temp, int_array[(temp + 2) % N]);
                }
            } else if (temp < -1000) {
                dresult -= helper3((double)(-temp), 3.0);
            } else {
                /* Switch statement with many cases */
                switch (temp % 10) {
                    case 0: temp = int_array[0] + int_array[1]; break;
                    case 1: temp = int_array[0] - int_array[1]; break;
                    case 2: temp = int_array[0] * int_array[1]; break;
                    case 3: temp = int_array[0] & int_array[1]; break;
                    case 4: temp = int_array[0] | int_array[1]; break;
                    case 5: temp = int_array[0] ^ int_array[1]; break;
                    case 6: temp = int_array[0] << (int_array[1] & 0x3); break;
                    case 7: temp = int_array[0] >> (int_array[1] & 0x3); break;
                    case 8: temp = ~int_array[0]; break;
                    case 9: temp = -int_array[0]; break;
                }
            }
        } else if (temp & 0x04) {
            /* Computed goto via function pointer */
            int idx = (temp >> 3) % 5;
            temp = funcs[idx](temp, int_array[(temp + 3) % N]);
        } else if (temp & 0x08) {
            /* Pointer chasing with mixed types */
            struct PackedData *pd_ptr = packed_array;
            for (int j = 0; j < 20; j++) {
                temp += pd_ptr->i + (int)pd_ptr->d;
                pd_ptr = packed_array + ((pd_ptr->i + j) % N);
            }
        } else {
            /* Default: reduction operation */
            for (int j = 0; j < N; j += 8) {
                temp ^= int_array[j];
                dresult += double_array[j];
            }
        }
        
        result += temp;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Final reduction across all data */
    int64_t final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += int_array[i];
        final_sum ^= (int64_t)(double_array[i] * 1000.0);
        final_sum += packed_array[i].c + packed_array[i].c2;
    }
    
    final_sum += (int64_t)result;
    final_sum += (int64_t)dresult;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld\n", (long)final_sum);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(packed_array);
    
    return 0;
}
