/* test_sched_context.c - Complex instruction sequences to trigger Haifa scheduler cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for SIMD-like operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile functions to prevent optimization */
static volatile int global_counter = 0;

/* Function with side effects - creates scheduling barriers */
static ALWAYS_INLINE int side_effect_func(int x) {
    global_counter += x;
    return x * 2;
}

/* Complex arithmetic with dependencies */
static ALWAYS_INLINE float complex_fp_chain(float a, float b, float c, float d) {
    float t1 = a * b + c;
    float t2 = sinf(t1) * cosf(d);
    float t3 = t2 / (a + 1.0f);
    float t4 = sqrtf(fabsf(t3)) + d;
    return t4 * t1 - t2;
}

/* Integer dependency chain */
static ALWAYS_INLINE int integer_chain(int a, int b, int c, int d, int e) {
    int t1 = a * b + c;
    int t2 = (t1 >> 3) & 0xFF;
    int t3 = t2 * d - e;
    int t4 = t3 ^ (t1 << 2);
    int t5 = (t4 % 127) + t2;
    return t5 * t3 / (t1 + 1);
}

/* Memory-intensive computation with potential aliasing */
static ALWAYS_INLINE void memory_ops(int* restrict arr1, int* restrict arr2, 
                                     int* restrict arr3, int size) {
    for (int i = 0; i < size; i++) {
        arr3[i] = arr1[i] * 2 + arr2[i];
        arr2[i] = arr3[i] - arr1[i];
        arr1[i] = (arr2[i] + arr3[i]) / 3;
    }
}

/* Mixed integer/FP operations */
static ALWAYS_INLINE double mixed_operations(int a, float b, double c, int d) {
    double result = (double)a * (double)b;
    result = result + c * (double)d;
    result = result / (sin((double)a) + 1.0);
    result = result * cos((double)b);
    return result + (double)(a % (d + 1));
}

/* Vector operations using GCC vector extensions */
static ALWAYS_INLINE v4sf vector_ops(v4sf a, v4sf b, v4sf c) {
    v4sf t1 = a * b + c;
    v4sf t2 = __builtin_ia32_sqrtps(t1);
    v4sf t3 = t2 * a - b;
    v4sf t4 = t3 / (c + 1.0f);
    return t1 + t2 + t3 + t4;
}

/* Test function 1: Wide basic block with unrolled loop */
void test_wide_block(int iterations) {
    volatile float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
    volatile int count1 = 0, count2 = 0, count3 = 0;
    
    /* Unrolled computation with multiple independent chains */
    for (int i = 0; i < iterations; i++) {
        /* Chain 1: FP operations */
        float a = (float)i * 1.5f;
        float b = sinf((float)i);
        acc1 += complex_fp_chain(a, b, acc1, acc2);
        
        /* Chain 2: Integer operations */
        int x = i * 3 + 1;
        int y = (i << 2) | 0xF;
        count1 += integer_chain(x, y, count1, count2, count3);
        
        /* Chain 3: Mixed operations */
        double d = mixed_operations(i, a, (double)b, y);
        acc2 += (float)d;
        
        /* Chain 4: More FP */
        acc3 += tanf((float)i) * cosf((float)(i + 1));
        
        /* Chain 5: Side effects */
        count2 += side_effect_func(i);
        
        /* Chain 6: More integer */
        count3 += (x * y) % 256;
        
        /* Chain 7: Another FP chain */
        acc1 += logf(fabsf(a) + 1.0f) * expf(b);
        
        /* Chain 8: Final mixed */
        acc3 += (float)mixed_operations(count1, acc2, d, count3);
    }
    
    /* Prevent dead code elimination */
    printf("Test1: %f %d %f\n", acc1 + acc2 + acc3, count1 + count2 + count3, 
           (float)(global_counter % 1000));
}

/* Test function 2: Complex control flow with speculative scheduling */
void test_speculative_scheduling(int size) {
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* arr3 = (int*)malloc(size * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3 + 1;
        arr3[i] = 0;
    }
    
    /* Complex loop with conditional updates - may trigger speculative scheduling */
    volatile int sum = 0;
    for (int i = 0; i < size; i++) {
        /* Multiple conditionals in a row */
        if (arr1[i] % 3 == 0) {
            arr3[i] = arr1[i] * 2 + arr2[i];
            sum += integer_chain(arr1[i], arr2[i], arr3[i], sum, i);
        } else if (arr1[i] % 5 == 0) {
            arr3[i] = arr1[i] / 2 - arr2[i];
            sum += side_effect_func(arr3[i]);
        } else {
            arr3[i] = arr1[i] + arr2[i];
            float fval = complex_fp_chain((float)arr1[i], (float)arr2[i], 
                                         (float)arr3[i], (float)sum);
            sum += (int)fval;
        }
        
        /* Nested conditionals */
        if (i % 7 == 0) {
            for (int j = 0; j < 4; j++) {
                arr3[i] += j * arr1[i] - arr2[i];
            }
        }
        
        /* Another conditional with computation */
        arr3[i] += (sum % 2 == 0) ? arr1[i] * 3 : arr2[i] * 2;
    }
    
    /* Process results with more complex flow */
    volatile int final_result = 0;
    for (int i = 0; i < size; i += 2) {
        switch (arr3[i] % 4) {
            case 0:
                final_result += arr1[i] * arr2[i];
                break;
            case 1:
                final_result += arr1[i] + arr2[i] * 3;
                break;
            case 2:
                final_result += integer_chain(arr1[i], arr2[i], arr3[i], final_result, i);
                break;
            default:
                final_result += side_effect_func(arr3[i]);
                break;
        }
    }
    
    printf("Test2: %d %d\n", sum, final_result);
    
    free(arr1);
    free(arr2);
    free(arr3);
}

/* Test function 3: Vector operations and large ready lists */
void test_vector_operations(int iterations) {
    v4sf vec_acc = {0.0f, 0.0f, 0.0f, 0.0f};
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vec_c = {0.1f, 0.2f, 0.3f, 0.4f};
    
    /* Many independent vector operations to fill ready list */
    for (int i = 0; i < iterations; i++) {
        /* Multiple independent vector chains */
        v4sf v1 = vector_ops(vec_a, vec_b, vec_c);
        v4sf v2 = vector_ops(vec_b, vec_c, vec_a);
        v4sf v3 = vector_ops(vec_c, vec_a, vec_b);
        v4sf v4 = vector_ops(vec_a + vec_b, vec_b + vec_c, vec_c + vec_a);
        
        /* Mix with scalar operations */
        float s1 = ((float*)&v1)[0] + ((float*)&v2)[1];
        float s2 = ((float*)&v3)[2] * ((float*)&v4)[3];
        float s3 = complex_fp_chain(s1, s2, (float)i, (float)(i * 2));
        
        /* Update vectors */
        vec_acc += v1 + v2 + v3 + v4;
        vec_a += v1 * s3;
        vec_b += v2 / (s1 + 1.0f);
        vec_c += v3 * s2;
        
        /* Integer operations in parallel */
        int i1 = integer_chain(i, (int)s1, (int)s2, (int)s3, global_counter);
        int i2 = side_effect_func(i1);
        global_counter += i2 % 256;
    }
    
    /* Extract results */
    float result[4];
    memcpy(result, &vec_acc, sizeof(result));
    printf("Test3: %f %f %f %f\n", result[0], result[1], result[2], result[3]);
}

/* Test function 4: Software pipelining candidate */
void test_software_pipelining(int size) {
    double* data = (double*)malloc(size * sizeof(double));
    double* output = (double*)malloc(size * sizeof(double));
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        data[i] = sin((double)i * 0.1) * cos((double)i * 0.05);
    }
    
    /* Small loop that might be software pipelined */
    for (int i = 2; i < size - 2; i++) {
        /* Stencil computation with dependencies */
        double d1 = data[i-2] * 0.1;
        double d2 = data[i-1] * 0.2;
        double d3 = data[i] * 0.4;
        double d4 = data[i+1] * 0.2;
        double d5 = data[i+2] * 0.1;
        
        /* Complex FP chain */
        double t1 = d1 + d2 * sin(d3);
        double t2 = d3 * cos(d4) + d5;
        double t3 = t1 / (fabs(t2) + 1.0);
        double t4 = sqrt(fabs(t3)) * tan(d2);
        
        output[i] = t4 + mixed_operations(i, (float)d1, d2, (int)d3);
        
        /* Conditional update */
        if (output[i] > 0.5) {
            data[i] = output[i] * 0.9;
            global_counter += i % 64;
        }
    }
    
    /* Compute checksum */
    volatile double checksum = 0.0;
    for (int i = 0; i < size; i++) {
        checksum += output[i] * (double)i;
    }
    
    printf("Test4: %f %d\n", checksum, global_counter);
    
    free(data);
    free(output);
}

/* Test function 5: Maximum scheduling pressure */
void test_max_pressure(int size) {
    /* Multiple arrays for memory operations */
    float* farr1 = (float*)aligned_alloc(16, size * sizeof(float));
    float* farr2 = (float*)aligned_alloc(16, size * sizeof(float));
    float* farr3 = (float*)aligned_alloc(16, size * sizeof(float));
    int* iarr1 = (int*)malloc(size * sizeof(int));
    int* iarr2 = (int*)malloc(size * sizeof(int));
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        farr1[i] = (float)i * 0.5f;
        farr2[i] = sinf((float)i * 0.1f);
        farr3[i] = 0.0f;
        iarr1[i] = i * 3;
        iarr2[i] = i * 7 + 1;
    }
    
    /* Extremely wide basic block with many independent operations */
    volatile float fsum = 0.0f;
    volatile int isum = 0;
    
    #pragma GCC unroll 8
    for (int i = 0; i < size; i++) {
        /* 8 independent FP chains */
        float f1 = complex_fp_chain(farr1[i], farr2[i], fsum, (float)i);
        float f2 = farr1[i] * farr2[i] + tanf(farr1[i]);
        float f3 = sinf(farr2[i]) * cosf(farr1[i]) / (farr2[i] + 1.0f);
        float f4 = sqrtf(fabsf(f1)) + logf(fabsf(f2) + 1.0f);
        float f5 = f3 * f4 - f1 / (f2 + 1.0f);
        float f6 = expf(f5 * 0.1f) * sinf(f4 * 0.2f);
        float f7 = farr1[i] + farr2[i] + f3 + f4 + f5 + f6;
        float f8 = f7 * 0.5f + complex_fp_chain(f7, f6, f5, f4);
        
        farr3[i] = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;
        fsum += farr3[i];
        
        /* 8 independent integer chains */
        int i1 = integer_chain(iarr1[i], iarr2[i], isum, i, global_counter);
        int i2 = side_effect_func(i1);
        int i3 = (iarr1[i] * iarr2[i]) % 1024;
        int i4 = (i1 << 3) | (i2 & 0xF);
        int i5 = i3 ^ i4 + i1;
        int i6 = (i5 * 7) / (i2 + 1);
        int i7 = mixed_operations(i1, (float)i2, (double)i3, i4) > 0.0 ? i5 : i6;
        int i8 = i7 + i6 + i5 + i4 + i3 + i2 + i1;
        
        iarr2[i] = i8;
        isum += i8;
        
        /* Mixed operations */
        double d1 = mixed_operations(i, (float)i1, (double)f1, i2);
        double d2 = mixed_operations(i1, f2, d1, i3);
        farr1[i] += (float)(d1 + d2);
        
        /* Vector operations every 4th iteration */
        if (i % 4 == 0) {
            v4sf va = {farr1[i], farr2[i], farr3[i], fsum};
            v4sf vb = {(float)i1, (float)i2, (float)i3, (float)i4};
            v4sf vc = vector_ops(va, vb, va + vb);
            farr3[i] += ((float*)&vc)[0] + ((float*)&vc)[1];
        }
    }
    
    printf("Test5: %f %d\n", fsum, isum);
    
    free(farr1);
    free(farr2);
    free(farr3);
    free(iarr1);
    free(iarr2);
}

/* Main driver */
int main(int argc, char** argv) {
    int iterations = 100;
    int size = 256;
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) size = atoi(argv[2]);
    
    printf("Starting scheduler stress tests...\n");
    
    /* Run all tests to trigger different scheduling scenarios */
    test_wide_block(iterations);
    test_speculative_scheduling(size);
    test_vector_operations(iterations);
    test_software_pipelining(size);
    test_max_pressure(size / 2);
    
    printf("All tests completed. Global counter: %d\n", global_counter);
    
    return 0;
}
