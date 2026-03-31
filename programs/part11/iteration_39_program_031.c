/* test_sched_context.c
 * 
 * This program is designed to trigger the uncovered cleanup logic in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that require:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
 * 3. Large instruction queues and ready lists
 * 4. Complex instruction mixes with dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for creating parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_accumulator = 0.0f;
volatile double global_double_acc = 0.0;

/* ============================================
 * Function 1: Mixed integer and FP operations
 * Creates dependency chains and uses different
 * execution units
 * ============================================ */
static ALWAYS_INLINE float complex_math_chain(int a, int b, float c, float d) {
    /* Long dependency chain with mixed operations */
    int t1 = a * b + global_seed;
    float t2 = c * d + (float)t1;
    int t3 = t1 ^ (b << 3);
    float t4 = t2 / (c + 1.0f);
    int t5 = t3 | (a & 0xFF);
    float t6 = t4 * t4 - t2;
    int t7 = t5 - t3;
    float t8 = sqrtf(fabsf(t6)) + t4;
    int t9 = t7 * 7 + t5;
    float t10 = sinf(t8) * cosf(t4);
    
    /* Memory operations with potential aliasing */
    volatile int mem1 = t9;
    volatile float mem2 = t10;
    
    return (float)t9 + t10 + mem1 + mem2;
}

/* ============================================
 * Function 2: SIMD/vector operations
 * Uses GCC vector extensions to create parallel
 * operations that expand during scheduling
 * ============================================ */
static ALWAYS_INLINE v4sf vector_operations(v4sf a, v4sf b, v4si mask) {
    /* Multiple independent vector operations */
    v4sf r1 = a + b;
    v4sf r2 = a * b;
    v4sf r3 = __builtin_shuffle(a, b, (v4si){3, 2, 1, 0});
    v4sf r4 = r1 - r2;
    
    /* Conditional vector operations */
    v4sf r5 = __builtin_ia32_blendvps(r3, r4, (v4sf)mask);
    v4sf r6 = r5 * (v4sf){1.5f, 2.0f, 0.5f, 3.0f};
    
    /* Horizontal reduction */
    float sum = r6[0] + r6[1] + r6[2] + r6[3];
    
    /* Scatter result */
    return r6 + (v4sf){sum, sum * 0.5f, sum * 0.25f, sum * 0.125f};
}

/* ============================================
 * Function 3: Loop with software pipelining
 * potential - triggers frontend state saving
 * ============================================ */
static ALWAYS_INLINE double pipelined_loop(int iterations) {
    double acc = 0.0;
    double a = 1.0, b = 2.0, c = 3.0;
    
    /* Small loop that might be software pipelined */
    for (int i = 0; i < iterations; i++) {
        /* Dependent FP operations */
        double t1 = a * b + (double)i;
        double t2 = sin(t1) * cos(b);
        double t3 = t1 * t2 - c;
        double t4 = sqrt(fabs(t3)) + t2;
        
        /* Conditional update - creates scheduling barriers */
        if (t4 > 0.0) {
            a = t4 * 0.9;
            b = t3 * 1.1;
        } else {
            a = t4 * 1.1;
            b = t3 * 0.9;
        }
        
        /* Memory operation with side effect */
        volatile double* ptr = &global_double_acc;
        *ptr = t4;
        
        acc += t4;
        c = t3 * 0.5;
    }
    
    return acc;
}

/* ============================================
 * Function 4: Wide basic block with unrolled
 * operations - fills instruction queues
 * ============================================ */
static ALWAYS_INLINE void wide_basic_block(float* arr, int size) {
    /* Multiple independent computation paths */
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
    float prod1 = 1.0f, prod2 = 1.0f;
    
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < size; i += 8) {
        /* Independent chains - can be reordered freely */
        float t1 = arr[i] * 1.1f + global_accumulator;
        float t2 = arr[i+1] * 2.2f - t1;
        float t3 = arr[i+2] * 3.3f / (t2 + 1.0f);
        float t4 = arr[i+3] * 4.4f + t3 * t2;
        
        float t5 = arr[i+4] * 5.5f - global_accumulator;
        float t6 = arr[i+5] * 6.6f + t5;
        float t7 = arr[i+6] * 7.7f / (t6 - 1.0f);
        float t8 = arr[i+7] * 8.8f - t7 * t6;
        
        /* Mix of operations */
        sum1 += t1 + t3;
        sum2 += t2 + t4;
        sum3 += t5 + t7;
        sum4 += t6 + t8;
        
        prod1 *= (t1 * t4 + 1.0f);
        prod2 *= (t5 * t8 - 1.0f);
        
        /* Array writes with potential aliasing */
        arr[i] = t1;
        arr[i+1] = t2;
        arr[i+2] = t3;
        arr[i+3] = t4;
        arr[i+4] = t5;
        arr[i+5] = t6;
        arr[i+6] = t7;
        arr[i+7] = t8;
    }
    
    /* Final reduction with dependencies */
    float final1 = sum1 * prod1 + sum2;
    float final2 = sum3 * prod2 - sum4;
    global_accumulator = final1 * final2 - (sum1 + sum2 + sum3 + sum4);
}

/* ============================================
 * Function 5: Complex control flow with switch
 * Triggers state saving for speculative scheduling
 * ============================================ */
static ALWAYS_INLINE float control_flow_intensive(int mode, float x, float y) {
    float result = 0.0f;
    
    /* Switch with multiple cases - creates complex control flow */
    switch (mode & 0x7) {
        case 0:
            result = x + y;
            /* Fall through */
        case 1:
            result *= sinf(x);
            result += cosf(y);
            break;
        case 2:
            result = x * y - (x + y);
            if (result > 0.0f) {
                result = sqrtf(result);
            } else {
                result = sqrtf(-result);
            }
            break;
        case 3:
            result = (x > y) ? x * x - y * y : y * y - x * x;
            result = logf(fabsf(result) + 1.0f);
            break;
        case 4:
            result = expf(x) * tanf(y);
            result = result / (1.0f + fabsf(result));
            break;
        case 5:
            result = powf(x, y) + powf(y, x);
            result = result * 0.5f;
            break;
        default:
            result = (x + y) * (x - y) / (x * y + 1.0f);
            break;
    }
    
    /* Inline assembly barrier - creates scheduling barrier */
    asm volatile("" ::: "memory");
    
    /* Additional computations after control flow */
    result += complex_math_chain((int)x, (int)y, x, y);
    
    return result;
}

/* ============================================
 * Function 6: Matrix operations with nested loops
 * Creates opportunities for instruction scheduling
 * ============================================ */
static ALWAYS_INLINE void matrix_multiply(float A[4][4], float B[4][4], float C[4][4]) {
    /* Nested loops with dependent operations */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            /* Unrolled inner loop */
            sum += A[i][0] * B[0][j];
            sum += A[i][1] * B[1][j];
            sum += A[i][2] * B[2][j];
            sum += A[i][3] * B[3][j];
            
            /* Conditional computation */
            if (sum > 0.0f) {
                C[i][j] = sum * 0.9f;
            } else {
                C[i][j] = sum * 1.1f;
            }
            
            /* Mix with integer operations */
            int idx = i * 4 + j;
            C[i][j] += (float)(idx % 8) * 0.125f;
        }
    }
}

/* ============================================
 * Main test driver
 * Calls all test functions with varying parameters
 * ============================================ */
int main() {
    clock_t start = clock();
    float total_result = 0.0f;
    
    /* Initialize test data */
    float array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = (float)i * 0.1f + 1.0f;
    }
    
    float matrixA[4][4], matrixB[4][4], matrixC[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrixA[i][j] = (float)(i * 4 + j) * 0.25f;
            matrixB[i][j] = (float)((3 - i) * 4 + (3 - j)) * 0.25f;
        }
    }
    
    /* Test 1: Mixed operations */
    printf("Test 1: Mixed integer/FP operations\n");
    for (int i = 0; i < 100; i++) {
        float r = complex_math_chain(i, i*2, (float)i*0.5f, (float)i*0.25f);
        total_result += r;
    }
    
    /* Test 2: Vector operations */
    printf("Test 2: Vector operations\n");
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4si mask = {0, -1, 0, -1};
    
    for (int i = 0; i < 50; i++) {
        v4sf r = vector_operations(vec1, vec2, mask);
        total_result += r[0] + r[1] + r[2] + r[3];
        vec1 = r;
        vec2 = r * (v4sf){0.9f, 1.1f, 0.8f, 1.2f};
    }
    
    /* Test 3: Software pipelining */
    printf("Test 3: Software pipelining loop\n");
    for (int i = 0; i < 20; i++) {
        double r = pipelined_loop(8);  /* Small loop for pipelining */
        total_result += (float)r;
    }
    
    /* Test 4: Wide basic block */
    printf("Test 4: Wide basic block\n");
    for (int i = 0; i < 10; i++) {
        wide_basic_block(array, 256);
        total_result += global_accumulator;
    }
    
    /* Test 5: Control flow intensive */
    printf("Test 5: Control flow with switch\n");
    for (int i = 0; i < 100; i++) {
        float r = control_flow_intensive(i, (float)i*0.1f, (float)(i+1)*0.2f);
        total_result += r;
    }
    
    /* Test 6: Matrix operations */
    printf("Test 6: Matrix operations\n");
    for (int i = 0; i < 50; i++) {
        matrix_multiply(matrixA, matrixB, matrixC);
        /* Use results to prevent dead code elimination */
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                total_result += matrixC[j][k];
            }
        }
    }
    
    /* Final computation to use all results */
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\n=== Scheduling Test Results ===\n");
    printf("Total computed result: %f\n", total_result);
    printf("Elapsed time: %f seconds\n", elapsed);
    printf("Global accumulator: %f\n", global_accumulator);
    printf("Global double accumulator: %f\n", global_double_acc);
    
    /* Return checksum to verify correctness */
    int checksum = (int)(fabs(total_result) * 1000.0f) % 1000000;
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : checksum;
}
