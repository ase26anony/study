/* test_sched_coverage.c
 * 
 * This program is designed to trigger the specific cleanup logic in GCC's
 * Haifa scheduler (free_sched_block function) by creating complex basic
 * blocks that require extensive instruction scheduling with:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
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

/* Vector types for creating parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* ======================================================================
 * Test Function 1: Complex Arithmetic with Mixed Types
 * Creates long dependency chains and parallel computation paths
 * ====================================================================== */
static ALWAYS_INLINE float test1_inner(float a, float b, float c, float d) {
    /* Complex dependency chain with mixed operations */
    float t1 = a * b + c;
    float t2 = t1 / (d + 1.0f);
    float t3 = sinf(t2) * cosf(t1);
    float t4 = t3 * t3 - t2 * t2;
    float t5 = sqrtf(fabsf(t4)) + 1.0f;
    return t5;
}

void test_function1(int iterations) {
    float result = 0.0f;
    
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < iterations; i += 4) {
        /* Multiple independent computation paths */
        float a1 = test1_inner(i * 1.1f, i * 2.2f, i * 3.3f, i * 4.4f);
        float a2 = test1_inner(i * 5.5f, i * 6.6f, i * 7.7f, i * 8.8f);
        float a3 = test1_inner(i * 9.9f, i * 10.1f, i * 11.2f, i * 12.3f);
        float a4 = test1_inner(i * 13.4f, i * 14.5f, i * 15.6f, i * 16.7f);
        
        /* Cross dependencies between paths */
        result += a1 * a2 - a3 / a4;
        result = fmodf(result, 1000.0f);
        
        /* Integer operations mixed with float */
        int int_op = (int)result * 17 + i;
        result += (float)int_op * 0.01f;
    }
    
    global_float = result;
}

/* ======================================================================
 * Test Function 2: Vector Operations with SIMD-like Computations
 * Triggers target-specific scheduling hooks for vector instructions
 * ====================================================================== */
void test_function2(int size) {
    /* Use vector extensions to create parallel operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_result = {0, 0, 0, 0};
    
    /* Complex vector computation chain */
    for (int i = 0; i < size; i++) {
        /* Multiple vector operations creating scheduling pressure */
        v4si temp1 = vec_a * vec_b + vec_c;
        v4si temp2 = vec_a + vec_b * vec_c;
        v4si temp3 = temp1 - temp2;
        vec_result = vec_result + temp3 * vec_a;
        
        /* Scalar operations mixed with vector */
        int scalar = i * 3 + 7;
        vec_a += scalar;
        vec_b -= scalar / 2;
        
        /* Conditional that may cause speculative scheduling */
        if (i % 8 == 0) {
            vec_c = vec_c << 1;
        } else {
            vec_c = vec_c >> 1;
        }
    }
    
    /* Store result to prevent elimination */
    int* res_ptr = (int*)&vec_result;
    global_seed = res_ptr[0] + res_ptr[1] + res_ptr[2] + res_ptr[3];
}

/* ======================================================================
 * Test Function 3: Memory Intensive with Aliasing
 * Creates memory dependencies and potential aliasing issues
 * ====================================================================== */
void test_function3(int* array_a, int* array_b, int* array_c, int size) {
    /* Complex memory access pattern with potential aliasing */
    for (int i = 0; i < size - 4; i++) {
        /* Multiple dependent memory operations */
        int val1 = array_a[i] * array_b[i + 1];
        int val2 = array_c[i + 2] + array_a[i + 3];
        int val3 = array_b[i] - array_c[i + 1];
        
        /* Cross-store dependencies */
        array_a[i + 1] = val1 + val2;
        array_b[i + 2] = val2 - val3;
        array_c[i + 3] = val3 * val1;
        
        /* Floating point computation in memory loop */
        float fp_val = sqrtf((float)val1) * global_float;
        array_a[i] += (int)fp_val;
    }
    
    /* Additional computation to widen the basic block */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += array_a[i] + array_b[i] - array_c[i];
    }
    global_seed = sum;
}

/* ======================================================================
 * Test Function 4: Mixed Integer/FP with Function Calls
 * Creates scheduling barriers and requires state saving
 * ====================================================================== */
static ALWAYS_INLINE double complex_math(double x, double y, int n) {
    /* Function with side effects through globals */
    double result = x;
    for (int i = 0; i < n; i++) {
        result = sin(result) * cos(y) + tan(x + y);
        global_double += 0.001;  /* Side effect */
    }
    return result;
}

void test_function4(int count) {
    double total = 0.0;
    
    /* Unrolled computation with function calls */
    for (int i = 0; i < count; i += 2) {
        /* Multiple function calls creating scheduling barriers */
        double r1 = complex_math(i * 0.5, i * 0.3, 3);
        double r2 = complex_math(i * 0.7, i * 0.2, 4);
        double r3 = complex_math(i * 0.9, i * 0.5, 2);
        double r4 = complex_math(i * 1.1, i * 0.8, 3);
        
        /* Dependent computations */
        total += r1 * r2 - r3 / r4;
        total = fmod(total, 100.0);
        
        /* Integer operations mixed in */
        int int_val = (int)total * 13 + i;
        total += (double)int_val * 0.001;
        
        /* Conditional that may trigger speculative scheduling */
        if (total > 50.0) {
            total -= 25.0;
        } else {
            total += 25.0;
        }
    }
    
    global_double = total;
}

/* ======================================================================
 * Test Function 5: Wide Basic Block with Many Independent Operations
 * Fills instruction queues and ready lists
 * ====================================================================== */
void test_function5(int iterations) {
    /* Many independent variables to create parallel computation paths */
    float a = 1.0f, b = 2.0f, c = 3.0f, d = 4.0f;
    float e = 5.0f, f = 6.0f, g = 7.0f, h = 8.0f;
    float i = 9.0f, j = 10.0f, k = 11.0f, l = 12.0f;
    float m = 13.0f, n = 14.0f, o = 15.0f, p = 16.0f;
    
    /* Unrolled computation with many independent operations */
    for (int iter = 0; iter < iterations; iter++) {
        /* 16 parallel computation chains */
        a = a * 1.1f + iter * 0.1f;
        b = b / 1.2f - iter * 0.2f;
        c = sinf(c) + cosf(iter * 0.3f);
        d = d * d - iter * 0.4f;
        e = sqrtf(fabsf(e)) + iter * 0.5f;
        f = f * 2.0f / (iter + 1);
        g = logf(fabsf(g) + 1.0f) + iter * 0.7f;
        h = h + h * 0.1f - iter * 0.8f;
        i = i * 0.9f + sinf(iter * 0.9f);
        j = j / 1.1f + cosf(iter * 1.0f);
        k = k * 1.2f - iter * 1.1f;
        l = l + sqrtf(l) + iter * 1.2f;
        m = m * 0.8f + iter * 1.3f;
        n = n / 1.3f - iter * 1.4f;
        o = o * 1.4f + iter * 1.5f;
        p = p * 0.7f - iter * 1.6f;
        
        /* Occasional dependencies to create scheduling complexity */
        if (iter % 8 == 0) {
            float sum = a + b + c + d + e + f + g + h;
            float prod = i * j * k * l * m * n * o * p;
            global_float = sum / prod;
        }
    }
    
    /* Final reduction */
    float final_result = a + b + c + d + e + f + g + h + 
                        i + j + k + l + m + n + o + p;
    global_float = final_result;
}

/* ======================================================================
 * Test Function 6: Switch Statement with Multiple Cases
 * Creates complex control flow requiring state tracking
 * ====================================================================== */
int test_function6(int value, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch creates multiple basic blocks */
        switch ((value + i) % 7) {
            case 0:
                result += i * 3;
                result = result * 2 - 5;
                break;
            case 1:
                result += i * 7;
                result = result / 2 + 10;
                break;
            case 2:
                result += i * 11;
                result = result * 3 - 15;
                /* Fall through */
            case 3:
                result += i * 13;
                result = result + 20;
                break;
            case 4:
                result += i * 17;
                result = result * 5 / 3;
                break;
            case 5:
                result += i * 19;
                result = result - 30;
                break;
            case 6:
                result += i * 23;
                result = result % 100;
                break;
        }
        
        /* Additional computation in each iteration */
        float fp_temp = sqrtf((float)result);
        result += (int)fp_temp;
        
        /* Memory operation */
        global_seed = result;
    }
    
    return result;
}

/* ======================================================================
 * Main Driver Function
 * Calls all test functions with various parameters
 * ====================================================================== */
int main() {
    clock_t start = clock();
    
    /* Initialize arrays for memory tests */
    const int ARRAY_SIZE = 1024;
    int* array_a = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* array_b = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* array_c = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = i;
        array_b[i] = i * 2;
        array_c[i] = i * 3;
    }
    
    printf("Starting scheduler coverage tests...\n");
    
    /* Run all test functions to trigger different scheduling scenarios */
    test_function1(128);      /* Mixed arithmetic with dependencies */
    test_function2(64);       /* Vector operations for target hooks */
    test_function3(array_a, array_b, array_c, ARRAY_SIZE); /* Memory ops */
    test_function4(96);       /* Function calls with side effects */
    test_function5(32);       /* Wide basic blocks */
    int switch_result = test_function6(global_seed, 48); /* Complex control flow */
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_seed + (int)global_float + (int)global_double + switch_result;
    
    /* Use results to prevent optimization */
    printf("Results: seed=%d, float=%.3f, double=%.3f, switch=%d\n",
           global_seed, global_float, global_double, switch_result);
    printf("Checksum: %d\n", checksum);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %.3f seconds\n", elapsed);
    
    free(array_a);
    free(array_b);
    free(array_c);
    
    return checksum % 256;
}
