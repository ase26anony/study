/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup logic in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that require:
 * 1. Target-specific scheduling hooks activation
 * 2. Frontend state saving for speculative scheduling
 * 3. Large instruction queues and ready lists
 * 4. Complex instruction mixes with dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline)) inline

/* Vector types for creating parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_accumulator = 0.0f;
volatile double global_double_acc = 0.0;

/* Function with side effects to create scheduling barriers */
static ALWAYS_INLINE int side_effect_func(int x) {
    /* Inline assembly creates a scheduling barrier */
    asm volatile("" : "+r" (x) : : "memory");
    return x ^ global_seed;
}

/* Complex integer computation with long dependency chain */
static ALWAYS_INLINE int complex_int_chain(int a, int b, int c, int d, int e) {
    /* Create a long dependency chain */
    int t1 = a + b * c;
    int t2 = t1 - d / (e + 1);
    int t3 = t2 ^ (a << 3);
    int t4 = t3 * t1 - t2;
    int t5 = side_effect_func(t4);
    int t6 = t5 | (b & c);
    int t7 = t6 + (d % (e | 1));
    int t8 = t7 * 3 - t5;
    int t9 = t8 >> (a & 7);
    int t10 = t9 + t4 - t7;
    
    return side_effect_func(t10);
}

/* Mixed integer and floating-point operations */
static ALWAYS_INLINE float mixed_operations(int a, float b, double c, int d) {
    /* Interleave integer and FP operations */
    int i1 = a * d + 7;
    float f1 = b * 3.14f;
    double d1 = c / 2.71828;
    
    i1 = side_effect_func(i1);
    f1 = f1 + sinf(b);
    d1 = d1 * cos(c);
    
    int i2 = i1 ^ (d << 2);
    float f2 = f1 * expf(f1);
    double d2 = d1 + log(fabs(c) + 1.0);
    
    /* More mixing */
    i2 = i2 + (int)(f2 * 100);
    f2 = f2 + (float)(d2 / 10.0);
    
    return f2 + (float)i2 + (float)d2;
}

/* Memory-intensive function with potential aliasing */
static ALWAYS_INLINE void memory_operations(int* arr1, int* arr2, float* farr, 
                                           int size, int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        /* Multiple independent memory operations */
        int idx1 = iter % size;
        int idx2 = (iter * 7) % size;
        int idx3 = (iter * 13) % size;
        
        /* Create scheduling pressure with memory ops */
        arr1[idx1] = arr2[idx2] + arr1[idx3];
        farr[idx1] = sinf(farr[idx2]) * cosf(farr[idx3]);
        
        /* More complex memory pattern */
        int temp = arr1[idx2] * arr2[idx3];
        arr2[idx1] = side_effect_func(temp);
        farr[idx3] = farr[idx1] + farr[idx2];
        
        /* Barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
}

/* Vector operations to create many parallel instructions */
static ALWAYS_INLINE v4sf vector_operations(v4sf a, v4sf b, v4sf c) {
    v4sf r1 = a + b * c;
    v4sf r2 = r1 - a / (b + 1.0f);
    v4sf r3 = r2 * r1 - a;
    v4sf r4 = r3 + b * c;
    v4sf r5 = r4 / (r1 + 2.0f);
    
    /* Mix with scalar operations */
    float s1 = ((float*)&r5)[0] + ((float*)&r5)[1];
    float s2 = ((float*)&r5)[2] * ((float*)&r5)[3];
    
    v4sf r6 = r5 + (v4sf){s1, s2, s1, s2};
    return r6;
}

/* Function with complex control flow and speculative scheduling opportunities */
static ALWAYS_INLINE double speculative_operations(int n, double* data) {
    double result = 0.0;
    
    /* Small loop that might be software pipelined */
    for (int i = 0; i < n && i < 8; i++) {
        /* Dependent operations within loop */
        double x = data[i];
        double y = sin(x * 0.1);
        double z = cos(x * 0.2);
        
        /* Conditional that creates speculative scheduling opportunity */
        if (x > 0.5) {
            result += y * z + exp(x);
        } else {
            result += y / (z + 1.0) + log(fabs(x) + 1.0);
        }
        
        /* More operations with dependencies */
        data[i] = side_effect_func((int)(x * 1000)) / 1000.0;
    }
    
    return result;
}

/* Wide basic block created by loop unrolling */
static ALWAYS_INLINE void wide_basic_block(int* out, const int* in, int n) {
    /* Manually unrolled loop creates wide basic block */
    int i = 0;
    
    /* First unrolled chunk - 16 operations */
    out[i] = in[i] * 3 + 7; i++;
    out[i] = in[i] / 2 - 5; i++;
    out[i] = side_effect_func(in[i]) ^ 0xFF; i++;
    out[i] = in[i] + in[i-1] * 2; i++;
    out[i] = in[i] | (in[i-2] & 0x7F); i++;
    out[i] = in[i] << (in[i-3] & 3); i++;
    out[i] = in[i] - in[i-4] / 3; i++;
    out[i] = side_effect_func(in[i] * in[i-5]); i++;
    
    /* Second unrolled chunk */
    out[i] = in[i] + 11; i++;
    out[i] = in[i] * in[i-1] - 13; i++;
    out[i] = side_effect_func(in[i] >> 2); i++;
    out[i] = in[i] & 0x3F | in[i-2]; i++;
    out[i] = in[i] + (in[i-3] % 17); i++;
    out[i] = in[i] ^ in[i-4] ^ in[i-5]; i++;
    out[i] = side_effect_func(in[i] * 3 - 7); i++;
    out[i] = in[i] + in[i-6] - in[i-7]; i++;
    
    /* Ensure we use all inputs */
    if (i < n) {
        for (; i < n; i++) {
            out[i] = side_effect_func(in[i] + i);
        }
    }
}

/* Main test function 1: Complex arithmetic with dependencies */
void test_function_1(int iterations) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    float f = 1.5f, g = 2.5f;
    double h = 3.14159;
    
    int int_result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple dependent chains */
        a = complex_int_chain(a, b, c, d, e);
        b = side_effect_func(b + c * d);
        c = a ^ b | (d << (e & 3));
        
        /* Mixed operations */
        float_result += mixed_operations(a, f, h, d);
        f = sinf(float_result) + cosf(g);
        
        /* FP chain */
        h = h * 1.01 + double_result * 0.99;
        double_result = speculative_operations(8, &h);
        
        /* Update for next iteration */
        d = (d * 3 + e) % 97;
        e = side_effect_func(e + i);
        g = g * 0.95f + float_result * 0.05f;
    }
    
    global_accumulator += float_result;
    global_double_acc += double_result;
    
    printf("Test 1: int=%d, float=%f, double=%f\n", 
           a, float_result, double_result);
}

/* Main test function 2: Memory and vector operations */
void test_function_2(int size) {
    /* Allocate arrays for memory operations */
    int* arr1 = malloc(size * sizeof(int));
    int* arr2 = malloc(size * sizeof(int));
    float* farr = malloc(size * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        arr1[i] = i;
        arr2[i] = size - i;
        farr[i] = sinf(i * 0.1f);
    }
    
    /* Perform memory-intensive operations */
    memory_operations(arr1, arr2, farr, size, 32);
    
    /* Vector operations */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vec3 = {2.0f, 3.0f, 4.0f, 5.0f};
    
    v4sf vec_result = {0.0f, 0.0f, 0.0f, 0.0f};
    for (int i = 0; i < 16; i++) {
        vec_result = vector_operations(vec1, vec2, vec3);
        vec1 = vec_result * 0.9f + vec1 * 0.1f;
        vec2 = side_effect_func(i) ? vec2 * 1.1f : vec2 * 0.9f;
    }
    
    /* Wide basic block test */
    int* out_arr = malloc(size * sizeof(int));
    wide_basic_block(out_arr, arr1, size > 16 ? 16 : size);
    
    /* Compute checksum */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < size; i++) {
        checksum ^= arr1[i] ^ arr2[i] ^ out_arr[i];
        fchecksum += farr[i];
    }
    
    global_accumulator += fchecksum;
    global_seed = checksum;
    
    printf("Test 2: checksum=%d, fchecksum=%f\n", checksum, fchecksum);
    
    free(arr1);
    free(arr2);
    free(farr);
    free(out_arr);
}

/* Main test function 3: Nested loops with complex control flow */
void test_function_3(int dim) {
    /* Small matrix multiplication with complex operations */
    float mat_a[8][8];
    float mat_b[8][8];
    float mat_c[8][8];
    
    /* Initialize matrices */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            mat_a[i][j] = sinf(i * 0.5f + j * 0.2f);
            mat_b[i][j] = cosf(i * 0.3f - j * 0.4f);
            mat_c[i][j] = 0.0f;
        }
    }
    
    /* Manual unrolling to create wide blocks */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float sum = 0.0f;
            
            /* Unrolled inner loop */
            sum += mat_a[i][0] * mat_b[0][j];
            sum += mat_a[i][1] * mat_b[1][j];
            sum += mat_a[i][2] * mat_b[2][j];
            sum += mat_a[i][3] * mat_b[3][j];
            sum += mat_a[i][4] * mat_b[4][j];
            sum += mat_a[i][5] * mat_b[5][j];
            sum += mat_a[i][6] * mat_b[6][j];
            sum += mat_a[i][7] * mat_b[7][j];
            
            /* Complex operation on result */
            if (sum > 0) {
                mat_c[i][j] = sqrtf(sum) + logf(fabs(sum) + 1.0f);
            } else {
                mat_c[i][j] = expf(sum * 0.1f) - 1.0f;
            }
            
            /* Side effect to prevent optimization */
            mat_c[i][j] = side_effect_func((int)(mat_c[i][j] * 1000)) / 1000.0f;
        }
    }
    
    /* Compute final result */
    float total = 0.0f;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            total += mat_c[i][j] * (i + j);
        }
    }
    
    global_accumulator += total;
    printf("Test 3: matrix total=%f\n", total);
}

/* Main driver function */
int main(int argc, char** argv) {
    int iterations = 100;
    int size = 64;
    
    printf("Starting scheduler coverage test...\n");
    
    /* Run all test functions multiple times */
    for (int run = 0; run < 3; run++) {
        test_function_1(iterations + run * 10);
        test_function_2(size + run * 8);
        test_function_3(8);
        
        /* Update global seed to vary execution */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Final accumulator: %f\n", (double)global_accumulator);
    printf("Final double accumulator: %f\n", global_double_acc);
    printf("Test completed.\n");
    
    return 0;
}
