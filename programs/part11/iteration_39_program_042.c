/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that require extensive instruction scheduling.
 * 
 * Compilation flags to trigger the target:
 *   gcc -O3 -fschedule-insns -fschedule-insns2 -mtune=generic -march=x86-64 \
 *       -fno-omit-frame-pointer -funroll-loops -fno-vect-cost-model \
 *       test_scheduler_coverage.c -o test_scheduler_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Prevent optimization of critical computations */
#define NOINLINE __attribute__((noinline))
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for SIMD-like operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent constant propagation */
volatile int global_seed = 42;
volatile float global_float_seed = 3.14159f;
volatile double global_double_seed = 2.71828;

/* Function with inline assembly to create scheduling barriers */
static inline void memory_barrier() {
    __asm__ volatile ("" ::: "memory");
}

/* Complex integer computation with dependency chains */
NOINLINE static int complex_integer_chain(int iterations) {
    int a = global_seed;
    int b = a + 1;
    int c = b * 2;
    int d = c - a;
    int e = d << 3;
    int f = e >> 1;
    int g = f ^ b;
    int h = g | c;
    int i = h & 0xFFFF;
    int j = i % 257;
    
    /* Create a long dependency chain */
    for (int k = 0; k < iterations; k++) {
        a = b + c;
        b = c * d;
        c = d - e;
        d = e ^ f;
        e = f | g;
        f = g & h;
        g = h << 1;
        h = i >> 2;
        i = j * 3;
        j = (a + b + c + d + e + f + g + h + i) % 1000;
    }
    
    /* Multiple independent chains to fill ready list */
    int x = a * 2, y = b * 3, z = c * 4;
    int p = d * 5, q = e * 6, r = f * 7;
    
    /* Mix them together */
    return (x + y + z + p + q + r + j) & 0xFF;
}

/* Mixed integer and floating-point operations */
NOINLINE static float mixed_operations(int count) {
    float f1 = global_float_seed;
    float f2 = f1 * 2.0f;
    float f3 = f2 / 1.5f;
    double d1 = global_double_seed;
    double d2 = d1 * 3.0;
    double d3 = d2 / 2.5;
    
    int i1 = global_seed;
    int i2 = i1 + 100;
    int i3 = i2 * 2;
    
    /* Interleaved FP and integer operations */
    for (int i = 0; i < count; i++) {
        /* FP operations */
        f1 = sinf(f2) + cosf(f3);
        f2 = f1 * f3 - tanf(f1);
        f3 = f2 / (f1 + 1.0f);
        
        /* Integer operations */
        i1 = (i1 * 1103515245 + 12345) & 0x7fffffff;
        i2 = i1 ^ (i1 >> 16);
        i3 = i2 * 3 + 1;
        
        /* Mixed type operations */
        f1 += (float)i1 * 0.001f;
        f2 -= (float)i2 * 0.0001f;
        f3 *= 1.0f + (float)i3 * 0.00001f;
        
        /* More FP operations */
        d1 = sin(d2) * cos(d3);
        d2 = d1 + exp(d3);
        d3 = log(fabs(d2) + 1.0);
        
        memory_barrier(); /* Prevent reordering across barrier */
    }
    
    return f1 + f2 + f3 + (float)d1 + (float)d2 + (float)d3;
}

/* Function with vector operations to create wide basic blocks */
NOINLINE static v4si vector_operations(v4si a, v4si b, v4si c) {
    v4si result = {0, 0, 0, 0};
    
    /* Multiple independent vector operations */
    v4si t1 = a + b;
    v4si t2 = b * c;
    v4si t3 = a - c;
    v4si t4 = t1 & t2;
    v4si t5 = t2 | t3;
    v4si t6 = t3 ^ t4;
    v4si t7 = t4 << 1;
    v4si t8 = t5 >> 2;
    
    /* Create dependency chains */
    for (int i = 0; i < 4; i++) {
        t1[i] = t1[i] * 2 + t2[i];
        t2[i] = t2[i] - t3[i] * t4[i];
        t3[i] = t3[i] ^ (t5[i] | t6[i]);
        t4[i] = t4[i] + (t7[i] << (i + 1));
        t5[i] = t5[i] * 3 - t8[i];
    }
    
    /* Combine results */
    result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
    
    /* Scalar operations mixed with vector */
    int scalar_sum = 0;
    for (int i = 0; i < 4; i++) {
        scalar_sum += result[i];
        result[i] = result[i] * (scalar_sum % 256);
    }
    
    return result;
}

/* Function with complex control flow and speculative scheduling */
NOINLINE static double speculative_computation(int size, double* data) {
    double sum = 0.0;
    double product = 1.0;
    double max_val = -1e100;
    double min_val = 1e100;
    
    /* Loop with small iteration count for potential software pipelining */
    for (int i = 0; i < size; i++) {
        double x = data[i];
        
        /* Complex conditional updates */
        if (x > 0.5) {
            sum += sin(x) * cos(x);
            product *= 1.0 + fabs(tan(x));
        } else if (x < -0.5) {
            sum -= exp(-x * x);
            product /= 1.0 + log(fabs(x) + 1.0);
        } else {
            sum += x * x * x;
            product *= 1.0 + x * x;
        }
        
        /* More conditionals for speculative scheduling */
        if (x > max_val) {
            max_val = x;
        } else if (x < min_val) {
            min_val = x;
        }
        
        /* Nested conditionals */
        if (i % 3 == 0) {
            sum += 0.1;
        } else if (i % 3 == 1) {
            product *= 1.01;
        } else {
            max_val += 0.001;
            min_val -= 0.001;
        }
        
        /* Memory operations with potential aliasing */
        if (i > 0) {
            data[i] = data[i-1] * 0.9 + x * 0.1;
        }
    }
    
    /* Final computation with branches */
    double result;
    if (sum > product) {
        result = sum / (product + 1.0);
    } else {
        result = product / (sum + 1.0);
    }
    
    if (max_val - min_val > 10.0) {
        result *= 2.0;
    } else {
        result /= 2.0;
    }
    
    return result;
}

/* Wide basic block with unrolled loop */
NOINLINE static void wide_basic_block(int* array, int size) {
    /* Unrolled loop creates many instructions */
    for (int i = 0; i < size; i += 8) {
        /* Eight independent chains per iteration */
        int a0 = array[i] * 2 + 1;
        int b0 = array[i] ^ 0xAA;
        int c0 = array[i] << 3;
        int d0 = array[i] >> 1;
        
        int a1 = array[i+1] * 3 - 1;
        int b1 = array[i+1] ^ 0x55;
        int c1 = array[i+1] << 2;
        int d1 = array[i+1] >> 2;
        
        int a2 = array[i+2] * 5 + 2;
        int b2 = array[i+2] ^ 0x33;
        int c2 = array[i+2] << 1;
        int d2 = array[i+2] >> 3;
        
        int a3 = array[i+3] * 7 - 3;
        int b3 = array[i+3] ^ 0xCC;
        int c3 = array[i+3] << 4;
        int d3 = array[i+3] >> 4;
        
        int a4 = array[i+4] * 11 + 4;
        int b4 = array[i+4] ^ 0xF0;
        int c4 = array[i+4] << 2;
        int d4 = array[i+4] >> 2;
        
        int a5 = array[i+5] * 13 - 5;
        int b5 = array[i+5] ^ 0x0F;
        int c5 = array[i+5] << 3;
        int d5 = array[i+5] >> 1;
        
        int a6 = array[i+6] * 17 + 6;
        int b6 = array[i+6] ^ 0xFF;
        int c6 = array[i+6] << 1;
        int d6 = array[i+6] >> 3;
        
        int a7 = array[i+7] * 19 - 7;
        int b7 = array[i+7] ^ 0xAA;
        int c7 = array[i+7] << 2;
        int d7 = array[i+7] >> 2;
        
        /* Cross-dependencies between chains */
        array[i] = a0 + b1 + c2 + d3;
        array[i+1] = a1 + b2 + c3 + d4;
        array[i+2] = a2 + b3 + c4 + d5;
        array[i+3] = a3 + b4 + c5 + d6;
        array[i+4] = a4 + b5 + c6 + d7;
        array[i+5] = a5 + b6 + c7 + d0;
        array[i+6] = a6 + b7 + c0 + d1;
        array[i+7] = a7 + b0 + c1 + d2;
        
        /* More operations to increase instruction count */
        array[i] ^= array[i+1];
        array[i+1] ^= array[i+2];
        array[i+2] ^= array[i+3];
        array[i+3] ^= array[i+4];
        array[i+4] ^= array[i+5];
        array[i+5] ^= array[i+6];
        array[i+6] ^= array[i+7];
        array[i+7] ^= array[i];
    }
}

/* Function with function calls and side effects */
NOINLINE static int function_call_mix(int x) {
    /* Multiple function calls with side effects */
    int a = complex_integer_chain(x % 10);
    float b = mixed_operations(x % 8 + 2);
    
    /* Inline small functions */
    static ALWAYS_INLINE int helper1(int y) {
        return (y * 3 + 1) ^ 0x1234;
    }
    
    static ALWAYS_INLINE int helper2(int y) {
        return (y << 2) | 0xAA;
    }
    
    static ALWAYS_INLINE float helper3(float y) {
        return y * 1.5f - 0.25f;
    }
    
    /* Call inline functions multiple times */
    int c = helper1(x);
    int d = helper2(c);
    int e = helper1(d);
    int f = helper2(e);
    
    float g = helper3(b);
    float h = helper3(g);
    float i = helper3(h);
    
    /* Memory operations */
    int* ptr = (int*)malloc(sizeof(int) * 4);
    if (ptr) {
        ptr[0] = a;
        ptr[1] = c;
        ptr[2] = e;
        ptr[3] = (int)(g + h + i);
        
        int result = ptr[0] + ptr[1] - ptr[2] * ptr[3];
        free(ptr);
        return result;
    }
    
    return a + c + e + (int)(g + h + i);
}

/* Main driver function */
int main() {
    clock_t start = clock();
    int checksum = 0;
    
    printf("Starting scheduler coverage test...\n");
    
    /* Test 1: Complex integer chains */
    printf("Test 1: Complex integer chains\n");
    for (int i = 0; i < 100; i++) {
        checksum ^= complex_integer_chain(i % 5 + 3);
    }
    
    /* Test 2: Mixed operations */
    printf("Test 2: Mixed integer/FP operations\n");
    float float_sum = 0.0f;
    for (int i = 0; i < 50; i++) {
        float_sum += mixed_operations(i % 4 + 2);
    }
    checksum ^= (int)(float_sum * 1000);
    
    /* Test 3: Vector operations */
    printf("Test 3: Vector operations\n");
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_result = {0, 0, 0, 0};
    
    for (int i = 0; i < 20; i++) {
        vec_result = vector_operations(vec_a, vec_b, vec_c);
        vec_a = vec_b;
        vec_b = vec_c;
        vec_c = vec_result;
    }
    
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
    }
    
    /* Test 4: Speculative computation */
    printf("Test 4: Speculative computation\n");
    double* data = (double*)malloc(sizeof(double) * 100);
    for (int i = 0; i < 100; i++) {
        data[i] = sin(i * 0.1) * cos(i * 0.05);
    }
    
    double spec_result = 0.0;
    for (int i = 0; i < 10; i++) {
        spec_result += speculative_computation(100, data);
    }
    checksum ^= (int)(spec_result * 10000);
    free(data);
    
    /* Test 5: Wide basic blocks */
    printf("Test 5: Wide basic blocks\n");
    int* array = (int*)malloc(sizeof(int) * 1024);
    for (int i = 0; i < 1024; i++) {
        array[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < 5; i++) {
        wide_basic_block(array, 1024);
    }
    
    for (int i = 0; i < 1024; i += 64) {
        checksum += array[i];
    }
    free(array);
    
    /* Test 6: Function call mix */
    printf("Test 6: Function call mix\n");
    for (int i = 0; i < 100; i++) {
        checksum += function_call_mix(i);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nAll tests completed.\n");
    printf("Final checksum: %d\n", checksum);
    printf("Elapsed time: %.3f seconds\n", elapsed);
    printf("\nCompile with: gcc -O3 -fschedule-insns -fschedule-insns2 ");
    printf("-mtune=generic -march=x86-64 -fno-omit-frame-pointer ");
    printf("-funroll-loops -fno-vect-cost-model test_scheduler_coverage.c\n");
    
    return checksum != 0 ? 0 : 1;
}
