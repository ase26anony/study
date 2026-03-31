#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to create scheduling hazards */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 0.0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Helper functions with different characteristics */
static int helper1(int a, int b) {
    volatile int barrier;
    barrier = a * b;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return barrier + (a ^ b);
}

static float helper2(float a, float b) {
    float temp = a * b;
    temp = temp / (b + 1.0f);
    asm volatile("" ::: "memory");
    return temp;
}

static double helper3(double a, double b) {
    volatile double v = a;
    for (int i = 0; i < 3; i++) {
        v = v * b + i;
    }
    return v;
}

/* Non-inlineable function (due to complexity) */
__attribute__((noinline)) 
int complex_switch(int val, int *arr, float *farr, double *darr) {
    int result = 0;
    
    /* Deep switch with many cases */
    switch (val % 12) {
        case 0:
            result = arr[val] + arr[val + 1];
            farr[val % 100] = helper2(farr[val % 100], farr[(val + 1) % 100]);
            break;
        case 1:
            result = arr[val] * arr[val - 1];
            darr[val % 50] = helper3(darr[val % 50], darr[(val + 1) % 50]);
            break;
        case 2:
            result = arr[val] ^ arr[val + 2];
            for (int j = 0; j < 4; j++) {
                farr[(val + j) % 100] += j * 0.5f;
            }
            break;
        case 3:
            result = arr[val] | arr[val + 3];
            darr[val % 50] = sqrt(fabs(darr[val % 50]));
            break;
        case 4:
            result = arr[val] & arr[val + 4];
            farr[val % 100] = sinf(farr[val % 100]);
            break;
        case 5:
            result = arr[val] << (val % 8);
            darr[val % 50] = cos(darr[val % 50]);
            break;
        case 6:
            result = arr[val] >> (val % 8);
            farr[val % 100] = tanf(farr[val % 100]);
            break;
        case 7:
            result = ~arr[val];
            darr[val % 50] = log(fabs(darr[val % 50]) + 1.0);
            break;
        case 8:
            result = arr[val] % (val + 1);
            farr[val % 100] = expf(farr[val % 100] * 0.1f);
            break;
        case 9:
            result = -arr[val];
            darr[val % 50] = pow(darr[val % 50], 1.5);
            break;
        case 10:
            result = abs(arr[val]);
            farr[val % 100] = atanf(farr[val % 100]);
            break;
        case 11:
            result = arr[val] / (val % 8 + 1);
            darr[val % 50] = asin(fmin(fabs(darr[val % 50]), 0.99));
            break;
    }
    
    asm volatile("" ::: "memory");
    return result;
}

/* Function with computed goto-like behavior using function pointers */
static int computed_jump_computation(int x, int y, int selector) {
    static compute_func_t funcs[] = {
        (compute_func_t)helper1,
        NULL, /* Will be filled */
        NULL
    };
    
    /* Initialize function pointers */
    funcs[1] = (compute_func_t)complex_switch;
    
    int result = 0;
    if (selector & 1) {
        result = funcs[0](x, y);
    } else {
        /* Create artificial dependency chain */
        int temp = x;
        for (int i = 0; i < 8; i++) {
            temp = temp * 1103515245 + 12345;
            temp = (temp >> 16) & 0x7FFF;
            asm volatile("" : "+r"(temp) : : "memory");
        }
        result = temp;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(N * sizeof(int));
    float *float_array = (float*)malloc(N * sizeof(float));
    double *double_array = (double*)malloc(N * sizeof(double));
    struct MixedData *mixed_array = (struct MixedData*)malloc(N * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        int_array[i] = i * 1103515245;
        float_array[i] = (float)(i * 1103515245) / 1000.0f;
        double_array[i] = (double)(i * 1103515245) / 10000.0;
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = i * 123456789;
        mixed_array[i].d = (double)i / 100.0;
        mixed_array[i].s = (short)(i * 54321);
    }
    
    /* Main computation loop with complex dependencies */
    long long total_sum = 0;
    int next_index = 0;
    
    for (int iteration = 0; iteration < N; iteration++) {
        /* Pointer chasing through int_array */
        int chase_sum = 0;
        int chase_idx = iteration % N;
        for (int chase = 0; chase < 10; chase++) {
            chase_sum += int_array[chase_idx];
            chase_idx = (chase_idx * 1103515245 + 12345) % N;
            asm volatile("" ::: "memory"); /* Barrier */
        }
        
        /* Complex dependency chain */
        int a = int_array[iteration % N];
        int b = int_array[(iteration + 1) % N];
        int c = int_array[(iteration + 2) % N];
        
        /* Long chain of dependent operations */
        int d = a * b + c;
        int e = d ^ (a << 3);
        int f = e * 1103515245;
        int g = f % (b + 1);
        int h = g | (c & 0xFF);
        int i = h * 123456789;
        int j = i >> 4;
        int k = j + (a % 7);
        int l = k * 54321;
        
        /* Mix in floating point operations */
        float fa = float_array[iteration % N];
        float fb = float_array[(iteration + 3) % N];
        float fc = fa * fb + (float)l;
        float_array[iteration % N] = fc;
        
        double da = double_array[iteration % N];
        double db = double_array[(iteration + 5) % N];
        double dc = da * db + (double)chase_sum;
        double_array[iteration % N] = dc;
        
        /* Access packed struct with misaligned accesses */
        struct MixedData *md = &mixed_array[iteration % N];
        int mixed_val = md->i + md->s;
        md->i = mixed_val;
        md->d += (double)mixed_val / 1000.0;
        
        /* Deep conditional chain */
        int cond_result = 0;
        if (iteration & 0x01) {
            cond_result = helper1(a, b);
            if (iteration & 0x02) {
                cond_result += complex_switch(iteration, int_array, float_array, double_array);
                if (iteration & 0x04) {
                    cond_result += computed_jump_computation(a, b, iteration);
                    if (iteration & 0x08) {
                        cond_result *= 2;
                        g_volatile_counter++;
                    }
                }
            }
        } else {
            cond_result = chase_sum;
        }
        
        /* Switch with many cases */
        int switch_result = complex_switch(iteration, int_array, float_array, double_array);
        
        /* Update volatile variables */
        g_volatile_double += (double)cond_result / 1000.0;
        
        /* Final reduction with many dependencies */
        int final_val = l + cond_result + switch_result + mixed_val;
        int_array[iteration % N] = final_val;
        
        total_sum += final_val + (long long)(fc * 100) + (long long)(dc * 100);
        
        /* Occasionally call helper function */
        if (iteration % 7 == 0) {
            float_array[(iteration + 11) % N] = helper2(fa, fb);
        }
        
        /* Create loop-carried dependency */
        next_index = (next_index + final_val) % N;
    }
    
    /* Additional reduction across arrays */
    long long array_sum = 0;
    for (int i = 0; i < N; i++) {
        array_sum += int_array[i];
        array_sum += (long long)(float_array[i] * 100);
        array_sum += (long long)(double_array[i] * 100);
        array_sum += mixed_array[i].i;
    }
    
    total_sum += array_sum;
    total_sum += g_volatile_counter;
    total_sum += (long long)(g_volatile_double * 1000);
    
    printf("Result: %lld\n", total_sum);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(mixed_array);
    
    return 0;
}
