/* test_scheduler_coverage.c
 * 
 * This program creates complex basic blocks that force GCC's Haifa scheduler
 * to allocate and use the full scheduling context, ensuring the cleanup
 * code in free_sched_block() is executed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_accumulator = 0.0f;
volatile int global_counter = 0;

/* Function with side effects to create scheduling barriers */
static inline ALWAYS_INLINE int side_effect_func(int x) {
    asm volatile("" : "+r" (x) : : "memory");
    return x ^ global_seed;
}

/* Complex integer computation with dependencies */
static ALWAYS_INLINE void test_integer_dep_chain(int *arr, int n) {
    int a = arr[0];
    int b = arr[1];
    int c = arr[2];
    int d = arr[3];
    
    /* Long dependency chain */
    a = side_effect_func(a);
    b = a + side_effect_func(b);
    c = b * side_effect_func(c);
    d = c - side_effect_func(d);
    
    /* More dependencies */
    for (int i = 4; i < n && i < 16; i++) {
        arr[i] = arr[i] + d * i;
        d = side_effect_func(d + arr[i]);
    }
    
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d;
}

/* Mixed integer and floating-point operations */
static ALWAYS_INLINE void test_mixed_operations(float *farr, int *iarr, int n) {
    float f1 = farr[0];
    float f2 = farr[1];
    int i1 = iarr[0];
    int i2 = iarr[1];
    
    /* Interleaved FP and integer ops */
    for (int i = 0; i < n && i < 8; i++) {
        f1 = sinf(f1) * 1.5f + f2;
        i1 = side_effect_func(i1) * 3 + i2;
        f2 = cosf(f2) * 0.5f - f1;
        i2 = side_effect_func(i2) / 2 - i1;
        
        /* Memory operations with potential aliasing */
        farr[i] = f1;
        iarr[i] = i1;
        
        /* More operations to increase block size */
        global_accumulator += f1 + f2;
        global_counter += i1 + i2;
    }
}

/* Vector operations to create many parallel instructions */
static ALWAYS_INLINE void test_vector_ops(v4sf *vf, v4si *vi, int n) {
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4si i1 = {1, 2, 3, 4};
    v4si i2 = {5, 6, 7, 8};
    
    /* Unrolled vector operations */
    for (int j = 0; j < n && j < 12; j++) {
        /* Multiple independent vector operations */
        v1 = v1 * v2 + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
        i1 = i1 + i2 * (v4si){j, j+1, j+2, j+3};
        v2 = v2 - v1 * 0.5f;
        i2 = i2 - i1 / 2;
        
        /* Store results with side effects */
        vf[j] = v1;
        vi[j] = i1;
        
        /* Additional scalar operations */
        float temp = ((float*)&v1)[0] + ((float*)&v1)[1];
        global_accumulator += temp;
    }
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE int test_speculative(int *arr, int n) {
    int result = 0;
    
    /* Complex conditional with many operations */
    for (int i = 0; i < n; i++) {
        int x = arr[i];
        
        /* Long chain of dependent operations before branch */
        x = side_effect_func(x);
        x = x * 3 + 7;
        x = x ^ 0xABCD;
        x = x >> 3;
        x = side_effect_func(x);
        
        /* Conditional that might be speculatively scheduled */
        if (x & 1) {
            result += x * 2;
            arr[i] = x + 1;
        } else {
            result -= x / 2;
            arr[i] = x - 1;
        }
        
        /* More operations after branch */
        result = side_effect_func(result);
        global_counter++;
    }
    
    return result;
}

/* Wide basic block with many independent operations */
static ALWAYS_INLINE void test_wide_block(double *darr, int *iarr, int n) {
    /* Many independent variables to fill ready list */
    double d1 = darr[0], d2 = darr[1], d3 = darr[2], d4 = darr[3];
    double d5 = darr[4], d6 = darr[5], d7 = darr[6], d8 = darr[7];
    
    int i1 = iarr[0], i2 = iarr[1], i3 = iarr[2], i4 = iarr[3];
    int i5 = iarr[4], i6 = iarr[5], i7 = iarr[6], i8 = iarr[7];
    
    /* Many independent operations - scheduler can reorder freely */
    d1 = sin(d1) * 2.0;      i1 = i1 * 3 + 1;
    d2 = cos(d2) * 0.5;      i2 = i2 / 2 - 1;
    d3 = tan(d3) * 1.5;      i3 = i3 << 2;
    d4 = exp(d4 * 0.1);      i4 = i4 >> 1;
    d5 = log(fabs(d5) + 1);  i5 = i5 ^ 0xFF;
    d6 = sqrt(fabs(d6));     i6 = i6 & 0x0F0F;
    d7 = d7 * d7 + 1.0;      i7 = i7 | 0x0101;
    d8 = 1.0 / (d8 + 1.0);   i8 = ~i8;
    
    /* More operations mixing them */
    for (int i = 0; i < n && i < 4; i++) {
        d1 = d1 + d2 * i;    i1 = i1 + i2 * i;
        d3 = d3 - d4 / i;    i3 = i3 - i4 / (i+1);
        d5 = d5 * d6;        i5 = i5 * i6;
        d7 = d7 / d8;        i7 = i7 / (i8 + 1);
        
        /* Store with side effects */
        darr[i*2] = d1 + d3;
        darr[i*2+1] = d5 + d7;
        iarr[i*2] = i1 + i3;
        iarr[i*2+1] = i5 + i7;
    }
    
    /* Final side effect */
    global_accumulator += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
}

/* Main test function with multiple complex basic blocks */
static void run_scheduler_tests(void) {
    const int ARRAY_SIZE = 64;
    int int_array[ARRAY_SIZE];
    float float_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    v4sf vector_float[16];
    v4si vector_int[16];
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_array[i] = (float)int_array[i] / 1000.0f;
        double_array[i] = (double)int_array[i] / 1000.0;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            ((float*)&vector_float[i])[j] = (float)(i * 4 + j) / 10.0f;
            ((int*)&vector_int[i])[j] = i * 4 + j;
        }
    }
    
    int checksum = 0;
    
    /* Test 1: Integer dependency chains */
    for (int iter = 0; iter < 100; iter++) {
        test_integer_dep_chain(int_array, ARRAY_SIZE);
        checksum += int_array[iter % ARRAY_SIZE];
    }
    
    /* Test 2: Mixed operations */
    for (int iter = 0; iter < 50; iter++) {
        test_mixed_operations(float_array, int_array, 16);
        checksum += (int)float_array[iter % 16];
    }
    
    /* Test 3: Vector operations */
    for (int iter = 0; iter < 25; iter++) {
        test_vector_ops(vector_float, vector_int, 12);
        checksum += ((int*)&vector_int[iter % 12])[0];
    }
    
    /* Test 4: Speculative scheduling */
    for (int iter = 0; iter < 10; iter++) {
        checksum += test_speculative(int_array, 32);
    }
    
    /* Test 5: Wide basic blocks */
    for (int iter = 0; iter < 20; iter++) {
        test_wide_block(double_array, int_array, 4);
        checksum += (int)double_array[iter % 8];
    }
    
    /* Use results to prevent dead code elimination */
    printf("Scheduler test checksum: %d\n", checksum);
    printf("Global accumulator: %f\n", (double)global_accumulator);
    printf("Global counter: %d\n", global_counter);
}

/* Additional test functions with different patterns */
static void test_loop_unrolling(void) {
    int array[100];
    float farray[100];
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        array[i] = i;
        farray[i] = i * 0.1f;
    }
    
    /* Manually unrolled loop with dependencies */
    for (int i = 0; i < 96; i += 8) {
        /* Unrolled block with cross-iteration dependencies */
        array[i] = side_effect_func(array[i]) + array[i+1];
        array[i+1] = array[i] * side_effect_func(array[i+2]);
        array[i+2] = array[i+1] - side_effect_func(array[i+3]);
        array[i+3] = array[i+2] / (side_effect_func(array[i+4]) + 1);
        array[i+4] = array[i+3] ^ side_effect_func(array[i+5]);
        array[i+5] = array[i+4] << (side_effect_func(array[i+6]) & 3);
        array[i+6] = array[i+5] >> (side_effect_func(array[i+7]) & 3);
        array[i+7] = array[i+6] | side_effect_func(array[i]);
        
        /* Floating point interleaved */
        farray[i] = sinf(farray[i]) + farray[i+1];
        farray[i+1] = farray[i] * cosf(farray[i+2]);
        farray[i+2] = farray[i+1] - tanf(farray[i+3]);
        farray[i+3] = farray[i+2] / (expf(farray[i+4]) + 1.0f);
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += array[i] + (int)farray[i];
    }
    printf("Unrolled loop checksum: %d\n", sum);
}

static void test_complex_control_flow(void) {
    int data[256];
    float results[256];
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        data[i] = (i * 197 + 53) % 1000;
    }
    
    /* Complex control flow within basic block */
    for (int i = 0; i < 255; i++) {
        int x = data[i];
        float r = 0.0f;
        
        /* Many operations before conditional */
        x = side_effect_func(x);
        x = x * 3 + 7;
        x = x ^ (x >> 4);
        x = side_effect_func(x);
        
        /* Switch-like behavior with computed operations */
        switch (x % 5) {
            case 0: r = sinf(x * 0.01f); break;
            case 1: r = cosf(x * 0.02f); break;
            case 2: r = tanf(x * 0.005f); break;
            case 3: r = logf(fabs(x) + 1.0f); break;
            default: r = sqrtf(fabs(x)); break;
        }
        
        /* More operations after */
        x = side_effect_func(x + (int)r);
        data[i] = x;
        results[i] = r;
        
        global_accumulator += r;
        global_counter += x;
    }
    
    printf("Control flow test completed\n");
}

int main(void) {
    clock_t start = clock();
    
    printf("Starting scheduler coverage tests...\n");
    
    /* Run all tests multiple times */
    for (int run = 0; run < 3; run++) {
        run_scheduler_tests();
        test_loop_unrolling();
        test_complex_control_flow();
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("All tests completed in %.3f seconds\n", elapsed);
    printf("Final global_counter: %d\n", global_counter);
    
    return 0;
}
