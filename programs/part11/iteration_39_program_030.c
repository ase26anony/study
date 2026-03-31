/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that force the scheduler to allocate and use the full
 * scheduling context, including target-specific hooks, frontend state
 * saving, large instruction queues, and ready lists.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

/* Prevent excessive optimization of our test computations */
#define NO_OPTIMIZE __attribute__((optimize("O0")))

/* Vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Inline functions to increase instruction count in basic blocks */
static inline int compute_hash(int a, int b, int c) {
    return (a * 31 + b) * 31 + c;
}

static inline float fp_mix(float a, float b, float c) {
    return a * b + c / (a + 1.0f);
}

static inline double complex_fp(double x, double y, int iter) {
    double result = x;
    for (int i = 0; i < iter; i++) {
        result = sin(result) * cos(y) + tan(x * y) * 0.1;
    }
    return result;
}

/* Function with wide basic block (unrolled operations) */
NO_OPTIMIZE
void wide_basic_block(int *input, int *output, int size) {
    /* This creates a wide basic block with many independent operations */
    int temp[16];
    
    /* Multiple independent computation paths */
    temp[0] = input[0] + input[1] * 2;
    temp[1] = input[2] - input[3] / 3;
    temp[2] = input[4] * input[5] + 7;
    temp[3] = input[6] << 2 | input[7];
    temp[4] = compute_hash(input[8], input[9], input[10]);
    temp[5] = input[11] ^ input[12] & input[13];
    temp[6] = input[14] * 3 - input[15];
    temp[7] = (input[16] + input[17]) * (input[18] - input[19]);
    
    /* More operations to increase instruction count */
    for (int i = 0; i < 8; i++) {
        temp[i + 8] = temp[i] * (i + 1) - input[i];
    }
    
    /* Conditional updates create scheduling complexity */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        if (temp[i] > 0) {
            sum += temp[i] * 2;
        } else {
            sum -= temp[i] / 2;
        }
    }
    
    /* Memory operations with potential aliasing */
    output[0] = sum;
    for (int i = 1; i < size && i < 16; i++) {
        output[i] = output[i-1] + temp[i-1];
    }
}

/* Function mixing integer and floating point operations */
NO_OPTIMIZE
float mixed_operations(int n, float *a, float *b, int *mask) {
    float result = 0.0f;
    
    /* Create dependency chains with mixed operations */
    for (int i = 0; i < n; i++) {
        /* Integer operations */
        int idx = mask[i] & 0xFF;
        
        /* Floating point operations with dependencies */
        float x = a[idx] * 2.5f;
        float y = b[i] / 1.7f;
        
        /* More complex FP chain */
        float z = x * y - sinf(x) + cosf(y);
        
        /* Integer operation dependent on FP result */
        int count = (int)(fabsf(z) * 100);
        
        /* Another FP operation dependent on integer */
        result += z * count + fp_mix(x, y, result);
        
        /* Conditional that may cause speculative scheduling */
        if (count > 1000) {
            result *= 0.99f;
        }
    }
    
    return result;
}

/* Function with SIMD-like operations using vector extensions */
NO_OPTIMIZE
void vector_operations(v4si *va, v4si *vb, v4si *vc, int count) {
    /* Vector operations create many parallel instructions */
    for (int i = 0; i < count; i++) {
        v4si v1 = va[i] + vb[i] * 2;
        v4si v2 = vb[i] - va[i] / 3;
        v4si v3 = v1 & v2 | va[i];
        v4si v4 = v1 * v2 + v3;
        
        /* Conditional vector operation */
        v4si mask = v1 > v2;
        vc[i] = (v1 & mask) | (v2 & ~mask);
        
        /* Additional scalar operations mixed in */
        int sum = 0;
        for (int j = 0; j < 4; j++) {
            sum += vc[i][j];
        }
        vc[i][0] = sum;
    }
}

/* Function with inner loop for software pipelining attempts */
NO_OPTIMIZE
double loop_scheduling(int iterations) {
    double acc = 0.0;
    double x = 0.1;
    double y = 0.2;
    
    /* Small inner loop that might be software pipelined */
    for (int i = 0; i < iterations; i++) {
        /* Dependent operations creating long chains */
        double t1 = x * y + sin(x);
        double t2 = y * t1 - cos(y);
        double t3 = t1 * t2 + tan(t1 + t2);
        double t4 = complex_fp(t2, t3, 3);
        
        /* Conditional update - may trigger state saving */
        if (t4 > 0.5) {
            acc += t4 * 0.8;
            x = t3 * 0.9;
        } else {
            acc -= t4 * 0.7;
            y = t2 * 1.1;
        }
        
        /* Additional operations to increase pressure */
        acc = fmod(acc, 1000.0);
        x = fmod(x + 0.01, 1.0);
        y = fmod(y + 0.02, 1.0);
    }
    
    return acc;
}

/* Function with computed goto for complex control flow */
NO_OPTIMIZE
int computed_goto_switch(int value, int *data, int size) {
    static void *jumptable[] = {
        &&case0, &&case1, &&case2, &&case3,
        &&case4, &&case5, &&case6, &&case7
    };
    
    int idx = value & 7;
    int result = 0;
    
    /* Computed goto creates scheduling barriers */
    goto *jumptable[idx];
    
case0:
    result = data[0] * 2 + 1;
    break;
case1:
    result = data[1] - data[2] * 3;
    break;
case2:
    result = (data[3] << 4) | (data[4] & 0xF);
    break;
case3:
    result = compute_hash(data[5], data[6], data[7]);
    break;
case4:
    result = data[8] ^ data[9] ^ data[10];
    break;
case5:
    result = data[11] * data[12] / (data[13] + 1);
    break;
case6:
    result = (data[14] + data[15]) * (data[16] - data[17]);
    break;
case7:
    result = 0;
    for (int i = 0; i < 8 && i < size; i++) {
        result += data[i] * (i + 1);
    }
    break;
    
    /* Common tail with more operations */
    result = result * 2 - 1;
    return result > 0 ? result : -result;
}

/* Function with inline assembly creating scheduling barriers */
NO_OPTIMIZE
void asm_barriers(int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Inline assembly acts as scheduling barrier */
        asm volatile("" : : "r"(a[i]), "r"(b[i]) : "memory");
        
        /* Complex dependency chain after barrier */
        int t1 = a[i] * 3 + b[i];
        int t2 = b[i] * 2 - a[i];
        int t3 = t1 ^ t2;
        int t4 = (t1 + t2) * (t1 - t2);
        
        /* Another assembly barrier */
        asm volatile("" : : "r"(t3), "r"(t4) : "memory");
        
        c[i] = t3 + t4 / (t1 + 1);
    }
}

/* Main test driver */
int main() {
    const int SIZE = 1024;
    int *int_data = malloc(SIZE * sizeof(int));
    float *float_data_a = malloc(SIZE * sizeof(float));
    float *float_data_b = malloc(SIZE * sizeof(float));
    int *mask_data = malloc(SIZE * sizeof(int));
    int *output_data = malloc(SIZE * sizeof(int));
    v4si *vec_data_a = malloc(SIZE/4 * sizeof(v4si));
    v4si *vec_data_b = malloc(SIZE/4 * sizeof(v4si));
    v4si *vec_data_c = malloc(SIZE/4 * sizeof(v4si));
    
    /* Initialize data with pseudo-random values */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data_a[i] = (float)rand() / RAND_MAX * 100.0f;
        float_data_b[i] = (float)rand() / RAND_MAX * 100.0f;
        mask_data[i] = rand() % SIZE;
    }
    
    for (int i = 0; i < SIZE/4; i++) {
        for (int j = 0; j < 4; j++) {
            vec_data_a[i][j] = rand() % 1000;
            vec_data_b[i][j] = rand() % 1000;
        }
    }
    
    printf("Starting scheduler coverage tests...\n");
    
    /* Test 1: Wide basic block with many instructions */
    clock_t start = clock();
    wide_basic_block(int_data, output_data, SIZE);
    clock_t end = clock();
    printf("Test 1 (wide basic block): %.6f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Test 2: Mixed integer/float operations */
    start = clock();
    float mixed_result = mixed_operations(100, float_data_a, float_data_b, mask_data);
    end = clock();
    printf("Test 2 (mixed operations): %.6f seconds, result = %f\n",
           (double)(end - start) / CLOCKS_PER_SEC, mixed_result);
    
    /* Test 3: Vector operations */
    start = clock();
    vector_operations(vec_data_a, vec_data_b, vec_data_c, SIZE/4);
    end = clock();
    printf("Test 3 (vector operations): %.6f seconds\n",
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Test 4: Loop scheduling with software pipelining */
    start = clock();
    double loop_result = loop_scheduling(1000);
    end = clock();
    printf("Test 4 (loop scheduling): %.6f seconds, result = %f\n",
           (double)(end - start) / CLOCKS_PER_SEC, loop_result);
    
    /* Test 5: Computed goto with complex control flow */
    start = clock();
    int goto_result = 0;
    for (int i = 0; i < 10000; i++) {
        goto_result += computed_goto_switch(i, int_data, SIZE);
    }
    end = clock();
    printf("Test 5 (computed goto): %.6f seconds, result = %d\n",
           (double)(end - start) / CLOCKS_PER_SEC, goto_result);
    
    /* Test 6: Assembly barriers */
    start = clock();
    asm_barriers(int_data, output_data, mask_data, SIZE/2);
    end = clock();
    printf("Test 6 (assembly barriers): %.6f seconds\n",
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Verify results aren't optimized away */
    int checksum = 0;
    for (int i = 0; i < SIZE && i < 100; i++) {
        checksum ^= output_data[i];
        checksum ^= mask_data[i];
    }
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(int_data);
    free(float_data_a);
    free(float_data_b);
    free(mask_data);
    free(output_data);
    free(vec_data_a);
    free(vec_data_b);
    free(vec_data_c);
    
    return 0;
}
