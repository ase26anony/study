/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that force the scheduler to allocate and use:
 * 1. Target-specific scheduling hooks (targetm.sched.free_sched_context)
 * 2. Frontend state saving (current_sched_info->restore_state)
 * 3. Large instruction queues and ready lists
 * 4. Complex dependency chains requiring extensive scheduling
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Prevent excessive inlining that would simplify basic blocks */
#define NOINLINE __attribute__((noinline))

/* Vector types for creating wide operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent constant propagation */
volatile int g_seed = 42;
volatile float g_fseed = 3.14159f;
volatile double g_dseed = 2.71828;

/* Complex arithmetic dependency chain with mixed operations */
NOINLINE static int complex_dependency_chain(int a, int b, int c, int d, int e) {
    /* Creates a long dependency chain requiring careful scheduling */
    int t1 = a + b * c;
    int t2 = t1 - d / (e + 1);
    int t3 = t2 * (a - c) + (b << 2);
    int t4 = t3 ^ (d & 0xFF);
    int t5 = t4 * t3 - t2;
    int t6 = t5 / (t1 + 1) + (e << 3);
    int t7 = t6 | (a & b & c);
    int t8 = t7 * 3 - t5 / 2;
    int t9 = t8 + (d % 7) * t6;
    int t10 = t9 ^ (t4 << 1);
    
    /* Memory operations with potential aliasing */
    int* ptr = &t10;
    *ptr += 1;
    int result = *ptr * 2 - t9;
    
    /* Conditional to create control flow complexity */
    if (result > 1000) {
        result = result / 2 + t8;
    } else {
        result = result * 3 - t7;
    }
    
    return result;
}

/* Mixed integer and floating-point operations */
NOINLINE static float mixed_operations(int n, float* arr) {
    float sum = 0.0f;
    float prod = 1.0f;
    
    /* Loop with small iteration count for potential software pipelining */
    for (int i = 0; i < 8; i++) {
        /* Integer operations */
        int idx = (i * 7) % n;
        int mask = idx & 0xF;
        
        /* Floating-point operations with dependencies */
        float val = arr[idx] * g_fseed;
        float sin_val = sinf(val);
        float cos_val = cosf(val);
        
        /* Mixed type computations */
        sum += sin_val * (mask + 1);
        prod *= cos_val / (i + 1);
        
        /* More integer math */
        idx = (idx * 13 + 1) % n;
        arr[idx] = sum - prod;
    }
    
    /* Complex final computation */
    float result = sum * prod - sqrtf(fabsf(sum - prod));
    result += (float)((int)result % 256) / 255.0f;
    
    return result;
}

/* Function with SIMD/vector operations */
NOINLINE static v4si vector_operations(v4si a, v4si b, v4si c) {
    /* Multiple independent vector operation chains */
    v4si t1 = a + b * c;
    v4si t2 = a - b / (c + 1);
    v4si t3 = t1 * t2;
    v4si t4 = t3 << 2;
    v4si t5 = t4 & 0x7F;
    v4si t6 = t5 | (a & b);
    
    /* Cross-lane operations */
    int sum = t6[0] + t6[1] + t6[2] + t6[3];
    v4si t7 = t6 + sum;
    
    /* More vector operations */
    v4si t8 = t7 * 3 - t5 / 2;
    v4si t9 = t8 ^ (t4 << 1);
    
    return t9;
}

/* Wide basic block with unrolled loop */
NOINLINE static double wide_basic_block(double* data, int size) {
    /* Unrolled operations creating wide basic block */
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
    double prod1 = 1.0, prod2 = 1.0, prod3 = 1.0, prod4 = 1.0;
    
    /* Partially unrolled loop - creates many instructions */
    for (int i = 0; i < size; i += 4) {
        /* Independent computation paths */
        double d1 = data[i] * g_dseed;
        double d2 = data[i + 1] * g_dseed * 2.0;
        double d3 = data[i + 2] * g_dseed * 0.5;
        double d4 = data[i + 3] * g_dseed * 1.5;
        
        /* Complex operations on each path */
        sum1 += sin(d1) * cos(d1);
        sum2 += tan(d2) * atan(d2);
        sum3 += exp(d3) * log(fabs(d3) + 1.0);
        sum4 += sqrt(fabs(d4)) * pow(d4, 0.3);
        
        prod1 *= d1 * d1 + 1.0;
        prod2 *= d2 * d2 + 2.0;
        prod3 *= d3 * d3 + 3.0;
        prod4 *= d4 * d4 + 4.0;
        
        /* More operations to increase instruction count */
        data[i] = sum1 - prod1;
        data[i + 1] = sum2 - prod2;
        data[i + 2] = sum3 - prod3;
        data[i + 3] = sum4 - prod4;
    }
    
    /* Final reduction with many operations */
    double total = (sum1 + sum2 + sum3 + sum4) * 
                   (prod1 + prod2 + prod3 + prod4);
    total = total / (size + 1) * M_PI;
    
    /* Conditional with complex expression */
    if (total > 1000.0) {
        total = log(total) * exp(total / 1000.0);
    } else {
        total = sqrt(total) * pow(total, 1.5);
    }
    
    return total;
}

/* Function with inline assembly creating scheduling barriers */
NOINLINE static int asm_barriers(int x, int y) {
    int result = x;
    
    /* Inline assembly that acts as scheduling barrier */
    asm volatile ("# Scheduling barrier 1" : : : "memory");
    
    /* Complex integer computations */
    for (int i = 0; i < 6; i++) {
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
        result ^= y << (i % 16);
        result = (result >> 3) | (result << 29);
    }
    
    asm volatile ("# Scheduling barrier 2" : : : "memory");
    
    /* More computations */
    result = result % 10007;
    result = result * result - result;
    
    asm volatile ("# Scheduling barrier 3" : : : "memory");
    
    return result;
}

/* Main test function that calls all patterns */
NOINLINE static void run_scheduler_tests(void) {
    const int ARRAY_SIZE = 256;
    float* farr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* darr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        farr[i] = (float)((i * 17 + 23) % 100) / 10.0f;
        darr[i] = (double)((i * 13 + 7) % 200) / 20.0;
    }
    
    int int_result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    /* Test 1: Complex dependency chains */
    for (int i = 0; i < 100; i++) {
        int a = (i * 3) % 50;
        int b = (i * 5) % 50;
        int c = (i * 7) % 50;
        int d = (i * 11) % 50;
        int e = (i * 13) % 50;
        
        int_result ^= complex_dependency_chain(a, b, c, d, e);
    }
    
    /* Test 2: Mixed operations */
    for (int i = 0; i < 50; i++) {
        float_result += mixed_operations(ARRAY_SIZE, farr);
    }
    
    /* Test 3: Vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    v4si vec_result = {0, 0, 0, 0};
    
    for (int i = 0; i < 100; i++) {
        vec_a[0] = i;
        vec_b[1] = i * 2;
        vec_c[2] = i * 3;
        vec_result += vector_operations(vec_a, vec_b, vec_c);
    }
    
    /* Test 4: Wide basic block */
    for (int i = 0; i < 20; i++) {
        double_result += wide_basic_block(darr, ARRAY_SIZE);
    }
    
    /* Test 5: Assembly barriers */
    int asm_result = 0;
    for (int i = 0; i < 1000; i++) {
        asm_result += asm_barriers(i, int_result);
    }
    
    /* Combine results to prevent dead code elimination */
    int final_result = int_result + (int)float_result + 
                      (int)double_result + asm_result +
                      vec_result[0] + vec_result[1] + 
                      vec_result[2] + vec_result[3];
    
    printf("Scheduler test checksum: %d\n", final_result % 1000000);
    
    free(farr);
    free(darr);
}

/* Additional test functions with different patterns */
NOINLINE static void test_speculative_scheduling(void) {
    /* Function designed to trigger speculative scheduling */
    int array[64];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        array[i] = i * i - i;
    }
    
    /* Loop with conditional that might be speculatively scheduled */
    for (int i = 0; i < 63; i++) {
        int x = array[i];
        int y = array[i + 1];
        
        /* Complex conditional with many operations */
        if (x > y * 2) {
            /* Branch 1: Many dependent operations */
            int t1 = x * y + (x << 3);
            int t2 = t1 / (y + 1) - (x % 7);
            int t3 = t2 * t2 - t1;
            int t4 = t3 ^ (x & y);
            sum += t4 * 3 - t2;
        } else {
            /* Branch 2: Different set of operations */
            int t1 = y * x - (y >> 2);
            int t2 = t1 + (x * 3) / (y + 2);
            int t3 = (t2 << 1) | (x ^ y);
            int t4 = t3 * 5 + t1;
            sum += t4 / 2 + t3;
        }
        
        /* More operations outside condition */
        array[i] = sum % 1000;
    }
    
    /* Final computation with loop */
    int result = 0;
    for (int i = 0; i < 64; i++) {
        result = (result * 31 + array[i]) % 10007;
    }
    
    g_seed = result;
}

/* Function with switch statement for complex control flow */
NOINLINE static int switch_scheduling(int x) {
    int result = x;
    
    /* Switch with multiple cases creates complex control flow */
    switch (x % 8) {
        case 0:
            result = x * 2 + 1;
            result = result * result - x;
            break;
        case 1:
            result = x / 2 + 3;
            result = result | (x << 4);
            break;
        case 2:
            result = x * 3 - 5;
            result = result ^ 0xABCD;
            break;
        case 3:
            result = x + 7;
            result = result & 0xFF;
            result = result * 11;
            break;
        case 4:
            result = x - 2;
            result = result * result;
            result = result % 1009;
            break;
        case 5:
            result = x << 1;
            result = result + (x >> 3);
            result = result * 7;
            break;
        case 6:
            result = x ^ 0x1234;
            result = result * 13;
            result = result - x;
            break;
        case 7:
            result = x % 17;
            result = result * 19 + 1;
            break;
    }
    
    /* Additional computations after switch */
    result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    
    return result;
}

int main(void) {
    clock_t start = clock();
    
    printf("Starting scheduler coverage tests...\n");
    
    /* Run main scheduler tests */
    run_scheduler_tests();
    
    /* Run additional tests */
    test_speculative_scheduling();
    
    /* Test with switch statements */
    int switch_result = 0;
    for (int i = 0; i < 1000; i++) {
        switch_result += switch_scheduling(i);
    }
    printf("Switch scheduling result: %d\n", switch_result % 10000);
    
    /* More complex computations to ensure scheduling happens */
    volatile int checksum = g_seed;
    for (int i = 0; i < 10000; i++) {
        checksum = (checksum * 31 + i) % 1000000;
        checksum ^= (checksum << 13) | (checksum >> 19);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("All tests completed in %.3f seconds\n", elapsed);
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
