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
typedef double v2df __attribute__((vector_size(16)));

/* Complex arithmetic chains with dependencies */
static ALWAYS_INLINE double complex_chain_1(double a, double b, double c, 
                                           double d, double e, double f) {
    /* Long dependency chain mixing operations */
    double t1 = a * b + c;
    double t2 = t1 / (d + 1.0);
    double t3 = sin(t2) * cos(e);
    double t4 = t3 - exp(f);
    double t5 = log(fabs(t4) + 1.0);
    double t6 = t5 * t5 + t3;
    double t7 = sqrt(fabs(t6)) + t2;
    return t7;
}

static ALWAYS_INLINE int integer_chain(int a, int b, int c, int d, int e) {
    /* Integer dependency chain with mixed operations */
    int t1 = a * b + c;
    int t2 = (t1 >> 3) | (d << 2);
    int t3 = t2 ^ e;
    int t4 = t3 * 0x5A827999;
    int t5 = (t4 + a) * (b + 1);
    int t6 = t5 % (abs(c) + 1);
    int t7 = t6 & 0x7FFFFFFF;
    return t7;
}

/* Function with inline assembly to create scheduling barriers */
static ALWAYS_INLINE void memory_barrier() {
    /* Compiler barrier preventing reordering */
    asm volatile("" ::: "memory");
}

/* Function that creates a wide basic block with independent paths */
static ALWAYS_INLINE void wide_block_operations(int* arr, float* farr, 
                                               double* darr, int n) {
    /* Multiple independent computation paths */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    float fsum1 = 0.0f, fsum2 = 0.0f, fsum3 = 0.0f, fsum4 = 0.0f;
    double dsum1 = 0.0, dsum2 = 0.0, dsum3 = 0.0, dsum4 = 0.0;
    
    /* Unrolled loop creates many instructions in one basic block */
    for (int i = 0; i < n; i += 4) {
        /* Path 1: Integer operations */
        sum1 += arr[i] * 3;
        sum2 += arr[i+1] * 7;
        sum3 += arr[i+2] * 11;
        sum4 += arr[i+3] * 13;
        
        /* Path 2: Float operations */
        fsum1 += farr[i] * 1.5f;
        fsum2 += farr[i+1] * 2.5f;
        fsum3 += farr[i+2] * 3.5f;
        fsum4 += farr[i+3] * 4.5f;
        
        /* Path 3: Double precision operations */
        dsum1 += darr[i] * 0.25;
        dsum2 += darr[i+1] * 0.5;
        dsum3 += darr[i+2] * 0.75;
        dsum4 += darr[i+3] * 1.0;
        
        /* Memory operations with potential aliasing */
        arr[i] = sum1 ^ sum2;
        farr[i+1] = fsum2 * 0.1f;
        darr[i+2] = dsum3 / 2.0;
    }
    
    /* Final reduction with dependencies */
    int final_int = sum1 + sum2 + sum3 + sum4;
    float final_float = fsum1 + fsum2 + fsum3 + fsum4;
    double final_double = dsum1 + dsum2 + dsum3 + dsum4;
    
    /* Store results with barrier */
    memory_barrier();
    arr[0] = final_int;
    farr[0] = final_float;
    darr[0] = final_double;
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE int speculative_block(int cond, int* data, int n) {
    int result = 0;
    
    /* Complex conditional with multiple basic blocks */
    if (cond > 0) {
        /* Block with arithmetic chain */
        for (int i = 0; i < n; i++) {
            result += integer_chain(data[i], data[i+1], data[i+2], 
                                   data[i+3], data[i+4]);
        }
    } else if (cond < 0) {
        /* Different block with FP operations */
        double acc = 0.0;
        for (int i = 0; i < n; i++) {
            acc += complex_chain_1(data[i], data[i+1], data[i+2],
                                  data[i+3], data[i+4], data[i+5]);
        }
        result = (int)acc;
    } else {
        /* Third path with mixed operations */
        for (int i = 0; i < n; i++) {
            int t = data[i];
            float ft = t * 0.5f;
            double dt = ft * 2.0;
            result += (int)(dt * t);
        }
    }
    
    /* Common tail with more computations */
    result = result * 31 + 17;
    memory_barrier();
    return result;
}

/* Function using vector extensions for parallel operations */
static ALWAYS_INLINE v4si vector_operations(v4si a, v4si b, v4si c, v4si d) {
    /* Multiple vector operations creating many instructions */
    v4si t1 = a + b;
    v4si t2 = c * d;
    v4si t3 = t1 & t2;
    v4si t4 = t3 | a;
    v4si t5 = t4 << 2;
    v4si t6 = t5 >> 1;
    v4si t7 = t6 ^ b;
    v4si t8 = t7 + c;
    v4si t9 = t8 * d;
    v4si t10 = t9 & 0x7F;
    
    return t10;
}

/* Main test function with multiple complex basic blocks */
static void test_function_1(int iterations) {
    const int ARRAY_SIZE = 256;
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = i + 1;
        float_data[i] = (i + 1) * 0.1f;
        double_data[i] = (i + 1) * 0.01;
    }
    
    int checksum = 0;
    double fp_checksum = 0.0;
    
    /* Loop with small iteration count for potential software pipelining */
    for (int iter = 0; iter < iterations; iter++) {
        /* Basic Block 1: Wide block with independent paths */
        wide_block_operations(int_data, float_data, double_data, 64);
        
        /* Basic Block 2: Speculative scheduling test */
        int spec_result = speculative_block(iter % 3 - 1, int_data, 16);
        checksum += spec_result;
        
        /* Basic Block 3: Vector operations */
        v4si va = {1, 2, 3, 4};
        v4si vb = {5, 6, 7, 8};
        v4si vc = {9, 10, 11, 12};
        v4si vd = {13, 14, 15, 16};
        
        for (int i = 0; i < 8; i++) {
            v4si result = vector_operations(va, vb, vc, vd);
            /* Extract and sum elements */
            int res_arr[4];
            memcpy(res_arr, &result, sizeof(result));
            checksum += res_arr[0] + res_arr[1] + res_arr[2] + res_arr[3];
            
            /* Modify vectors for next iteration */
            va += 1; vb += 2; vc += 3; vd += 4;
        }
        
        /* Basic Block 4: Mixed FP and integer chains */
        for (int i = 0; i < 8; i++) {
            double fp_val = complex_chain_1(
                double_data[i], double_data[i+1], double_data[i+2],
                double_data[i+3], double_data[i+4], double_data[i+5]
            );
            fp_checksum += fp_val;
            
            int int_val = integer_chain(
                int_data[i], int_data[i+1], int_data[i+2],
                int_data[i+3], int_data[i+4]
            );
            checksum += int_val;
        }
        
        /* Basic Block 5: Memory intensive operations with barriers */
        memory_barrier();
        for (int i = 0; i < 32; i++) {
            float_data[i] = float_data[i] * 1.1f + int_data[i] * 0.01f;
            double_data[i] = double_data[i] * 0.9 + fp_checksum * 0.001;
        }
        memory_barrier();
    }
    
    printf("Test 1 checksum: int=%d, fp=%.6f\n", checksum, fp_checksum);
    
    free(int_data);
    free(float_data);
    free(double_data);
}

/* Second test function with different instruction mix */
static void test_function_2(int size) {
    /* Create matrix operations for scheduling complexity */
    const int N = 32;
    double A[N][N], B[N][N], C[N][N];
    
    /* Initialize matrices */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = (i + j) * 0.1;
            B[i][j] = (i - j) * 0.2;
            C[i][j] = 0.0;
        }
    }
    
    /* Small matrix multiplication - creates dependency chains */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            /* Unrolled inner loop for wider basic block */
            for (int k = 0; k < N; k += 4) {
                sum += A[i][k] * B[k][j];
                sum += A[i][k+1] * B[k+1][j];
                sum += A[i][k+2] * B[k+2][j];
                sum += A[i][k+3] * B[k+3][j];
            }
            C[i][j] = sum;
            
            /* Interleave with integer operations */
            int idx = i * N + j;
            int hash = idx;
            hash = (hash << 5) - hash + (int)sum;
            hash ^= hash >> 16;
            C[i][j] += hash * 0.000001;
        }
    }
    
    /* Compute checksum */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            checksum += C[i][j] * (i + j);
        }
    }
    
    printf("Test 2 matrix checksum: %.6f\n", checksum);
}

/* Third test with switch statement for control flow complexity */
static int switch_block(int value, int* data, int n) {
    int result = 0;
    
    /* Switch creates multiple basic blocks needing scheduling */
    switch (value % 7) {
        case 0:
            for (int i = 0; i < n; i++) {
                result += data[i] * 2;
            }
            break;
        case 1:
            for (int i = 0; i < n; i++) {
                result += data[i] / 3;
            }
            break;
        case 2:
            for (int i = 0; i < n; i++) {
                float ftmp = data[i] * 0.25f;
                result += (int)(ftmp * ftmp);
            }
            break;
        case 3:
            for (int i = 0; i < n; i++) {
                double dtmp = sqrt(fabs(data[i]));
                result += (int)(dtmp * 100.0);
            }
            break;
        case 4:
            for (int i = 0; i < n; i++) {
                result ^= data[i] << (i % 16);
            }
            break;
        case 5:
            for (int i = 0; i < n; i++) {
                result |= data[i] & 0xFF;
            }
            break;
        case 6:
            for (int i = 0; i < n; i++) {
                result = (result * 31 + data[i]) & 0x7FFFFFFF;
            }
            break;
    }
    
    return result;
}

static void test_function_3(int iterations) {
    const int DATA_SIZE = 128;
    int* data = (int*)malloc(DATA_SIZE * sizeof(int));
    
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = i * 3 + 1;
    }
    
    int total = 0;
    for (int i = 0; i < iterations; i++) {
        total += switch_block(i, data, 32);
        
        /* Interleave with other operations */
        if (i % 2 == 0) {
            for (int j = 0; j < 16; j++) {
                data[j] = integer_chain(data[j], data[j+1], data[j+2],
                                       data[j+3], data[j+4]);
            }
        }
    }
    
    printf("Test 3 switch checksum: %d\n", total);
    free(data);
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    printf("Starting scheduler coverage tests with %d iterations\n", iterations);
    
    /* Run all test functions to exercise different scheduling scenarios */
    test_function_1(iterations / 4);
    test_function_2(iterations / 10);
    test_function_3(iterations / 4);
    
    /* Final mixed test */
    printf("Running final mixed test...\n");
    for (int i = 0; i < 10; i++) {
        test_function_1(2);
        test_function_3(2);
    }
    
    printf("All scheduler tests completed.\n");
    return 0;
}
