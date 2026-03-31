/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in GCC's
 * Haifa scheduler (haifa-sched.cc lines 4681-4691) by creating complex
 * basic blocks that force the scheduler to allocate and use:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
 * 3. Large instruction queues and ready lists
 * 4. Complex scheduling contexts
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

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* ============================================
   Function 1: Mixed Integer and FP Operations
   Creates dependency chains and parallel paths
   ============================================ */
static ALWAYS_INLINE int complex_integer_chain(int a, int b, int c, int d, int e) {
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
    return t9;
}

static ALWAYS_INLINE float complex_float_chain(float a, float b, float c, float d, float e) {
    /* FP dependency chain */
    float t1 = a + b;
    float t2 = t1 * c;
    float t3 = t2 - d;
    float t4 = t3 / e;
    float t5 = sinf(t4);
    float t6 = cosf(t5);
    float t7 = t6 * t5;
    float t8 = t7 + t4;
    return t8;
}

/* Function with speculative scheduling opportunities */
int test_mixed_operations(int iterations) {
    int result = 0;
    float fp_result = 0.0f;
    
    /* Complex basic block with mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Integer dependency chain */
        int int_val = complex_integer_chain(i, i+1, i+2, i+3, i+4);
        
        /* Floating-point dependency chain */
        float fp_val = complex_float_chain(i*0.1f, (i+1)*0.1f, 
                                          (i+2)*0.1f, (i+3)*0.1f,
                                          (i+4)*0.1f);
        
        /* Independent parallel computations */
        int parallel1 = (i * 3) / 2;
        int parallel2 = (i << 1) ^ 0xFF;
        float parallel3 = sqrtf(i * 1.0f);
        double parallel4 = log(i + 1);
        
        /* Conditional that creates scheduling barriers */
        if (int_val % 3 == 0) {
            result += int_val + parallel1;
            fp_result += fp_val + parallel3;
        } else if (int_val % 3 == 1) {
            result += int_val * parallel2;
            fp_result += fp_val * parallel4;
        } else {
            result += int_val | parallel1;
            fp_result += fp_val - parallel3;
        }
        
        /* Memory operations with potential aliasing */
        volatile int* mem_ptr = &global_seed;
        *mem_ptr = *mem_ptr + i;
        
        /* Function call with side effects */
        fp_result += sinf(cosf(fp_result));
    }
    
    return result + (int)fp_result;
}

/* ============================================
   Function 2: Vector Operations with Unrolling
   Creates wide basic blocks for large queues
   ============================================ */
int test_vector_operations(int size) {
    /* Create vector arrays */
    v4si vec_a[32], vec_b[32], vec_c[32];
    v4sf vec_f[32], vec_g[32];
    
    /* Initialize vectors */
    for (int i = 0; i < 32; i++) {
        vec_a[i] = (v4si){i, i+1, i+2, i+3};
        vec_b[i] = (v4si){i*2, i*3, i*4, i*5};
        vec_f[i] = (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
        vec_g[i] = (v4sf){i*0.5f, i*0.6f, i*0.7f, i*0.8f};
    }
    
    int total = 0;
    float fp_total = 0.0f;
    
    /* UNROLLED loop - creates very wide basic block */
    /* Each iteration processes multiple vectors independently */
    for (int i = 0; i < size; i++) {
        /* Process 8 vectors in parallel - creates many independent ops */
        vec_c[0] = vec_a[0] + vec_b[0];
        vec_c[1] = vec_a[1] * vec_b[1];
        vec_c[2] = vec_a[2] - vec_b[2];
        vec_c[3] = vec_a[3] & vec_b[3];
        vec_c[4] = vec_a[4] | vec_b[4];
        vec_c[5] = vec_a[5] ^ vec_b[5];
        vec_c[6] = vec_a[6] << 1;
        vec_c[7] = vec_a[7] >> 2;
        
        /* More vector operations */
        v4sf vec_h[8];
        vec_h[0] = vec_f[0] + vec_g[0];
        vec_h[1] = vec_f[1] * vec_g[1];
        vec_h[2] = vec_f[2] - vec_g[2];
        vec_h[3] = vec_f[3] / (vec_g[3] + 1.0f);
        
        /* Extract and sum results */
        int temp[4];
        memcpy(temp, &vec_c[0], sizeof(temp));
        total += temp[0] + temp[1] + temp[2] + temp[3];
        
        float ftemp[4];
        memcpy(ftemp, &vec_h[0], sizeof(ftemp));
        fp_total += ftemp[0] + ftemp[1] + ftemp[2] + ftemp[3];
        
        /* Rotate arrays to create different dependencies */
        v4si temp_vec = vec_a[0];
        for (int j = 0; j < 31; j++) {
            vec_a[j] = vec_a[j+1];
        }
        vec_a[31] = temp_vec;
    }
    
    return total + (int)fp_total;
}

/* ============================================
   Function 3: Memory Intensive with Aliasing
   Creates scheduling barriers and state saving
   ============================================ */
int test_memory_aliasing(int* array, int size) {
    int sum = 0;
    
    /* Complex memory access pattern with potential aliasing */
    for (int i = 0; i < size - 4; i++) {
        /* Multiple dependent memory operations */
        int val1 = array[i];
        int val2 = array[i+1];
        int val3 = array[i+2];
        int val4 = array[i+3];
        
        /* Computation with dependencies */
        int t1 = val1 * val2;
        int t2 = val3 + val4;
        int t3 = t1 - t2;
        int t4 = t3 ^ val1;
        
        /* Store back with offset - creates WAR/WAW dependencies */
        array[i+1] = t4;
        array[i+2] = t1;
        
        /* Pointer arithmetic that could alias */
        int* ptr1 = array + i;
        int* ptr2 = array + (i % 8);
        *ptr1 = *ptr1 + *ptr2;
        
        /* Conditional store */
        if (t4 > 1000) {
            array[i+3] = t4 / 2;
        } else {
            array[i+3] = t4 * 2;
        }
        
        sum += array[i] + array[i+1] + array[i+2] + array[i+3];
        
        /* Inline assembly to create scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

/* ============================================
   Function 4: Nested Loops with Small Iterations
   Triggers software pipelining attempts
   ============================================ */
int test_nested_loops(int outer, int inner) {
    int total = 0;
    
    /* Outer loop with small inner loop - may trigger software pipelining */
    for (int i = 0; i < outer; i++) {
        int local_sum = 0;
        
        /* Small inner loop with dependent operations */
        for (int j = 0; j < inner; j++) {
            /* Create dependency chain across iterations */
            local_sum = local_sum * 3 + j;
            
            /* Mixed operations */
            float fp_val = sinf(j * 0.1f) * cosf(i * 0.1f);
            local_sum += (int)(fp_val * 100);
            
            /* Conditional inside loop */
            if ((i + j) % 7 == 0) {
                local_sum = local_sum ^ 0xABCD;
            }
        }
        
        total += local_sum;
        
        /* Function call that can't be inlined (creates scheduling barrier) */
        total = abs(total);
    }
    
    return total;
}

/* ============================================
   Function 5: Switch Statement with Computed Goto
   Forces frontend state saving
   ============================================ */
int test_switch_complex(int value, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex switch with multiple cases */
        switch ((value + i) % 8) {
            case 0: {
                /* Case with complex computations */
                int a = i * 2;
                int b = a + 3;
                int c = b * a;
                result += c - 5;
                break;
            }
            case 1: {
                /* Different computation pattern */
                float f = i * 0.5f;
                f = sinf(f) * cosf(f);
                result += (int)(f * 100);
                break;
            }
            case 2: {
                /* Memory operations */
                volatile int* p = &global_seed;
                result += *p + i;
                break;
            }
            case 3: {
                /* Vector operations */
                v4si v1 = {i, i+1, i+2, i+3};
                v4si v2 = {i*2, i*3, i*4, i*5};
                v4si v3 = v1 + v2;
                int temp[4];
                memcpy(temp, &v3, sizeof(temp));
                result += temp[0] + temp[1];
                break;
            }
            case 4: {
                /* Nested conditionals */
                if (i % 3 == 0) {
                    result += i * 7;
                } else if (i % 3 == 1) {
                    result += i / 3;
                } else {
                    result += i ^ 0xFF;
                }
                break;
            }
            default: {
                /* Default case with loop */
                for (int j = 0; j < 3; j++) {
                    result += (i + j) * 11;
                }
                break;
            }
        }
        
        /* Additional computation after switch */
        result = (result * 13) % 1000000;
    }
    
    return result;
}

/* ============================================
   Function 6: Matrix Operations
   Creates 2D access patterns and complex addressing
   ============================================ */
void test_matrix_operations(int size) {
    /* Dynamically allocate matrices */
    float* matrix_a = (float*)malloc(size * size * sizeof(float));
    float* matrix_b = (float*)malloc(size * size * sizeof(float));
    float* matrix_c = (float*)malloc(size * size * sizeof(float));
    
    if (!matrix_a || !matrix_b || !matrix_c) {
        free(matrix_a);
        free(matrix_b);
        free(matrix_c);
        return;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < size * size; i++) {
        matrix_a[i] = (i % 100) * 0.01f;
        matrix_b[i] = ((i + 50) % 100) * 0.01f;
    }
    
    /* Matrix multiplication - triple nested loops */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float sum = 0.0f;
            
            /* Inner loop with FP multiply-add chain */
            for (int k = 0; k < size; k++) {
                /* Complex addressing calculations */
                int idx_a = i * size + k;
                int idx_b = k * size + j;
                
                /* FP operation with dependency */
                sum += matrix_a[idx_a] * matrix_b[idx_b];
                
                /* Additional computation to increase complexity */
                sum = sum + sinf(matrix_a[idx_a] * 0.01f) * 0.001f;
            }
            
            matrix_c[i * size + j] = sum;
            
            /* Conditional store */
            if (sum > 50.0f) {
                matrix_c[i * size + j] = sqrtf(sum);
            }
        }
    }
    
    /* Cleanup */
    free(matrix_a);
    free(matrix_b);
    free(matrix_c);
}

/* ============================================
   Main Driver Function
   Calls all test functions with various parameters
   ============================================ */
int main() {
    printf("Starting scheduler coverage test...\n");
    
    int total_result = 0;
    clock_t start, end;
    
    /* Seed for reproducible results */
    srand(global_seed);
    
    /* Test 1: Mixed operations */
    start = clock();
    total_result += test_mixed_operations(1000);
    end = clock();
    printf("Test 1 completed in %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Test 2: Vector operations */
    start = clock();
    total_result += test_vector_operations(500);
    end = clock();
    printf("Test 2 completed in %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Test 3: Memory aliasing */
    int array[2048];
    for (int i = 0; i < 2048; i++) {
        array[i] = rand() % 1000;
    }
    start = clock();
    total_result += test_memory_aliasing(array, 2048);
    end = clock();
    printf("Test 3 completed in %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Test 4: Nested loops */
    start = clock();
    total_result += test_nested_loops(100, 8);  /* Small inner loop */
    end = clock();
    printf("Test 4 completed in %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Test 5: Switch statement */
    start = clock();
    total_result += test_switch_complex(42, 1000);
    end = clock();
    printf("Test 5 completed in %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Test 6: Matrix operations */
    start = clock();
    test_matrix_operations(64);  /* 64x64 matrix */
    end = clock();
    printf("Test 6 completed in %f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Final checksum to prevent dead code elimination */
    printf("Final checksum: %d\n", total_result);
    printf("All tests completed. Check coverage of haifa-sched.cc lines 4681-4691\n");
    
    return total_result != 0 ? 0 : 1;
}
