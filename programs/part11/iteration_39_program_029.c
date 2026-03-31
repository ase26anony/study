/* test_sched_context.c - Comprehensive test for Haifa scheduler cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;

/* Function with complex dependency chain */
static ALWAYS_INLINE int complex_dependency_chain(int a, int b, int c, int d, int e) {
    /* Multiple dependent operations */
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 - d;
    int t4 = t3 / (e + 1);
    int t5 = t4 ^ a;
    int t6 = t5 | b;
    int t7 = t6 & c;
    int t8 = t7 << 2;
    int t9 = t8 >> 1;
    return t9 * t1; /* Cross dependency */
}

/* Mixed integer and floating point operations */
static ALWAYS_INLINE float mixed_operations(int a, float b, double c, int d) {
    float f1 = (float)a * b;
    double d1 = (double)f1 + c;
    int i1 = a * d;
    float f2 = f1 + (float)d1;
    double d2 = d1 * 1.5;
    int i2 = i1 ^ d;
    float f3 = f2 * (float)d2;
    return f3 + (float)i2;
}

/* Memory operations with potential aliasing */
static ALWAYS_INLINE void memory_aliasing_ops(int* arr1, int* arr2, int size) {
    for (int i = 1; i < size - 1; i++) {
        arr1[i] = arr1[i-1] + arr2[i];
        arr2[i+1] = arr1[i] * arr2[i-1];
        arr1[i] ^= arr2[i];
    }
}

/* Function with SIMD operations */
static ALWAYS_INLINE v4sf simd_operations(v4sf a, v4sf b, v4sf c) {
    v4sf r1 = a + b;
    v4sf r2 = r1 * c;
    v4sf r3 = r2 - a;
    v4sf r4 = r3 * b;
    v4sf r5 = r4 + c;
    return r5;
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE int speculative_ops(int x, int y, int* data) {
    int result = 0;
    
    /* Multiple conditional updates */
    if (x > 0) {
        result += data[0] * x;
        if (y > 0) {
            result += data[1] * y;
            if (x > y) {
                result -= data[2];
            } else {
                result += data[3];
            }
        }
    }
    
    /* More conditions */
    if (x < 100) {
        result ^= data[4];
        if (y < 100) {
            result |= data[5];
        }
    }
    
    return result;
}

/* Wide basic block with unrolled loop */
static ALWAYS_INLINE void wide_basic_block(int* input, int* output, int n) {
    /* Unrolled operations creating many independent chains */
    for (int i = 0; i < n; i += 8) {
        /* Chain 1 */
        int t1 = input[i] + input[i+1];
        int t2 = t1 * input[i+2];
        int t3 = t2 - input[i+3];
        
        /* Chain 2 (independent) */
        int u1 = input[i+4] ^ input[i+5];
        int u2 = u1 | input[i+6];
        int u3 = u2 & input[i+7];
        
        /* Chain 3 (mixed with chain 1) */
        int v1 = t3 * u3;
        int v2 = v1 + input[i];
        
        /* Chain 4 (floating point) */
        float f1 = (float)input[i] * 1.5f;
        float f2 = f1 + (float)input[i+1];
        float f3 = f2 * 2.0f;
        
        /* Store results */
        output[i] = t3;
        output[i+1] = u3;
        output[i+2] = v2;
        output[i+3] = (int)f3;
        
        /* More independent operations */
        output[i+4] = input[i] << 2;
        output[i+5] = input[i+1] >> 1;
        output[i+6] = input[i+2] & 0xFF;
        output[i+7] = input[i+3] | 0x80;
    }
}

/* Complex function with all scheduling challenges */
void test_function_1(int iterations) {
    int data[256];
    int result[256];
    float fdata[128];
    double ddata[64];
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        data[i] = (i * global_seed) & 0xFFF;
    }
    for (int i = 0; i < 128; i++) {
        fdata[i] = (float)i * global_float;
    }
    for (int i = 0; i < 64; i++) {
        ddata[i] = (double)i * 2.71828;
    }
    
    int acc = 0;
    float facc = 0.0f;
    
    /* Outer loop with inner complex blocks */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex dependency chain */
        acc += complex_dependency_chain(
            data[iter & 0xFF],
            data[(iter + 1) & 0xFF],
            data[(iter + 2) & 0xFF],
            data[(iter + 3) & 0xFF],
            data[(iter + 4) & 0xFF]
        );
        
        /* Mixed operations */
        facc += mixed_operations(
            data[iter & 0x7F],
            fdata[iter & 0x7F],
            ddata[iter & 0x3F],
            global_seed
        );
        
        /* Memory aliasing operations */
        memory_aliasing_ops(&data[0], &data[128], 64);
        
        /* Speculative operations */
        acc ^= speculative_ops(iter, iter * 2, data);
        
        /* Wide basic block */
        if (iter % 4 == 0) {
            wide_basic_block(&data[0], &result[0], 64);
        }
        
        /* SIMD operations using vector extensions */
        v4sf va = {fdata[0], fdata[1], fdata[2], fdata[3]};
        v4sf vb = {fdata[4], fdata[5], fdata[6], fdata[7]};
        v4sf vc = {fdata[8], fdata[9], fdata[10], fdata[11]};
        v4sf vresult = simd_operations(va, vb, vc);
        
        /* Use results to prevent elimination */
        fdata[iter & 0x7] += vresult[0] + vresult[1] + vresult[2] + vresult[3];
    }
    
    /* Use results */
    printf("Test 1: acc = %d, facc = %f\n", acc, facc);
}

/* Second test function with different patterns */
void test_function_2(int size) {
    int* buffer1 = malloc(size * sizeof(int));
    int* buffer2 = malloc(size * sizeof(int));
    float* fbuffer = malloc(size * sizeof(float));
    
    if (!buffer1 || !buffer2 || !fbuffer) {
        free(buffer1);
        free(buffer2);
        free(fbuffer);
        return;
    }
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        buffer1[i] = i * 3;
        buffer2[i] = i * 5;
        fbuffer[i] = (float)i * 0.5f;
    }
    
    int sum = 0;
    
    /* Complex loop with software pipelining opportunities */
    for (int i = 0; i < size - 8; i++) {
        /* Multiple independent chains */
        int chain1 = buffer1[i] + buffer1[i+1];
        int chain2 = buffer2[i] * buffer2[i+1];
        int chain3 = chain1 ^ chain2;
        
        float fchain1 = fbuffer[i] * fbuffer[i+1];
        float fchain2 = fchain1 + fbuffer[i+2];
        int ichain = (int)fchain2;
        
        /* Cross dependencies */
        chain1 = chain3 * ichain;
        chain2 = chain1 + buffer1[i+2];
        
        /* Memory operations */
        buffer1[i+3] = chain1;
        buffer2[i+3] = chain2;
        
        /* Conditional updates */
        if (chain1 > chain2) {
            fbuffer[i+4] *= 2.0f;
        } else {
            fbuffer[i+4] /= 2.0f;
        }
        
        /* More operations */
        sum += chain1 + chain2 + ichain;
        
        /* Function call simulation with inline asm barrier */
        asm volatile("" : : "r"(chain1), "r"(chain2) : "memory");
    }
    
    /* Process results */
    for (int i = 0; i < size; i += 4) {
        buffer1[i] = (buffer1[i] + buffer2[i]) * sum;
        buffer1[i+1] = (buffer1[i+1] ^ buffer2[i+1]) | sum;
        buffer1[i+2] = (buffer1[i+2] - buffer2[i+2]) & sum;
        buffer1[i+3] = (buffer1[i+3] * buffer2[i+3]) ^ sum;
    }
    
    printf("Test 2: sum = %d, buffer1[0] = %d\n", sum, buffer1[0]);
    
    free(buffer1);
    free(buffer2);
    free(fbuffer);
}

/* Third test with nested loops and complex control flow */
void test_function_3(int dim) {
    int matrix[64][64];
    int result[64][64];
    
    /* Initialize matrix */
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            matrix[i][j] = (i * 17 + j * 13) & 0xFF;
        }
    }
    
    /* Matrix operations with complex scheduling */
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            int val = 0;
            
            /* Complex computation with multiple dependencies */
            for (int k = 0; k < dim; k++) {
                int a = matrix[i][k];
                int b = matrix[k][j];
                
                /* Long dependency chain */
                int t1 = a * b;
                int t2 = t1 + (a ^ b);
                int t3 = t2 << (k & 3);
                int t4 = t3 >> 1;
                int t5 = t4 * global_seed;
                int t6 = t5 - (a & b);
                int t7 = t6 | (a | b);
                int t8 = t7 ^ (a + b);
                
                val += t8;
                
                /* Conditional operation */
                if (t8 > 1000) {
                    val -= t5;
                } else if (t8 < 100) {
                    val += t3;
                }
            }
            
            result[i][j] = val;
            
            /* More operations in same basic block */
            if (i > 0 && j > 0) {
                result[i][j] += result[i-1][j] + result[i][j-1];
            }
        }
    }
    
    /* Final reduction */
    int total = 0;
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            total ^= result[i][j];
            total += (result[i][j] * 3) >> 1;
            total |= result[i][j] & 0xFFFF;
        }
    }
    
    printf("Test 3: total = %d\n", total);
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    int iterations = 100;
    int size = 256;
    int dim = 32;
    
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) size = atoi(argv[2]);
    if (argc > 3) dim = atoi(argv[3]);
    
    printf("Starting scheduler context tests...\n");
    
    /* Call test functions multiple times to ensure scheduling happens */
    for (int run = 0; run < 3; run++) {
        test_function_1(iterations);
        test_function_2(size);
        test_function_3(dim);
    }
    
    printf("All tests completed.\n");
    return 0;
}
