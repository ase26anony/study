/* test_sched_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler by creating complex basic blocks that require:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving for speculative scheduling
 * 3. Large instruction queues and ready lists
 * 4. Mixed instruction types and dependencies
 */

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

/* Complex arithmetic with dependencies - creates long dependency chains */
static inline ALWAYS_INLINE double complex_chain(double a, double b, double c, double d, int iterations) {
    double result = a;
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chain with mixed operations */
        result = result * b + c;
        result = sin(result) * cos(result);
        result = result / (d + 1.0);
        result = sqrt(fabs(result)) + 0.5;
        result = result * result - result / 2.0;
    }
    return result;
}

/* Matrix operations - creates wide basic blocks when unrolled */
static inline ALWAYS_INLINE void matrix_multiply_3x3(double A[3][3], double B[3][3], double C[3][3]) {
    /* Unrolled 3x3 matrix multiplication - creates many independent operations */
    C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0] + A[0][2]*B[2][0];
    C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1] + A[0][2]*B[2][1];
    C[0][2] = A[0][0]*B[0][2] + A[0][1]*B[1][2] + A[0][2]*B[2][2];
    
    C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0] + A[1][2]*B[2][0];
    C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1] + A[1][2]*B[2][1];
    C[1][2] = A[1][0]*B[0][2] + A[1][1]*B[1][2] + A[1][2]*B[2][2];
    
    C[2][0] = A[2][0]*B[0][0] + A[2][1]*B[1][0] + A[2][2]*B[2][0];
    C[2][1] = A[2][0]*B[0][1] + A[2][1]*B[1][1] + A[2][2]*B[2][1];
    C[2][2] = A[2][0]*B[0][2] + A[2][1]*B[1][2] + A[2][2]*B[2][2];
}

/* Function with speculative scheduling opportunities */
static inline ALWAYS_INLINE int speculative_operation(int* data, int size, int threshold) {
    int sum = 0;
    int product = 1;
    
    /* Loop with conditional that may cause speculative scheduling */
    for (int i = 0; i < size; i++) {
        /* Memory access with potential aliasing */
        int val = data[i];
        
        /* Conditional operations - scheduler may speculate */
        if (val > threshold) {
            sum += val * 2;
            product *= (val + 1);
        } else {
            sum -= val / 2;
            product /= (val > 0 ? val : 1);
        }
        
        /* More arithmetic to create scheduling pressure */
        val = val * val - val;
        data[i] = val % 256;
    }
    
    /* Final conditional with complex expression */
    return (sum > product) ? sum : product;
}

/* Function using GCC vector extensions - creates parallel operations */
static inline ALWAYS_INLINE v4sf vector_operations(v4sf a, v4sf b, v4sf c) {
    v4sf result;
    
    /* Multiple vector operations that can be scheduled in parallel */
    result = a + b * c;
    result = result - a * b;
    result = result + c / (a + 1.0f);
    result = result * result - b;
    
    /* Conditional vector operation */
    v4sf mask = a > b;
    result = (mask & result) | (~mask & c);
    
    return result;
}

/* Function with mixed integer/float operations and function calls */
static double mixed_operations(int n, double* array) {
    double acc = 0.0;
    float f_acc = 0.0f;
    int i_acc = 0;
    
    /* Loop with small iteration count - may trigger software pipelining */
    for (int i = 0; i < 8; i++) {
        /* Mixed type operations */
        double d_val = array[i];
        float f_val = (float)d_val;
        int i_val = (int)d_val;
        
        /* Function calls create scheduling barriers */
        d_val = sin(d_val) * cos(d_val);
        f_val = sqrtf(fabsf(f_val));
        i_val = i_val * i_val - i_val;
        
        /* More arithmetic with dependencies */
        acc += d_val;
        f_acc += f_val * 0.5f;
        i_acc += i_val % 100;
        
        /* Store back with potential aliasing */
        array[i] = d_val + f_val + i_val;
    }
    
    return acc + f_acc + i_acc;
}

/* Wide basic block with many independent operations */
static double wide_basic_block(double a, double b, double c, double d,
                               double e, double f, double g, double h) {
    /* Many independent arithmetic operations - fills ready list */
    double r1 = a * b + c;
    double r2 = d - e * f;
    double r3 = g / h + a;
    double r4 = b * c - d;
    double r5 = e + f * g;
    double r6 = h / a + b;
    double r7 = c * d - e;
    double r8 = f + g / h;
    double r9 = a - b * c;
    double r10 = d + e - f;
    
    /* Create some dependencies */
    r1 = r1 * r2;
    r3 = r3 + r4 * r5;
    r6 = r6 - r7 / r8;
    r9 = r9 * r10;
    
    /* More operations */
    double r11 = sin(r1) * cos(r2);
    double r12 = sqrt(fabs(r3)) + r4;
    double r13 = exp(r5 * 0.1) - r6;
    double r14 = log(fabs(r7) + 1.0) * r8;
    double r15 = pow(r9, 2.0) + r10;
    
    /* Final combination */
    return r11 + r12 + r13 + r14 + r15;
}

/* Function with computed goto-like switch - may trigger state saving */
static int switch_based_computation(int mode, int x) {
    int result = x;
    
    /* Complex switch that may require state tracking */
    switch (mode % 5) {
        case 0:
            result = x * x - x;
            /* Fall through */
        case 1:
            result += x / 2;
            result = result * 3;
            break;
        case 2:
            result = x << 2;
            result = result | (x & 0xFF);
            break;
        case 3:
            result = ~x;
            result = result ^ 0xAAAA;
            break;
        case 4:
            result = x * 3;
            result = result % 257;
            break;
        default:
            result = -x;
    }
    
    /* Additional conditional */
    if (result > 1000) {
        result = result / 2;
    } else if (result < -1000) {
        result = result * 2;
    } else {
        result = result + 1;
    }
    
    return result;
}

/* Main test function that combines all patterns */
double test_scheduler_1(int iterations) {
    double total = 0.0;
    double array[8];
    
    /* Initialize array */
    for (int i = 0; i < 8; i++) {
        array[i] = (i + 1) * 0.5;
    }
    
    /* Test complex chain */
    total += complex_chain(1.0, 2.0, 3.0, 4.0, 4);
    
    /* Test mixed operations */
    total += mixed_operations(8, array);
    
    /* Test wide basic block */
    total += wide_basic_block(1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8);
    
    /* Test matrix operations */
    double A[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    double B[3][3] = {{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};
    double C[3][3];
    matrix_multiply_3x3(A, B, C);
    total += C[0][0] + C[1][1] + C[2][2];
    
    /* Test speculative operations */
    int int_data[16];
    for (int i = 0; i < 16; i++) int_data[i] = i * 10;
    total += speculative_operation(int_data, 16, 50);
    
    /* Test switch-based computation */
    for (int i = 0; i < 10; i++) {
        total += switch_based_computation(i, i * 100);
    }
    
    return total;
}

/* Second test function with different patterns */
double test_scheduler_2(int size) {
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_c = {9.0f, 10.0f, 11.0f, 12.0f};
    
    /* Vector operations */
    v4sf vec_result = vector_operations(vec_a, vec_b, vec_c);
    float vec_sum = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Nested loops with small iteration counts */
    double matrix[4][4];
    double sum = 0.0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i * 0.5 + j * 0.25;
            
            /* Inline complex computation */
            double val = matrix[i][j];
            for (int k = 0; k < 3; k++) {
                val = sin(val) * cos(val) + tan(val * 0.1);
                val = sqrt(fabs(val)) + 0.1;
            }
            matrix[i][j] = val;
            sum += val;
        }
    }
    
    /* Memory intensive operations */
    double* buffer = (double*)malloc(size * sizeof(double));
    if (!buffer) return 0.0;
    
    for (int i = 0; i < size; i++) {
        buffer[i] = i * 0.01;
    }
    
    /* Process buffer with stride */
    double buffer_sum = 0.0;
    for (int i = 0; i < size - 1; i += 2) {
        buffer_sum += buffer[i] * buffer[i + 1];
        buffer[i] = buffer_sum * 0.5;
    }
    
    free(buffer);
    
    return vec_sum + sum + buffer_sum;
}

/* Third test function with unrolled loops */
double test_scheduler_3() {
    /* Manually unrolled loop - creates very wide basic block */
    double values[32];
    double result = 0.0;
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        values[i] = i * 0.3;
    }
    
    /* Unrolled processing - 32 independent chains */
    values[0] = values[0] * 2.0 + 1.0;
    values[1] = values[1] / 1.5 - 0.5;
    values[2] = sin(values[2]) * cos(values[2]);
    values[3] = sqrt(fabs(values[3])) + 0.1;
    values[4] = values[4] * values[4] - values[4];
    values[5] = values[5] + values[0] * values[1];
    values[6] = values[6] - values[2] / values[3];
    values[7] = values[7] * exp(values[4] * 0.01);
    values[8] = log(fabs(values[5]) + 1.0);
    values[9] = pow(values[6], 1.5);
    values[10] = values[7] + values[8] * values[9];
    values[11] = values[10] / (values[0] + 1.0);
    values[12] = values[11] * 3.14159;
    values[13] = values[12] - 2.71828;
    values[14] = sin(values[13]) + cos(values[11]);
    values[15] = values[14] * values[14];
    
    /* More operations... */
    for (int i = 16; i < 32; i++) {
        values[i] = values[i-16] * 0.9 + values[i-15] * 0.1;
    }
    
    /* Combine results */
    for (int i = 0; i < 32; i++) {
        result += values[i];
    }
    
    return result;
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    double total_result = 0.0;
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    
    printf("Starting scheduler coverage test...\n");
    
    /* Run multiple test functions to cover different scheduling scenarios */
    clock_t start = clock();
    
    for (int i = 0; i < iterations; i++) {
        total_result += test_scheduler_1(i);
        total_result += test_scheduler_2(100);
        total_result += test_scheduler_3();
        
        /* Alternate between different computation patterns */
        if (i % 2 == 0) {
            /* Additional stress test */
            double temp = 1.0;
            for (int j = 0; j < 50; j++) {
                temp = temp * 1.01 + sin(temp * 0.1);
            }
            total_result += temp;
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total result: %f\n", total_result);
    printf("Elapsed time: %f seconds\n", elapsed);
    printf("Iterations: %d\n", iterations);
    
    /* Verify result isn't optimized away */
    if (total_result > 0.0) {
        printf("Test completed successfully.\n");
        return 0;
    } else {
        printf("Warning: Result may have been optimized away.\n");
        return 1;
    }
}
