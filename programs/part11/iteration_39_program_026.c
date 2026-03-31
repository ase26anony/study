/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that require extensive instruction scheduling.
 * 
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -mtune=generic -march=x86-64 -fno-omit-frame-pointer -o test test_scheduler_coverage.c
 * 
 * For ARM targets: gcc -O2 -fschedule-insns -fschedule-insns2 -mcpu=cortex-a57 -mfpu=neon -o test_arm test_scheduler_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Prevent compiler from optimizing away computations */
static volatile int sink;

/* Inline functions to increase instruction count in basic blocks */
static inline int compute_int(int a, int b, int c) {
    return (a * b) + (c << 3) - (a / (b + 1)) + (b ^ c);
}

static inline float compute_float(float a, float b, float c) {
    return (a * b) + (c / (a + 1.0f)) - sinf(b) * cosf(c);
}

/* Function with complex dependency chains and mixed operations */
void test_function_1(int n, int *result) {
    int i, j;
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    float fa = 1.5f, fb = 2.5f, fc = 3.5f, fd = 4.5f;
    
    /* Wide basic block with multiple independent computation paths */
    for (i = 0; i < n; i++) {
        /* Integer dependency chain 1 */
        a = b + c;
        d = a * e;
        e = d - b;
        c = e >> 2;
        b = c ^ a;
        
        /* Integer dependency chain 2 (independent) */
        int x = i * 7;
        int y = x + 11;
        int z = y / 3;
        int w = z << 1;
        
        /* Floating-point dependency chain */
        fa = fb * fc;
        fd = fa / (fb + 1.0f);
        fc = sinf(fd) * 2.0f;
        fb = fc - fa;
        
        /* Mixed operations */
        result[i] = (a + d) * (x - w) + (int)(fa * 100);
        
        /* Memory operations with potential aliasing */
        if (i > 0) {
            result[i] += result[i-1] * 2;
        }
    }
    
    /* Conditional update creating scheduling barrier */
    if (n > 100) {
        for (j = 0; j < 10; j++) {
            result[j] *= 2;
            sink = result[j]; /* Volatile store prevents elimination */
        }
    }
}

/* Function with unrolled loops and vector-like operations */
void test_function_2(float *array, int size) {
    int i;
    /* Create wide basic block through loop unrolling */
    for (i = 0; i < size - 3; i += 4) {
        /* Unrolled operations creating many independent instructions */
        float t0 = array[i] * 1.1f;
        float t1 = array[i+1] * 2.2f;
        float t2 = array[i+2] * 3.3f;
        float t3 = array[i+3] * 4.4f;
        
        t0 = t0 + sinf(t0);
        t1 = t1 + cosf(t1);
        t2 = t2 + sqrtf(fabsf(t2));
        t3 = t3 + expf(t3 * 0.1f);
        
        array[i] = t0 * t1;
        array[i+1] = t1 - t2;
        array[i+2] = t2 / (t3 + 0.001f);
        array[i+3] = t3 * t0;
        
        /* Additional integer computations */
        int idx = i * 2;
        idx = (idx * 3) / 2;
        idx = idx ^ (i << 2);
        sink = idx; /* Prevent elimination */
    }
    
    /* Small inner loop that might trigger software pipelining */
    for (int j = 0; j < 8; j++) {
        float acc = 0.0f;
        for (int k = 0; k < 4; k++) {
            acc += array[j + k] * (k + 1);
        }
        array[j] = acc;
        sink = (int)acc;
    }
}

/* Function with complex control flow and speculative scheduling opportunities */
int test_function_3(int *data, int n) {
    int sum = 0;
    int i = 0;
    
    /* Complex basic block ending with conditional branch */
    while (i < n) {
        int val = data[i];
        
        /* Multiple dependent operations */
        int x = val * 3;
        int y = x + 7;
        int z = y >> 2;
        int w = z ^ val;
        
        /* Conditional computation that might be speculatively scheduled */
        if (val > 100) {
            w = w * 2;
            x = x / 2;
            y = y + 100;
        } else if (val < 50) {
            w = w - 5;
            x = x * 3;
            y = y >> 1;
        } else {
            w = w + 10;
            x = x - 3;
            y = y * 2;
        }
        
        /* More operations after conditionals */
        int result = (w + x) * y;
        result = result - (val % 7);
        result = result << (val & 3);
        
        sum += result;
        
        /* Function call with side effects (inline asm) */
        asm volatile ("" : : "r"(result) : "memory");
        
        i++;
        
        /* Additional independent computations to fill ready list */
        if (i % 4 == 0) {
            int temp = sum * i;
            temp = temp / (n + 1);
            sink = temp;
        }
    }
    
    /* Switch statement requiring state tracking */
    switch (sum % 5) {
        case 0: return sum * 2;
        case 1: return sum / 2;
        case 2: return sum + 100;
        case 3: return sum - 50;
        default: return sum ^ 0xFF;
    }
}

/* Function using GCC vector extensions to create many parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

void test_function_4(v4si *vec_int, v4sf *vec_float, int count) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fc = {9.0f, 10.0f, 11.0f, 12.0f};
    
    for (int i = 0; i < count; i++) {
        /* Vector operations that expand to multiple instructions */
        v4si r1 = a * b + c;
        v4si r2 = b << (a & 3);
        v4si r3 = r1 - r2;
        
        v4sf fr1 = fa * fb + fc;
        v4sf fr2 = fb / (fa + 1.0f);
        v4sf fr3 = fr1 - fr2;
        
        /* Mixed vector and scalar operations */
        vec_int[i] = r3 + (a >> 1);
        vec_float[i] = fr3 * 0.5f;
        
        /* Update vectors for next iteration */
        a = a + 1;
        b = b * 2;
        c = c - 1;
        
        fa = fa * 1.1f;
        fb = fb * 0.9f;
        fc = fc + 0.5f;
        
        /* Memory barrier */
        asm volatile ("" : : : "memory");
    }
}

/* Main driver that calls all test functions */
int main() {
    const int SIZE = 256;
    int *int_data = malloc(SIZE * sizeof(int));
    float *float_data = malloc(SIZE * sizeof(float));
    v4si *vec_int_data = malloc(SIZE/4 * sizeof(v4si));
    v4sf *vec_float_data = malloc(SIZE/4 * sizeof(v4sf));
    
    /* Initialize data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = rand() % 200;
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    printf("Starting scheduler coverage test...\n");
    
    /* Call test functions multiple times to ensure scheduling happens */
    for (int iter = 0; iter < 10; iter++) {
        test_function_1(SIZE, int_data);
        test_function_2(float_data, SIZE);
        int result = test_function_3(int_data, SIZE);
        
        if (iter % 3 == 0) {
            test_function_4(vec_int_data, vec_float_data, SIZE/4);
        }
        
        /* Compute checksum to prevent dead code elimination */
        int checksum = 0;
        for (int i = 0; i < SIZE; i++) {
            checksum ^= int_data[i];
            checksum += (int)(float_data[i] * 100);
        }
        
        printf("Iteration %d, checksum: %d, result: %d\n", 
               iter, checksum, result);
    }
    
    free(int_data);
    free(float_data);
    free(vec_int_data);
    free(vec_float_data);
    
    printf("Test completed.\n");
    return 0;
}
