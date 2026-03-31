/* test_sched_context.c
 * Complex instruction sequences to trigger full scheduling context allocation
 * and cleanup in GCC's Haifa scheduler.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* ========== Function 1: Mixed Integer/FP with Dependency Chains ========== */
static ALWAYS_INLINE float complex_math_chain(int a, int b, float c, float d) {
    /* Long dependency chain mixing int and FP ops */
    int t1 = a * b + global_seed;
    float t2 = c * d + (float)t1;
    int t3 = t1 ^ (t1 >> 3);
    float t4 = t2 * global_float - sinf(t2);
    int t5 = t3 * 7 + (a & b);
    float t6 = t4 / (1.0f + fabsf(t4));
    double t7 = (double)t6 * global_double;
    int t8 = t5 - (t3 % 17);
    float t9 = t6 + cosf((float)t7);
    int t10 = t8 | (t5 << 2);
    
    /* Memory operations with potential aliasing */
    float array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = t9 * i + (float)t10;
    }
    
    /* Another dependency chain */
    float sum = array[0];
    for (int i = 1; i < 8; i++) {
        sum = sum * 1.1f + array[i];
    }
    
    return sum + (float)t10;
}

/* ========== Function 2: Wide Basic Block with Parallel Chains ========== */
static ALWAYS_INLINE void wide_parallel_computation(int* restrict out, 
                                                   const int* restrict in1,
                                                   const float* restrict in2,
                                                   int n) {
    /* Multiple independent computation paths */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    float facc1 = 0.0f, facc2 = 0.0f, facc3 = 0.0f, facc4 = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Chain 1: Integer operations */
        int val1 = in1[i] * 3;
        val1 = (val1 >> 2) + (val1 & 0xF);
        val1 = val1 ^ (val1 << 3);
        acc1 += val1;
        
        /* Chain 2: Different integer operations */
        int val2 = in1[i] + global_seed;
        val2 = val2 * 7 - (val2 / 5);
        val2 = (val2 << 1) | (val2 >> 31);
        acc2 ^= val2;
        
        /* Chain 3: Float operations */
        float fval1 = in2[i] * 2.5f;
        fval1 = fval1 - floorf(fval1);
        fval1 = sinf(fval1) * cosf(fval1);
        facc1 += fval1;
        
        /* Chain 4: Mixed operations */
        float fval2 = (float)in1[i] * in2[i];
        int ival = (int)(fval2 * 100.0f);
        ival = ival * ival - (ival % 11);
        acc3 += ival;
        facc2 += fval2 * 0.5f;
        
        /* Chain 5: More operations to widen the block */
        double dval = (double)in2[i] * global_double;
        dval = exp(dval * 0.1) - 1.0;
        facc3 += (float)dval;
        
        /* Chain 6: Bit manipulation */
        int val3 = in1[i] ^ global_seed;
        val3 = (val3 * 0xCCCCCCCD) >> 3;  /* Multiply by 1/10 */
        acc4 += val3;
        facc4 += (float)val3 * 0.01f;
    }
    
    /* Combine all accumulators */
    out[0] = acc1 + acc2;
    out[1] = acc3 ^ acc4;
    out[2] = (int)(facc1 * 1000.0f);
    out[3] = (int)(facc2 * facc3 * 100.0f);
    out[4] = (int)(facc4 * 10000.0f);
}

/* ========== Function 3: Vector Operations ========== */
static ALWAYS_INLINE v4sf vector_operations(v4sf a, v4sf b, v4si mask) {
    /* Multiple vector operations creating scheduling pressure */
    v4sf r1 = a + b;
    v4sf r2 = a * b;
    v4sf r3 = r1 - r2;
    v4sf r4 = __builtin_shuffle(r1, r2, (v4si){2, 1, 0, 3});
    
    /* Conditional select using mask */
    v4sf result;
    for (int i = 0; i < 4; i++) {
        if (mask[i]) {
            result[i] = r3[i];
        } else {
            result[i] = r4[i];
        }
    }
    
    /* More vector ops */
    v4sf r5 = result * (v4sf){1.1f, 0.9f, 1.2f, 0.8f};
    v4sf r6 = __builtin_ia32_sqrtps(r5);
    
    return r5 + r6;
}

/* ========== Function 4: Loop with Software Pipelining Potential ========== */
static ALWAYS_INLINE double software_pipeline_candidate(int iterations) {
    /* Small loop that might trigger software pipelining */
    double a = 1.0, b = 0.5, c = 0.25;
    double sum = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Dependent FP operations */
        double t1 = a * global_double;
        double t2 = b + sin(t1);
        double t3 = c * cos(t2);
        double t4 = t1 - t2 * t3;
        
        /* Update state for next iteration */
        a = t2 * 0.9;
        b = t3 * 1.1;
        c = t4 * 0.8;
        
        sum += t4;
        
        /* Conditional that might create speculative scheduling */
        if (sum > 1000.0) {
            sum *= 0.99;
        }
    }
    
    return sum;
}

/* ========== Function 5: Complex Control Flow ========== */
static ALWAYS_INLINE int switch_based_computation(int x, float y) {
    /* Switch statement requiring state tracking */
    int result = 0;
    
    switch (x % 7) {
        case 0:
            result = (int)(y * 10.0f) + x;
            /* Fall through */
        case 1:
            result ^= (result << 3);
            result += (int)(sinf(y) * 100.0f);
            break;
        case 2:
            result = (int)(cosf(y) * 200.0f) - x;
            /* Multiple operations in case */
            for (int i = 0; i < 3; i++) {
                result = result * 3 + i;
            }
            break;
        case 3:
            result = x * x - (int)(y * y);
            /* Nested condition */
            if (result > 0) {
                result = (int)sqrtf((float)result);
            } else {
                result = -(int)sqrtf((float)-result);
            }
            break;
        case 4:
            /* More complex path */
            result = (x << 4) | (x >> 4);
            result += (int)(expf(y) * 50.0f);
            result &= 0xFFFF;
            break;
        case 5:
            result = (int)(logf(fabsf(y) + 1.0f) * 300.0f);
            result = (result * 13) ^ 0xAAAAAAAA;
            break;
        default:
            result = x * 11 + (int)(y * 5.0f);
            result = (result % 97) + 1;
    }
    
    /* Post-switch computations */
    result = result * 2 - (result / 3);
    return result;
}

/* ========== Function 6: Memory Aliasing Stress ========== */
static ALWAYS_INLINE void memory_aliasing_test(int* arr, int size) {
    /* Accesses that might alias, preventing reordering */
    for (int i = 1; i < size - 1; i++) {
        arr[i] = arr[i-1] * 2 + arr[i] + arr[i+1] / 2;
    }
    
    /* Reverse pass with overlap */
    for (int i = size - 2; i > 0; i--) {
        arr[i] = (arr[i] + arr[i-1] + arr[i+1]) / 3;
    }
    
    /* Scatter/gather pattern */
    for (int i = 0; i < size; i += 4) {
        int idx1 = i % size;
        int idx2 = (i * 3) % size;
        int idx3 = (i * 5) % size;
        
        int temp = arr[idx1];
        arr[idx1] = arr[idx2] + global_seed;
        arr[idx2] = arr[idx3] - temp;
        arr[idx3] = temp * 2;
    }
}

/* ========== Main Test Driver ========== */
int main() {
    const int TEST_SIZE = 256;
    const int ITERATIONS = 1000;
    
    /* Allocate test data */
    int* int_data = (int*)malloc(TEST_SIZE * sizeof(int));
    float* float_data = (float*)malloc(TEST_SIZE * sizeof(float));
    int* results = (int*)malloc(5 * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(global_seed);
    for (int i = 0; i < TEST_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data[i] = (float)(rand() % 1000) / 100.0f;
    }
    
    long total_checksum = 0;
    
    /* Run multiple test functions to create different scheduling contexts */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Mixed math chain */
        float r1 = complex_math_chain(int_data[iter % TEST_SIZE], 
                                     int_data[(iter + 1) % TEST_SIZE],
                                     float_data[iter % TEST_SIZE],
                                     float_data[(iter + 2) % TEST_SIZE]);
        total_checksum += (long)(r1 * 1000.0f);
        
        /* Test 2: Wide parallel computation */
        wide_parallel_computation(results, 
                                 &int_data[iter % (TEST_SIZE - 5)],
                                 &float_data[iter % (TEST_SIZE - 5)],
                                 8);
        for (int i = 0; i < 5; i++) {
            total_checksum += results[i];
        }
        
        /* Test 3: Vector operations */
        v4sf vec_a = {float_data[iter % TEST_SIZE],
                      float_data[(iter + 1) % TEST_SIZE],
                      float_data[(iter + 2) % TEST_SIZE],
                      float_data[(iter + 3) % TEST_SIZE]};
        v4sf vec_b = {float_data[(iter + 4) % TEST_SIZE],
                      float_data[(iter + 5) % TEST_SIZE],
                      float_data[(iter + 6) % TEST_SIZE],
                      float_data[(iter + 7) % TEST_SIZE]};
        v4si mask = {iter & 1, (iter >> 1) & 1, 
                    (iter >> 2) & 1, (iter >> 3) & 1};
        v4sf vec_result = vector_operations(vec_a, vec_b, mask);
        
        for (int i = 0; i < 4; i++) {
            total_checksum += (long)(vec_result[i] * 100.0f);
        }
        
        /* Test 4: Software pipelining candidate */
        double r4 = software_pipeline_candidate(8);
        total_checksum += (long)(r4 * 10000.0);
        
        /* Test 5: Switch-based computation */
        int r5 = switch_based_computation(int_data[iter % TEST_SIZE],
                                         float_data[iter % TEST_SIZE]);
        total_checksum += r5;
        
        /* Test 6: Memory aliasing */
        if (iter % 100 == 0) {
            memory_aliasing_test(int_data, TEST_SIZE);
            total_checksum += int_data[iter % TEST_SIZE];
        }
        
        /* Update global variables to prevent optimization */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
        global_float = sinf(global_float + 0.1f);
        global_double = cos(global_double * 1.01);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %ld\n", total_checksum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(results);
    
    return 0;
}
