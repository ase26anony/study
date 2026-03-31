/* test_sched_context.c
 * 
 * This program is designed to exercise GCC's Haifa instruction scheduler,
 * specifically targeting the cleanup logic in free_sched_block().
 * It creates complex basic blocks that force the scheduler to allocate
 * and use the full scheduling context, including target-specific hooks,
 * frontend state saving, large instruction queues, and ready lists.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with complex dependency chains and mixed operations */
static ALWAYS_INLINE double complex_math_chain(double a, double b, double c, 
                                               double d, double e, double f) {
    /* Long dependency chain with mixed operations */
    double t1 = a * b + c;
    double t2 = sin(t1) * cos(d);
    double t3 = t2 / (e + 1.0);
    double t4 = t3 * t3 - f;
    double t5 = sqrt(fabs(t4));
    double t6 = t5 + log(fabs(t1) + 1.0);
    double t7 = t6 * exp(-t2);
    return t7;
}

/* Function with SIMD operations using GCC vector extensions */
static ALWAYS_INLINE v4si vector_operations(v4si a, v4si b, v4si c, v4si d) {
    /* Multiple independent vector operations */
    v4si r1 = a * b + c;
    v4si r2 = b * c - d;
    v4si r3 = r1 & r2;
    v4si r4 = r1 | r2;
    v4si r5 = r3 ^ r4;
    v4si r6 = r5 << 2;
    v4si r7 = r6 >> 1;
    return r1 + r2 + r3 + r4 + r5 + r6 + r7;
}

/* Function with memory aliasing and pointer chasing */
static ALWAYS_INLINE int memory_aliasing_chain(int* arr1, int* arr2, 
                                               int* arr3, int size) {
    int sum = 0;
    /* Complex memory access pattern with potential aliasing */
    for (int i = 0; i < size; i++) {
        arr1[i] = arr2[i] * arr3[i];
        sum += arr1[i];
        arr2[i] = arr1[i] + arr3[size - i - 1];
        sum -= arr2[i];
        arr3[i] = arr1[i] * arr2[i];
        sum ^= arr3[i];
    }
    return sum;
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE double speculative_math(double x, int iterations) {
    double result = x;
    /* Loop with conditional updates - creates speculative scheduling opportunities */
    for (int i = 0; i < iterations; i++) {
        if (result > 0.0) {
            result = result * 0.99 - 0.01;
        } else {
            result = result * 1.01 + 0.01;
        }
        
        /* Mixed operations to use different execution units */
        result = sin(result) * cos(result);
        result = result + (double)i * 0.001;
        
        /* Branch with computation in both paths */
        if (i % 3 == 0) {
            result = sqrt(fabs(result));
        } else if (i % 3 == 1) {
            result = log(fabs(result) + 1.0);
        } else {
            result = exp(result * 0.1);
        }
    }
    return result;
}

/* Function with wide basic block from unrolled loop */
static ALWAYS_INLINE void wide_basic_block(float* input, float* output, int n) {
    /* Manually unrolled loop to create wide basic block */
    for (int i = 0; i < n; i += 8) {
        /* 8 independent chains of operations */
        output[i] = input[i] * 2.0f - input[i+1];
        output[i+1] = input[i+1] * 3.0f + input[i];
        output[i+2] = input[i+2] / 2.0f - input[i+3];
        output[i+3] = input[i+3] * 1.5f + input[i+2];
        output[i+4] = sqrtf(fabsf(input[i+4])) * input[i+5];
        output[i+5] = sinf(input[i+5]) * cosf(input[i+4]);
        output[i+6] = input[i+6] * input[i+7] - input[i];
        output[i+7] = input[i+7] / input[i+6] + input[i+1];
        
        /* Cross dependencies */
        output[i] += output[i+7] * 0.1f;
        output[i+1] -= output[i+6] * 0.2f;
        output[i+2] *= output[i+5] * 0.3f;
        output[i+3] /= output[i+4] * 0.4f + 1.0f;
    }
}

/* Function with function calls creating scheduling barriers */
static ALWAYS_INLINE double function_call_barrier(double x) {
    /* External function calls act as scheduling barriers */
    double r1 = sin(x);
    double r2 = cos(x);
    double r3 = tan(x);
    
    /* Computation between calls */
    double t1 = r1 * r2 + r3;
    double t2 = exp(t1);
    
    double r4 = atan(t2);
    double r5 = asin(r4 * 0.5);
    
    /* More computation */
    double t3 = r4 * r5 - t1;
    double t4 = log(fabs(t3) + 1.0);
    
    double r6 = sinh(t4);
    double r7 = cosh(r6);
    
    return r7 * t4 - r5;
}

/* Test 1: Complex arithmetic with dependency chains */
void test_complex_chains() {
    printf("Test 1: Complex Arithmetic Chains\n");
    
    double a = 1.234, b = 2.345, c = 3.456, d = 4.567, e = 5.678, f = 6.789;
    double result = 0.0;
    
    /* Multiple chains to create scheduling pressure */
    for (int i = 0; i < 100; i++) {
        result += complex_math_chain(a, b, c, d, e, f);
        a += 0.1; b += 0.2; c += 0.3; d += 0.4; e += 0.5; f += 0.6;
        
        /* Interleave integer operations */
        int i1 = (int)a * (int)b;
        int i2 = i1 + (int)c;
        int i3 = i2 - (int)d;
        result += (double)(i1 * i2 * i3) * 0.001;
    }
    
    printf("  Result: %f\n", result);
}

/* Test 2: Vector operations with SIMD */
void test_vector_ops() {
    printf("Test 2: Vector/SIMD Operations\n");
    
    v4si va = {1, 2, 3, 4};
    v4si vb = {5, 6, 7, 8};
    v4si vc = {9, 10, 11, 12};
    v4si vd = {13, 14, 15, 16};
    v4si result = {0, 0, 0, 0};
    
    /* Multiple vector operations to fill instruction queue */
    for (int i = 0; i < 50; i++) {
        result += vector_operations(va, vb, vc, vd);
        va += vb;
        vb += vc;
        vc += vd;
        vd += result;
        
        /* Mix with scalar operations */
        int* rp = (int*)&result;
        for (int j = 0; j < 4; j++) {
            rp[j] = rp[j] * 2 - rp[(j+1)%4];
        }
    }
    
    int* rp = (int*)&result;
    printf("  Result: [%d, %d, %d, %d]\n", rp[0], rp[1], rp[2], rp[3]);
}

/* Test 3: Memory operations with aliasing */
void test_memory_aliasing() {
    printf("Test 3: Memory Operations with Aliasing\n");
    
    const int SIZE = 128;
    int* arr1 = malloc(SIZE * sizeof(int));
    int* arr2 = malloc(SIZE * sizeof(int));
    int* arr3 = malloc(SIZE * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    int sum = 0;
    /* Multiple passes with aliasing */
    for (int pass = 0; pass < 10; pass++) {
        sum += memory_aliasing_chain(arr1, arr2, arr3, SIZE);
        
        /* Additional computation between memory ops */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] = (arr1[i] * 13 + 17) % 7919;
            arr2[i] = (arr2[i] * 17 + 13) % 7919;
            arr3[i] = (arr3[i] * 19 + 23) % 7919;
        }
    }
    
    printf("  Checksum: %d\n", sum);
    
    free(arr1);
    free(arr2);
    free(arr3);
}

/* Test 4: Speculative scheduling with branches */
void test_speculative_scheduling() {
    printf("Test 4: Speculative Scheduling\n");
    
    double results[4];
    
    /* Multiple independent speculative chains */
    for (int i = 0; i < 4; i++) {
        results[i] = speculative_math(1.0 + i * 0.5, 8);
    }
    
    /* Combine results with more branches */
    double final_result = 0.0;
    for (int i = 0; i < 4; i++) {
        if (results[i] > 0.0) {
            final_result += sqrt(results[i]);
        } else {
            final_result -= sqrt(fabs(results[i]));
        }
        
        /* Nested conditionals */
        if (i % 2 == 0) {
            final_result *= 1.1;
        } else {
            final_result /= 1.1;
        }
    }
    
    printf("  Final result: %f\n", final_result);
}

/* Test 5: Wide basic block from unrolled operations */
void test_wide_basic_block() {
    printf("Test 5: Wide Basic Block\n");
    
    const int N = 1024;
    float* input = malloc(N * sizeof(float));
    float* output = malloc(N * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        input[i] = sinf(i * 0.1f) * 100.0f;
    }
    
    /* Process in wide basic blocks */
    for (int iter = 0; iter < 5; iter++) {
        wide_basic_block(input, output, N);
        
        /* Swap and process again */
        float* temp = input;
        input = output;
        output = temp;
        
        /* Additional scalar operations */
        float sum = 0.0f;
        for (int i = 0; i < N; i++) {
            sum += input[i];
            output[i] = input[i] - sum * 0.001f;
        }
    }
    
    /* Compute checksum */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += output[i];
    }
    
    printf("  Checksum: %f\n", checksum);
    
    free(input);
    free(output);
}

/* Test 6: Function call barriers */
void test_function_call_barriers() {
    printf("Test 6: Function Call Barriers\n");
    
    double x = 0.1;
    double result = 0.0;
    
    /* Chain of function calls with computations */
    for (int i = 0; i < 20; i++) {
        result += function_call_barrier(x);
        x += 0.05;
        
        /* Inline assembly as additional barrier */
        asm volatile("" : "+r"(result) : : "memory");
        
        /* More math between barriers */
        result = result * 0.95 + 0.05 * sin(result);
    }
    
    printf("  Result: %f\n", result);
}

/* Test 7: Mixed everything - maximum scheduling pressure */
void test_mixed_max_pressure() {
    printf("Test 7: Mixed Operations - Maximum Pressure\n");
    
    /* Integer computations */
    int int_sum = 0;
    for (int i = 0; i < 100; i++) {
        int a = i * 3;
        int b = i * 5;
        int c = i * 7;
        int d = i * 11;
        
        /* Dependency chain */
        int t1 = a * b + c;
        int t2 = t1 - d;
        int t3 = t2 * a;
        int t4 = t3 / (b + 1);
        int_sum += t1 + t2 + t3 + t4;
        
        /* Floating point interleaved */
        double f1 = sin((double)i);
        double f2 = cos((double)t1);
        double f3 = f1 * f2 + (double)t2;
        int_sum += (int)(f3 * 1000.0);
        
        /* Memory operations */
        int* ptr = &int_sum;
        *ptr = *ptr * 13 + 17;
        
        /* Conditional with both paths having computations */
        if (i % 7 == 0) {
            int_sum = int_sum * 2 - i;
        } else if (i % 7 == 1) {
            int_sum = int_sum / 2 + i;
        } else {
            int_sum = int_sum ^ i;
        }
    }
    
    printf("  Integer checksum: %d\n", int_sum);
    
    /* Floating point computations */
    double fp_sum = 0.0;
    double arr[8] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8};
    
    for (int i = 0; i < 50; i++) {
        /* Unrolled FP operations */
        fp_sum += arr[0] * arr[1] - arr[2];
        fp_sum += arr[1] / arr[3] + arr[4];
        fp_sum += sin(arr[2]) * cos(arr[5]);
        fp_sum += exp(arr[3]) * log(fabs(arr[6]) + 1.0);
        fp_sum += sqrt(arr[4]) * tan(arr[7]);
        
        /* Update array with dependency */
        for (int j = 0; j < 8; j++) {
            arr[j] = arr[j] * 0.9 + fp_sum * 0.01;
        }
    }
    
    printf("  FP checksum: %f\n", fp_sum);
}

int main() {
    printf("=== Testing Haifa Scheduler Cleanup Logic ===\n\n");
    
    /* Run all tests to exercise different scheduling scenarios */
    test_complex_chains();
    test_vector_ops();
    test_memory_aliasing();
    test_speculative_scheduling();
    test_wide_basic_block();
    test_function_call_barriers();
    test_mixed_max_pressure();
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
