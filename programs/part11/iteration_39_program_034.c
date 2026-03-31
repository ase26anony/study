/* test_sched_cleanup.c
 * 
 * This program is designed to trigger the cleanup logic in GCC's Haifa scheduler,
 * specifically the free_sched_block function that frees scheduling context.
 * It creates complex basic blocks that force the scheduler to allocate and use:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
 * 3. Large instruction queues and ready lists
 * 4. Complex dependency chains
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
volatile double global_result = 0.0;

/* Function with inline assembly to create scheduling barriers */
static inline ALWAYS_INLINE int barrier_op(int x) {
    int result;
    /* Memory barrier that scheduler must respect */
    asm volatile("" : "+r"(x) : : "memory");
    result = x * 2 + 1;
    asm volatile("" : "+r"(result) : : "memory");
    return result;
}

/* Complex integer computation with dependency chain */
static ALWAYS_INLINE int complex_int_chain(int a, int b, int c, int d, int e) {
    /* Long dependency chain */
    int t1 = a + b * 3;
    int t2 = t1 - c / 2;
    int t3 = t2 * d + barrier_op(e);
    int t4 = t3 ^ (t1 << 2);
    int t5 = t4 + barrier_op(t2);
    int t6 = t5 * 7 - t3;
    int t7 = t6 / 4 + barrier_op(t4);
    int t8 = t7 ^ t5;
    int t9 = t8 * 11 - t6;
    int t10 = t9 + barrier_op(t7);
    return t10;
}

/* Mixed integer and floating-point operations */
static ALWAYS_INLINE double mixed_operations(int a, float b, double c, int d) {
    /* Interleaved int and FP ops */
    double r1 = (double)a * 1.5;
    float r2 = b * 2.0f + (float)d;
    int r3 = a * d + (int)r2;
    double r4 = c + r1 * 0.5;
    float r5 = r2 * 3.0f - (float)r3;
    double r6 = r4 / 2.0 + (double)r5;
    int r7 = r3 ^ barrier_op(a);
    double r8 = r6 * (double)r7 + sin(r4);
    return r8 + cos(r6);
}

/* Function with small loop that might be software pipelined */
static ALWAYS_INLINE double small_loop_pipeline(int iterations) {
    double acc = 0.0;
    float f_acc = 0.0f;
    int i_acc = 0;
    
    /* Small loop that scheduler might try to pipeline */
    for (int i = 0; i < iterations; i++) {
        /* Dependent operations within loop */
        f_acc += sinf((float)i * 0.1f) * 2.0f;
        i_acc += barrier_op(i) * 3;
        acc += (double)f_acc * 0.5 + (double)i_acc;
        
        /* Conditional that might cause speculative scheduling */
        if (i_acc > 1000) {
            f_acc *= 0.9f;
            i_acc = barrier_op(i_acc / 2);
        }
    }
    return acc + (double)f_acc;
}

/* Wide basic block with many independent operations */
static ALWAYS_INLINE void wide_basic_block(int *results, const int *inputs, int n) {
    /* Many independent chains to fill ready list */
    int chain1 = inputs[0];
    int chain2 = inputs[1];
    int chain3 = inputs[2];
    int chain4 = inputs[3];
    float fchain1 = (float)inputs[4];
    float fchain2 = (float)inputs[5];
    double dchain1 = (double)inputs[6];
    double dchain2 = (double)inputs[7];
    
    /* Unrolled independent computations */
    for (int i = 0; i < n; i += 8) {
        /* Integer chains */
        chain1 = chain1 * 3 + barrier_op(inputs[i % n]);
        chain2 = chain2 / 2 - barrier_op(inputs[(i+1) % n]);
        chain3 = chain3 ^ (chain1 << 1);
        chain4 = chain4 + chain2 * 7;
        
        /* Floating point chains */
        fchain1 = fchain1 * 1.1f + sinf((float)i);
        fchain2 = fchain2 * 0.9f - cosf((float)(i+1));
        dchain1 = dchain1 * 1.01 + log(1.0 + (double)i);
        dchain2 = dchain2 * 0.99 - exp((double)(i+1) * -0.1);
        
        /* Memory operations with potential aliasing */
        results[i % n] = chain1 + chain3;
        results[(i+1) % n] = chain2 - chain4;
        results[(i+2) % n] = (int)(fchain1 + fchain2);
        results[(i+3) % n] = (int)(dchain1 * 100.0);
        
        /* More operations to increase block size */
        chain1 = barrier_op(chain1);
        chain3 = barrier_op(chain3);
        fchain1 = fchain1 * 2.0f - 1.0f;
        dchain2 = dchain2 + dchain1 * 0.5;
    }
}

/* Function using vector extensions for parallel operations */
static ALWAYS_INLINE v4sf vector_operations(v4sf a, v4sf b, v4si mask) {
    /* Vector operations that expand during scheduling */
    v4sf r1 = a + b;
    v4sf r2 = a * b;
    v4sf r3 = r1 - r2;
    v4sf r4 = __builtin_shuffle(r1, r2, (v4si){0, 1, 2, 3});
    
    /* Conditional vector operations */
    v4sf r5 = __builtin_ia32_blendvps(r3, r4, (v4sf)mask);
    v4sf r6 = r5 * (v4sf){1.1f, 0.9f, 1.2f, 0.8f};
    
    /* More vector ops to increase scheduling complexity */
    v4sf r7 = __builtin_ia32_sqrtps(r6);
    v4sf r8 = __builtin_ia32_rcpps(r7);
    
    return r8 + r6 * 0.5f;
}

/* Complex function with switch statement for state tracking */
static ALWAYS_INLINE double complex_control_flow(int mode, double x) {
    double result = x;
    
    /* Switch with multiple cases creates control flow */
    switch (mode % 5) {
        case 0:
            result = sin(x) * cos(x);
            result += barrier_op((int)(result * 1000));
            break;
        case 1:
            result = exp(x * 0.5);
            result -= barrier_op((int)(result * 100));
            break;
        case 2:
            result = log(fabs(x) + 1.0);
            result *= barrier_op((int)(result * 10));
            break;
        case 3:
            result = x * x * x;
            result /= barrier_op((int)(result)) + 1;
            break;
        case 4:
            result = sqrt(fabs(x));
            result = barrier_op((int)(result * 10000)) / 10000.0;
            break;
    }
    
    /* Additional computations after switch */
    for (int i = 0; i < 4; i++) {
        result += small_loop_pipeline(3);
        result = mixed_operations((int)result, (float)result, result, i);
    }
    
    return result;
}

/* Main test function 1: Complex dependency chains */
double test_function_1(int iterations) {
    double total = 0.0;
    int int_state = global_seed;
    float float_state = (float)global_seed * 0.5f;
    double double_state = (double)global_seed * 0.25;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex integer chain */
        int_state = complex_int_chain(int_state, i, i*2, i*3, i*4);
        
        /* Mixed operations */
        double_state = mixed_operations(int_state, float_state, double_state, i);
        
        /* Small loop that might be pipelined */
        float_state += (float)small_loop_pipeline(6);
        
        /* Vector operations */
        v4sf vec_a = (v4sf){float_state, float_state*0.5f, float_state*0.25f, float_state*0.125f};
        v4sf vec_b = (v4sf){(float)i, (float)(i+1), (float)(i+2), (float)(i+3)};
        v4si mask = (v4si){i, ~i, i^0xFF, i|0x0F};
        v4sf vec_result = vector_operations(vec_a, vec_b, mask);
        
        /* Extract result from vector */
        float vec_sum = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        total += (double)vec_sum + double_state + (double)int_state;
        
        /* Complex control flow */
        total += complex_control_flow(int_state, total);
    }
    
    return total;
}

/* Main test function 2: Wide basic blocks */
double test_function_2(int size) {
    int *array1 = (int*)malloc(size * sizeof(int));
    int *array2 = (int*)malloc(size * sizeof(int));
    double total = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        array1[i] = global_seed + i;
        array2[i] = 0;
    }
    
    /* Execute wide basic block multiple times */
    for (int iter = 0; iter < 10; iter++) {
        wide_basic_block(array2, array1, size);
        
        /* Process results with mixed operations */
        for (int i = 0; i < size; i += 4) {
            double val = mixed_operations(array1[i], (float)array2[i], 
                                         (double)array1[i+1], array2[i+1]);
            total += val + complex_control_flow(array2[i+2], val);
        }
    }
    
    free(array1);
    free(array2);
    return total;
}

/* Main test function 3: Nested loops with software pipelining */
double test_function_3(int outer, int inner) {
    double total = 0.0;
    int int_state = 1;
    
    for (int i = 0; i < outer; i++) {
        float float_state = (float)i * 0.1f;
        
        /* Inner loop that might be software pipelined */
        for (int j = 0; j < inner; j++) {
            /* Dependent operations in loop */
            int_state = complex_int_chain(int_state, j, i, j*i, j+i);
            float_state += (float)int_state * 0.01f;
            
            /* Mixed operations creating scheduling pressure */
            double dbl_val = mixed_operations(int_state, float_state, 
                                            (double)j * 0.1, i);
            
            /* Small conditional loop */
            if (j % 3 == 0) {
                dbl_val += small_loop_pipeline(4);
            }
            
            total += dbl_val;
            
            /* Memory operation with barrier */
            asm volatile("" : "+r"(int_state), "+r"(total) : : "memory");
        }
        
        /* After inner loop, complex control flow */
        total += complex_control_flow(int_state, total);
    }
    
    return total;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    double result1, result2, result3;
    clock_t start, end;
    
    printf("Starting scheduling stress test...\n");
    
    /* Test 1: Complex dependency chains */
    start = clock();
    result1 = test_function_1(100);
    end = clock();
    printf("Test 1 completed in %.2f seconds, result: %.6f\n", 
           (double)(end - start) / CLOCKS_PER_SEC, result1);
    
    /* Test 2: Wide basic blocks */
    start = clock();
    result2 = test_function_2(256);
    end = clock();
    printf("Test 2 completed in %.2f seconds, result: %.6f\n", 
           (double)(end - start) / CLOCKS_PER_SEC, result2);
    
    /* Test 3: Nested loops */
    start = clock();
    result3 = test_function_3(50, 20);
    end = clock();
    printf("Test 3 completed in %.2f seconds, result: %.6f\n", 
           (double)(end - start) / CLOCKS_PER_SEC, result3);
    
    /* Final computation using all results */
    double final_result = result1 * 0.3 + result2 * 0.4 + result3 * 0.3;
    global_result = final_result;
    
    printf("Final combined result: %.12f\n", final_result);
    printf("Global result stored: %.12f\n", global_result);
    
    return (final_result > 0.0) ? 0 : 1;
}
