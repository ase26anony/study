/* haifa_scheduler_test.c
 * Program designed to trigger GCC's HAIFA scheduler state save/restore
 * and ensure free_state() is called with non-empty scheduler structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent optimization and create scheduling hazards */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 1.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Small helper functions with different computation patterns */
static int helper1(int a, int b) {
    volatile int barrier;
    barrier = a * b;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return (barrier + (a ^ b)) * 3;
}

static int helper2(int a, int b) {
    int temp = a;
    for (int i = 0; i < 3; i++) {
        temp = (temp << 2) | (b & 3);
        b >>= 2;
    }
    return temp;
}

static double helper3(double a, double b) {
    volatile double result = a;
    for (int i = 0; i < 4; i++) {
        result = result * b + i;
        asm volatile("" ::: "memory");
    }
    return result;
}

/* Non-inlineable function (due to complexity) to create scheduling boundaries */
__attribute__((noinline)) 
int complex_calculation(int *arr, int n, struct PackedData *p) {
    int sum = 0;
    volatile int vsum = 0;
    
    /* Mixed data type operations */
    for (int i = 0; i < n; i++) {
        sum += arr[i] * p->i;
        sum ^= (int)(p->d * 100.0);
        sum += p->c * 2;
        sum -= p->s;
        
        /* Pointer chasing with volatile */
        vsum = sum;
        g_volatile_counter = vsum;
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Update packed struct */
        p->c = (char)(sum & 0xFF);
        p->i = sum ^ 0x12345678;
        p->d = (double)sum / 3.14159;
        p->s = (short)(sum >> 16);
    }
    
    return sum;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int val, int mode) {
    int result = val;
    
    switch (mode % 10) {
        case 0:
            result = result * 3 + 1;
            result ^= 0xAAAAAAAA;
            result = (result << 3) | (result >> 29);
            break;
        case 1:
            result = result + helper1(result, mode);
            result = result * 7 - 13;
            break;
        case 2:
            result = (int)helper3((double)result, (double)mode);
            result &= 0x7FFFFFFF;
            break;
        case 3:
            for (int i = 0; i < 5; i++) {
                result = (result ^ (result >> 1)) * 1103515245 + 12345;
            }
            break;
        case 4:
            result = result * result - result;
            result = result / (mode + 1);
            break;
        case 5:
            result = helper2(result, mode);
            result = ~result;
            break;
        case 6:
            result = (result << mode) | (result >> (32 - mode));
            result += g_volatile_counter;
            break;
        case 7:
            result = result ^ mode ^ (result * mode);
            asm volatile("" ::: "memory");
            break;
        case 8:
            result = (result & 0x55555555) + ((result >> 1) & 0x55555555);
            result = (result & 0x33333333) + ((result >> 2) & 0x33333333);
            result = (result & 0x0F0F0F0F) + ((result >> 4) & 0x0F0F0F0F);
            break;
        case 9:
            result = result + (mode << 16) + (mode >> 16);
            result = result * g_volatile_counter;
            break;
    }
    
    return result;
}

/* Main computation with complex control flow */
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
    
    /* Packed struct array */
    struct PackedData *packed_array = 
        (struct PackedData*)malloc(N * sizeof(struct PackedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        int_array[i] = i * 1103515245 + 12345;
        double_array[i] = (double)int_array[i] / 1073741824.0;
        float_array[i] = (float)(sin((double)i) * 100.0);
        
        packed_array[i].c = (char)(int_array[i] & 0xFF);
        packed_array[i].i = int_array[i];
        packed_array[i].d = double_array[i];
        packed_array[i].s = (short)((int_array[i] >> 16) & 0xFFFF);
    }
    
    /* Array of function pointers for computed jumps */
    ComputeFunc funcs[] = {helper1, helper2, NULL};
    
    int result = 0;
    int chain_result = 0;
    double fp_result = 1.0;
    
    /* Main computation loop with complex dependencies */
    for (int i = 0; i < N; i++) {
        /* Pointer chasing through arrays */
        int idx = i;
        int chase_sum = 0;
        for (int j = 0; j < 10; j++) {
            idx = (idx * 13 + 7) % N;
            chase_sum += int_array[idx];
            chase_sum ^= (int)(double_array[idx] * 1000.0);
            
            /* Volatile access creates scheduling hazard */
            g_volatile_double = double_array[idx];
            asm volatile("" ::: "memory");
        }
        
        /* Chain of dependent arithmetic operations */
        int a = chase_sum;
        int b = int_array[i % N];
        double c = double_array[(i + 1) % N];
        float d = float_array[(i + 2) % N];
        
        a = a * b + (int)(c * 100.0);
        b = a ^ (int)(d * 50.0f);
        c = (double)a / (double)(b + 1);
        d = (float)(c * 2.0);
        a = (int)(d * 3.0f) + b;
        
        chain_result += a;
        
        /* Complex conditional with function calls */
        if (i & 1) {
            /* Call helper function */
            chain_result = helper1(chain_result, i);
            
            /* Use function pointer */
            if (funcs[0]) {
                chain_result = funcs[0](chain_result, int_array[i % N]);
            }
        } else if (i & 2) {
            /* Different computation path */
            fp_result = helper3(fp_result, double_array[i % N]);
            chain_result += (int)fp_result;
        } else {
            /* Yet another path */
            chain_result = switch_computation(chain_result, i);
        }
        
        /* Nested loop with loop-carried dependency */
        int inner_sum = 0;
        for (int k = 0; k < 5; k++) {
            inner_sum = inner_sum * 3 + int_array[(i + k) % N];
            if (k & 1) {
                inner_sum ^= packed_array[(i + k) % N].i;
            } else {
                inner_sum += packed_array[(i + k) % N].s;
            }
        }
        
        /* Large basic block with many independent operations */
        int temp = inner_sum;
        temp = temp * 7 - 13;
        temp = temp ^ 0xDEADBEEF;
        temp = (temp << 5) | (temp >> 27);
        temp += g_volatile_counter;
        temp = temp * 3 + 1;
        temp ^= int_array[(i + 3) % N];
        temp = temp & 0x7FFFFFFF;
        temp = temp / ((i % 32) + 1);
        temp += (int)(double_array[(i + 4) % N] * 100.0);
        temp -= packed_array[(i + 5) % N].c;
        temp = temp * 11;
        
        /* Mix results */
        result ^= chain_result;
        result += temp;
        result = switch_computation(result, i);
        
        /* Periodically call complex function */
        if ((i % 100) == 99) {
            result += complex_calculation(int_array, 50, &packed_array[i % N]);
        }
        
        /* Update volatile for scheduling hazards */
        g_volatile_counter = result & 0xFF;
        asm volatile("" ::: "memory");
    }
    
    /* Final reduction across arrays */
    int final_sum = result;
    for (int i = 0; i < N; i += 1 + (N / 100)) {
        final_sum ^= int_array[i];
        final_sum += (int)(double_array[i] * 1000.0);
        final_sum -= packed_array[i].i;
        final_sum = (final_sum << 1) | (final_sum >> 31);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (N=%d)\n", final_sum, N);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(packed_array);
    
    return final_sum & 0xFF;
}
