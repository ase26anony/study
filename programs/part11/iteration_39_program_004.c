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
#include <string.h>
#include <time.h>

/* Define vector types to create parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Inline functions to increase instruction count in basic blocks */
static inline int compute_hash(int a, int b, int c) {
    /* Complex dependency chain */
    int t1 = a ^ b;
    int t2 = t1 * 0x5bd1e995;
    int t3 = t2 ^ (t2 >> 15);
    int t4 = t3 * c;
    int t5 = t4 ^ (t4 >> 13);
    return t5;
}

static inline float fp_complex(float a, float b, float c) {
    /* Floating-point dependency chain */
    float t1 = sinf(a) * cosf(b);
    float t2 = tanhf(t1) + expf(c);
    float t3 = logf(fabsf(t2) + 1.0f);
    return t3 * t3;
}

static inline double vector_dot(v2df a, v2df b) {
    /* Force vector operations */
    v2df prod = a * b;
    double* p = (double*)&prod;
    return p[0] + p[1];
}

/* Function 1: Complex arithmetic with mixed operations and dependencies */
/* This creates a wide basic block with multiple independent chains */
void test_complex_arithmetic(int* data, int n, int* result) {
    int i;
    /* Unrolled loop creates wide basic block */
    for (i = 0; i < n - 7; i += 8) {
        /* Multiple independent computation paths */
        int a1 = data[i] * 3 + data[i+1];
        int a2 = data[i+1] - data[i] * 2;
        int a3 = compute_hash(data[i], data[i+2], a1);
        int a4 = compute_hash(data[i+3], a2, a3);
        
        float f1 = fp_complex(data[i] * 0.1f, data[i+1] * 0.2f, data[i+2] * 0.3f);
        float f2 = fp_complex(data[i+3] * 0.4f, data[i+4] * 0.5f, f1);
        
        /* Memory operations with potential aliasing */
        result[i] = a1 + a3 + (int)(f1 * 100);
        result[i+1] = a2 + a4 + (int)(f2 * 100);
        result[i+2] = result[i] ^ result[i+1];
        result[i+3] = result[i+2] * 7 - a4;
        
        /* More independent chains */
        int b1 = data[i+4] << 2;
        int b2 = data[i+5] >> 1;
        int b3 = b1 | b2;
        int b4 = b3 & 0x7FFFFFFF;
        
        result[i+4] = b4 + result[i];
        result[i+5] = b3 - result[i+1];
        result[i+6] = result[i+4] * result[i+5];
        result[i+7] = result[i+6] % (abs(data[i+6]) + 1);
    }
    
    /* Handle remainder with conditional - creates control flow for state saving */
    for (; i < n; i++) {
        if (data[i] > 0) {
            result[i] = compute_hash(data[i], i, result[i-1]);
        } else {
            result[i] = -compute_hash(-data[i], i, -result[i-1]);
        }
    }
}

/* Function 2: Matrix operations with vector extensions */
/* Creates instruction mixes that use different execution units */
void test_matrix_vector_ops(float* mat, float* vec, float* result, int size) {
    int i, j;
    v4sf vsum = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Outer loop with inner unrolling - creates scheduling pressure */
    for (i = 0; i < size; i++) {
        v4sf vrow, vvec, vprod;
        float* row = &mat[i * size];
        
        /* Process 4 elements at a time using vector operations */
        for (j = 0; j < size - 3; j += 4) {
            /* Load row elements as vector */
            vrow = *(v4sf*)&row[j];
            /* Load vector elements */
            vvec = *(v4sf*)&vec[j];
            /* Multiply and accumulate */
            vprod = vrow * vvec;
            vsum += vprod;
        }
        
        /* Horizontal sum of vector */
        float* sum_ptr = (float*)&vsum;
        float dot = sum_ptr[0] + sum_ptr[1] + sum_ptr[2] + sum_ptr[3];
        
        /* Handle remainder with scalar operations */
        for (; j < size; j++) {
            dot += row[j] * vec[j];
        }
        
        /* Complex dependency chain with conditional */
        if (dot > 0.0f) {
            result[i] = sqrtf(fabsf(dot)) + sinf(dot * 0.01f);
        } else {
            result[i] = -sqrtf(fabsf(dot)) + cosf(dot * 0.01f);
        }
        
        /* Reset vector sum */
        vsum = (v4sf){0.0f, 0.0f, 0.0f, 0.0f};
    }
}

/* Function 3: Numerical integration with mixed precision */
/* Creates speculative scheduling opportunities */
double test_numerical_integration(int iterations) {
    double sum = 0.0;
    int i;
    
    /* Small loop that might be software pipelined */
    for (i = 0; i < iterations; i++) {
        double x = (i + 0.5) / iterations;
        
        /* Multiple dependent floating-point operations */
        double term1 = sin(x * M_PI);
        double term2 = cos(x * M_PI * 2.0);
        double term3 = exp(-x * x);
        double term4 = log(1.0 + x);
        
        /* Complex expression with dependencies */
        double value = term1 * term2 + term3 / (term4 + 1.0);
        
        /* Conditional update - creates scheduling barrier */
        if (value > 0.0) {
            sum += value * value;
        } else {
            sum -= value * value * 0.5;
        }
        
        /* Additional integer computation */
        int mod_check = i % 8;
        if (mod_check == 0) {
            sum += 0.001 * (i / 8);
        }
    }
    
    return sum / iterations;
}

/* Function 4: String processing with inline assembly */
/* Triggers target-specific scheduling hooks */
void test_string_processing(char* str, int len, unsigned int* hash) {
    int i;
    unsigned int h = 0xDEADBEEF;
    
    /* Process string in chunks */
    for (i = 0; i < len - 3; i += 4) {
        /* Load 4 bytes at once */
        unsigned int chunk;
        memcpy(&chunk, &str[i], 4);
        
        /* Complex hash computation with multiple operations */
        chunk ^= chunk >> 16;
        chunk *= 0x85ebca6b;
        chunk ^= chunk >> 13;
        chunk *= 0xc2b2ae35;
        chunk ^= chunk >> 16;
        
        /* Mix with previous hash */
        h = h ^ chunk;
        h = h * 0x5bd1e995 + 0x10001;
        
        /* Inline assembly for x86-specific operations */
        /* This triggers target-specific scheduling hooks */
        #ifdef __x86_64__
        asm volatile (
            "rorl $7, %0\n\t"
            "addl $0x9e3779b9, %0\n\t"
            : "+r" (h)
            :
            : "cc"
        );
        #endif
        
        /* Additional computation to increase instruction count */
        h = (h >> 3) | (h << 29);
        h += i * 0x1234567;
    }
    
    /* Handle remainder */
    for (; i < len; i++) {
        h ^= str[i];
        h *= 0x01000193;
    }
    
    *hash = h;
}

/* Function 5: Complex control flow with switch statement */
/* Forces frontend state saving */
int test_control_flow(int* data, int n, int mode) {
    int result = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        int val = data[i];
        
        /* Switch with multiple cases creates complex control flow */
        switch (mode) {
            case 0:
                /* Integer arithmetic chain */
                val = val * 3 + 1;
                val = (val >> 1) ^ (val << 3);
                result += val;
                break;
                
            case 1:
                /* Floating-point operations */
                float fval = val * 0.01f;
                fval = sinf(fval) * cosf(fval);
                result += (int)(fval * 1000);
                break;
                
            case 2:
                /* Memory-intensive */
                result ^= val;
                result = (result << 5) | (result >> 27);
                break;
                
            case 3:
                /* Complex computation */
                val = compute_hash(val, i, result);
                result = result * 0x9e3779b9 + val;
                break;
                
            default:
                /* Mixed operations */
                val = val * val - val;
                result += val % 17;
                break;
        }
        
        /* Conditional that might be speculatively scheduled */
        if (result < 0) {
            result = -result;
            result |= 0x80000000;
        }
    }
    
    return result;
}

/* Function 6: Wide basic block with many independent operations */
/* Fills instruction queues and ready lists */
void test_wide_block(int* in1, int* in2, int* out, int n) {
    /* Unroll heavily to create very wide basic block */
    int i;
    for (i = 0; i < n - 15; i += 16) {
        /* 16 independent computation chains */
        int t0 = in1[i] + in2[i];
        int t1 = in1[i+1] - in2[i+1];
        int t2 = in1[i+2] * in2[i+2];
        int t3 = in1[i+3] ^ in2[i+3];
        int t4 = in1[i+4] | in2[i+4];
        int t5 = in1[i+5] & in2[i+5];
        int t6 = in1[i+6] << (in2[i+6] & 3);
        int t7 = in1[i+7] >> (in2[i+7] & 3);
        int t8 = compute_hash(in1[i+8], in2[i+8], t0);
        int t9 = compute_hash(in1[i+9], in2[i+9], t1);
        int t10 = t2 * t3 + t4;
        int t11 = t5 ^ t6 | t7;
        int t12 = t8 - t9 * t10;
        int t13 = t11 + t12 / (abs(t10) + 1);
        int t14 = (t13 << 3) ^ (t13 >> 5);
        int t15 = t14 * 0x9e3779b9 + 0x10001;
        
        /* Store results */
        out[i] = t0;
        out[i+1] = t1;
        out[i+2] = t2;
        out[i+3] = t3;
        out[i+4] = t4;
        out[i+5] = t5;
        out[i+6] = t6;
        out[i+7] = t7;
        out[i+8] = t8;
        out[i+9] = t9;
        out[i+10] = t10;
        out[i+11] = t11;
        out[i+12] = t12;
        out[i+13] = t13;
        out[i+14] = t14;
        out[i+15] = t15;
    }
}

/* Main driver function */
int main() {
    const int SIZE = 1024;
    const int MAT_SIZE = 32;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate test data */
    int* data1 = (int*)malloc(SIZE * sizeof(int));
    int* data2 = (int*)malloc(SIZE * sizeof(int));
    int* result_int = (int*)malloc(SIZE * sizeof(int));
    float* matrix = (float*)malloc(MAT_SIZE * MAT_SIZE * sizeof(float));
    float* vector = (float*)malloc(MAT_SIZE * sizeof(float));
    float* result_float = (float*)malloc(MAT_SIZE * sizeof(float));
    char* test_string = (char*)malloc(SIZE);
    unsigned int hash_result;
    
    /* Initialize data with pseudo-random values */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        test_string[i] = 'A' + (rand() % 26);
    }
    
    for (int i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
        matrix[i] = (rand() % 1000) * 0.001f;
    }
    
    for (int i = 0; i < MAT_SIZE; i++) {
        vector[i] = (rand() % 1000) * 0.001f;
    }
    
    printf("Starting scheduler coverage tests...\n");
    
    /* Test 1: Complex arithmetic with dependencies */
    start = clock();
    test_complex_arithmetic(data1, SIZE, result_int);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Test 1 (Complex Arithmetic): %.6f seconds\n", cpu_time_used);
    
    /* Test 2: Matrix-vector operations with SIMD */
    start = clock();
    test_matrix_vector_ops(matrix, vector, result_float, MAT_SIZE);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Test 2 (Matrix-Vector SIMD): %.6f seconds\n", cpu_time_used);
    
    /* Test 3: Numerical integration */
    start = clock();
    double integral = test_numerical_integration(10000);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Test 3 (Numerical Integration): %.6f seconds, result = %f\n", 
           cpu_time_used, integral);
    
    /* Test 4: String processing with inline assembly */
    start = clock();
    test_string_processing(test_string, SIZE, &hash_result);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Test 4 (String Processing): %.6f seconds, hash = 0x%08x\n", 
           cpu_time_used, hash_result);
    
    /* Test 5: Complex control flow */
    start = clock();
    int cf_result = test_control_flow(data1, SIZE, 2);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Test 5 (Control Flow): %.6f seconds, result = %d\n", 
           cpu_time_used, cf_result);
    
    /* Test 6: Wide basic block */
    start = clock();
    test_wide_block(data1, data2, result_int, SIZE);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Test 6 (Wide Basic Block): %.6f seconds\n", cpu_time_used);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += result_int[i];
    }
    for (int i = 0; i < MAT_SIZE; i++) {
        checksum += (unsigned long long)(result_float[i] * 1000);
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(result_int);
    free(matrix);
    free(vector);
    free(result_float);
    free(test_string);
    
    return 0;
}
