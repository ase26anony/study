/* test_sched_context.c
 * 
 * This program creates complex basic blocks that force GCC's Haifa scheduler
 * to allocate and use the full scheduling context, ensuring the cleanup
 * code in free_sched_block() is executed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_accum = 0.0f;

/* ======================================================================
 * TEST 1: Complex dependency chains with mixed operations
 * ====================================================================== */
static ALWAYS_INLINE float test1_compute(float a, float b, float c, float d) {
    /* Long dependency chain with mixed operations */
    float t1 = a * b + c;
    float t2 = t1 / (d + 1.0f);
    float t3 = sinf(t2) * cosf(t1);
    float t4 = t3 * t3 - t2 * t2;
    float t5 = sqrtf(fabsf(t4)) + t1;
    float t6 = t5 * 2.71828f - logf(fabsf(t5) + 1.0f);
    
    /* Integer operations interleaved */
    int i1 = (int)(t6 * 1000);
    int i2 = i1 * 37 + 12345;
    int i3 = i2 ^ (i1 << 3);
    float t7 = t6 + (i3 % 100) * 0.01f;
    
    return t7;
}

void test1_large_block(int iterations) {
    float a = 1.234f, b = 5.678f, c = 9.012f, d = 3.456f;
    float result = 0.0f;
    
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < iterations; i++) {
        /* Multiple independent chains */
        float r1 = test1_compute(a, b, c, d);
        float r2 = test1_compute(b, c, d, a);
        float r3 = test1_compute(c, d, a, b);
        float r4 = test1_compute(d, a, b, c);
        
        /* Cross-dependent operations */
        a = r1 * r2 - r3;
        b = r2 + r3 * r4;
        c = r3 - r4 / r1;
        d = r4 * 2.0f + r1;
        
        result += a + b + c + d;
        
        /* Conditional that might trigger speculative scheduling */
        if (result > 1000.0f) {
            result *= 0.99f;
        }
    }
    
    global_accum += result;
    printf("Test1 result: %f\n", result);
}

/* ======================================================================
 * TEST 2: Vector operations and memory aliasing
 * ====================================================================== */
void test2_vector_ops(int size) {
    /* Use vector types to create parallel operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    
    v4sf fvec_a = {1.1f, 2.2f, 3.3f, 4.4f};
    v4sf fvec_b = {5.5f, 6.6f, 7.7f, 8.8f};
    
    /* Array with potential aliasing */
    int* array1 = (int*)malloc(size * sizeof(int));
    int* array2 = array1 + size/2; /* Create aliasing possibility */
    
    for (int i = 0; i < size; i++) {
        array1[i] = i * global_seed;
    }
    
    /* Complex memory access pattern */
    int sum = 0;
    for (int i = 0; i < size - 4; i++) {
        /* Vector load/store operations */
        vec_a = vec_a + vec_b * vec_c;
        vec_b = vec_b - vec_c;
        vec_c = vec_c + (v4si){1, 1, 1, 1};
        
        /* Mixed float/int vector ops */
        fvec_a = fvec_a * fvec_b + (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
        fvec_b = fvec_b - fvec_a * 0.1f;
        
        /* Memory operations that might alias */
        array1[i] = vec_a[0] + vec_b[1];
        array2[i-2] = vec_c[2] * array1[i];
        
        /* Dependency chain with memory */
        sum += array1[i] * array2[i-2] - i;
        
        /* Function call with side effects (prevents reordering) */
        sum += rand() % 100;
    }
    
    /* Final computation with all values */
    float final_result = 0.0f;
    for (int i = 0; i < 4; i++) {
        final_result += fvec_a[i] + vec_a[i];
    }
    final_result += sum;
    
    global_accum += final_result;
    printf("Test2 result: %f\n", final_result);
    
    free(array1);
}

/* ======================================================================
 * TEST 3: Loop with software pipelining opportunities
 * ====================================================================== */
void test3_software_pipeline(int n, float* input, float* output) {
    /* Small loop that might be software pipelined */
    for (int i = 2; i < n - 2; i++) {
        /* 5-stage dependency chain */
        float a = input[i-2] * 0.1f;
        float b = input[i-1] * 0.2f + a;
        float c = input[i] * 0.3f + b;
        float d = input[i+1] * 0.4f + c;
        float e = input[i+2] * 0.5f + d;
        
        /* Additional operations to increase ILP */
        float f = sinf(a) * cosf(b);
        float g = sqrtf(fabsf(c)) + d;
        float h = e * f - g;
        
        /* Conditional update - might trigger state saving */
        if (h > 0.0f) {
            output[i] = h * 0.9f;
        } else {
            output[i] = h * 1.1f;
        }
        
        /* Cross-iteration dependency */
        input[i] += output[i-1] * 0.01f;
    }
}

/* ======================================================================
 * TEST 4: Switch statement with complex basic blocks
 * ====================================================================== */
float test4_switch_block(int mode, float x, float y) {
    float result = 0.0f;
    
    /* Switch creates multiple basic blocks needing scheduling */
    switch (mode % 5) {
        case 0: {
            /* FP intensive block */
            for (int i = 0; i < 8; i++) {
                x = sinf(x) * cosf(y);
                y = tanf(x + y) * 0.5f;
                result += x * y;
            }
            break;
        }
        case 1: {
            /* Integer intensive block */
            int ix = (int)(x * 1000);
            int iy = (int)(y * 1000);
            for (int i = 0; i < 12; i++) {
                ix = ix * 37 + iy;
                iy = iy * 73 - ix;
                result += (ix % 100) * 0.01f;
            }
            break;
        }
        case 2: {
            /* Mixed operations */
            result = x * y;
            for (int i = 0; i < 6; i++) {
                result = sqrtf(fabsf(result)) + x;
                x = logf(fabsf(x) + 1.0f);
                y = expf(y * 0.5f);
            }
            break;
        }
        case 3: {
            /* Memory intensive */
            float temp[16];
            for (int i = 0; i < 16; i++) {
                temp[i] = x * i + y;
            }
            for (int i = 0; i < 15; i++) {
                temp[i] = temp[i] * temp[i+1] - temp[i];
            }
            for (int i = 0; i < 16; i++) {
                result += temp[i];
            }
            break;
        }
        default: {
            /* All operations combined */
            result = test1_compute(x, y, x*y, x/y);
            result += test1_compute(y, x, y*x, y/x);
            break;
        }
    }
    
    return result;
}

/* ======================================================================
 * TEST 5: Inline assembly creating scheduling barriers
 * ====================================================================== */
void test5_asm_barriers(int count) {
    int a = 1, b = 2, c = 3, d = 4;
    float fa = 1.0f, fb = 2.0f, fc = 3.0f;
    
    for (int i = 0; i < count; i++) {
        /* Inline assembly acts as scheduling barrier */
        __asm__ volatile (
            "nop\n\t"
            "nop\n\t"
            : : : "memory"
        );
        
        /* Complex dependency chains around asm */
        a = a * b + c;
        b = b * c - d;
        c = c * d + a;
        d = d * a - b;
        
        fa = fa * fb + sinf(fc);
        fb = fb * fc - cosf(fa);
        fc = fc * fa + tanf(fb);
        
        /* Another asm barrier */
        __asm__ volatile (
            "nop\n\t"
            : : : "memory"
        );
        
        /* More operations */
        a = (a ^ b) | (c & d);
        b = (b << 2) ^ (c >> 1);
        
        fa = sqrtf(fa*fb + fc);
        fb = logf(fabsf(fb) + 1.0f);
    }
    
    global_accum += a + b + fa + fb;
}

/* ======================================================================
 * Main driver function
 * ====================================================================== */
int main() {
    clock_t start = clock();
    
    printf("Starting scheduling context tests...\n");
    
    /* Seed RNG for reproducible results */
    srand(global_seed);
    
    /* Test 1: Large basic blocks with dependency chains */
    printf("\n=== Test 1: Dependency chains ===\n");
    for (int i = 0; i < 3; i++) {
        test1_large_block(50 + i * 20);
    }
    
    /* Test 2: Vector operations */
    printf("\n=== Test 2: Vector operations ===\n");
    test2_vector_ops(256);
    
    /* Test 3: Software pipelining */
    printf("\n=== Test 3: Software pipelining ===\n");
    float* input = (float*)malloc(1024 * sizeof(float));
    float* output = (float*)malloc(1024 * sizeof(float));
    for (int i = 0; i < 1024; i++) {
        input[i] = (float)i / 1024.0f;
    }
    for (int i = 0; i < 5; i++) {
        test3_software_pipeline(1024, input, output);
    }
    
    /* Test 4: Switch with multiple blocks */
    printf("\n=== Test 4: Switch blocks ===\n");
    float switch_result = 0.0f;
    for (int i = 0; i < 100; i++) {
        switch_result += test4_switch_block(i, 1.0f + i*0.1f, 2.0f + i*0.2f);
    }
    printf("Switch test result: %f\n", switch_result);
    global_accum += switch_result;
    
    /* Test 5: Assembly barriers */
    printf("\n=== Test 5: Assembly barriers ===\n");
    test5_asm_barriers(1000);
    
    /* Final computation to use all results */
    printf("\n=== Final results ===\n");
    printf("Global accumulator: %f\n", global_accum);
    
    /* Verify results aren't optimized away */
    if (global_accum > 0.0f) {
        printf("All tests completed successfully.\n");
    } else {
        printf("Warning: Results may have been optimized away.\n");
    }
    
    free(input);
    free(output);
    
    clock_t end = clock();
    printf("Total time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    return 0;
}
