/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * instruction scheduling scenarios that require:
 * 1. Target-specific scheduling hooks
 * 2. Frontend state saving
 * 3. Large instruction queues and ready lists
 * 4. Complex dependency chains and mixed operations
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

/* Function with side effects to create scheduling barriers */
static ALWAYS_INLINE int side_effect_func(int x) {
    /* Inline assembly creates a scheduling barrier */
    asm volatile("" : "+r"(x) : : "memory");
    return x ^ global_seed;
}

/* Complex integer computation with dependency chain */
static ALWAYS_INLINE int complex_int_chain(int a, int b, int c, int d, int e) {
    int t1 = a + b;
    int t2 = t1 * c;
    int t3 = t2 - d;
    int t4 = t3 / (e + 1);
    int t5 = t4 ^ t1;
    int t6 = t5 | t2;
    int t7 = t6 & t3;
    int t8 = t7 << 2;
    int t9 = t8 >> 1;
    return side_effect_func(t9);
}

/* Mixed integer and floating point operations */
static ALWAYS_INLINE float mixed_operations(int a, float b, double c, int d) {
    float f1 = (float)a * b;
    double d1 = (double)f1 + c;
    int i1 = (int)d1 * d;
    float f2 = (float)i1 / b;
    double d2 = (double)f2 - c;
    float f3 = (float)d2 + b;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return f3 * global_float;
}

/* Vector operations to create many parallel instructions */
static ALWAYS_INLINE v4si vector_operations(v4si a, v4si b, v4si c) {
    v4si r1 = a + b;
    v4si r2 = r1 * c;
    v4si r3 = r2 - a;
    v4si r4 = r3 & b;
    v4si r5 = r4 | c;
    v4si r6 = r5 << 1;
    v4si r7 = r6 >> 2;
    
    /* Interleave with scalar operations */
    int s1 = r7[0] + r7[1];
    int s2 = r7[2] * r7[3];
    int s3 = s1 - s2;
    
    /* Store results back to vector */
    r7[0] = s3;
    r7[1] = s1;
    r7[2] = s2;
    r7[3] = s3 ^ s1;
    
    return r7;
}

/* Function with loop that may trigger software pipelining */
static ALWAYS_INLINE double loop_pipelining(int iterations, double start) {
    double result = start;
    int i;
    
    /* Small loop that might be software pipelined */
    for (i = 0; i < iterations; i++) {
        result = result * 1.01;
        result = result + (double)i * 0.5;
        result = sin(result * 0.1);
        result = cos(result * 0.2);
        
        /* Conditional that creates control flow */
        if (i & 1) {
            result = result * 0.9;
        } else {
            result = result * 1.1;
        }
        
        /* Memory operation with potential aliasing */
        global_double = result * 0.5;
        result = result + global_double;
    }
    
    return result;
}

/* Wide basic block with many independent operations */
static ALWAYS_INLINE void wide_basic_block(int *results, float *floats, 
                                          double *doubles, int n) {
    /* Create many independent computation chains */
    int r1 = complex_int_chain(results[0], results[1], results[2], 
                              results[3], results[4]);
    float f1 = mixed_operations(r1, floats[0], doubles[0], results[5]);
    
    int r2 = complex_int_chain(results[5], results[6], results[7], 
                              results[8], results[9]);
    float f2 = mixed_operations(r2, floats[1], doubles[1], results[10]);
    
    int r3 = complex_int_chain(results[10], results[11], results[12], 
                              results[13], results[14]);
    float f3 = mixed_operations(r3, floats[2], doubles[2], results[15]);
    
    int r4 = complex_int_chain(results[15], results[16], results[17], 
                              results[18], results[19]);
    float f4 = mixed_operations(r4, floats[3], doubles[3], results[0]);
    
    /* Vector operations */
    v4si va = {r1, r2, r3, r4};
    v4si vb = {results[1], results[2], results[3], results[4]};
    v4si vc = {results[5], results[6], results[7], results[8]};
    v4si vr = vector_operations(va, vb, vc);
    
    /* Store results with memory operations */
    results[0] = r1 + vr[0];
    results[1] = r2 + vr[1];
    results[2] = r3 + vr[2];
    results[3] = r4 + vr[3];
    floats[0] = f1;
    floats[1] = f2;
    floats[2] = f3;
    floats[3] = f4;
    
    /* More independent operations */
    double d1 = loop_pipelining(4, doubles[0]);
    double d2 = loop_pipelining(4, doubles[1]);
    double d3 = loop_pipelining(4, doubles[2]);
    double d4 = loop_pipelining(4, doubles[3]);
    
    doubles[0] = d1;
    doubles[1] = d2;
    doubles[2] = d3;
    doubles[3] = d4;
}

/* Function with switch statement for complex control flow */
static ALWAYS_INLINE int switch_control_flow(int x, int y, int z) {
    int result = 0;
    
    /* Complex switch that may require state saving */
    switch (x % 8) {
        case 0:
            result = y + z;
            result = complex_int_chain(result, y, z, x, global_seed);
            break;
        case 1:
            result = y * z;
            result = side_effect_func(result);
            break;
        case 2:
            result = y - z;
            result = result ^ x;
            break;
        case 3:
            result = y / (z + 1);
            result = result | x;
            break;
        case 4:
            result = (y << 2) | (z >> 2);
            result = complex_int_chain(result, x, y, z, global_seed);
            break;
        case 5:
            result = y ^ z;
            result = side_effect_func(result);
            break;
        case 6:
            result = (y + z) * x;
            result = result & 0xFFFF;
            break;
        case 7:
            result = (y - z) ^ x;
            result = complex_int_chain(result, z, x, y, global_seed);
            break;
    }
    
    /* Additional computation after switch */
    float f = mixed_operations(result, global_float, global_double, x);
    result += (int)f;
    
    return result;
}

/* Main test function that combines all patterns */
static void test_scheduler_comprehensive(int iterations) {
    int int_array[32];
    float float_array[16];
    double double_array[16];
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < 32; i++) {
        int_array[i] = i + global_seed;
    }
    for (i = 0; i < 16; i++) {
        float_array[i] = (float)i * 0.5f + global_float;
        double_array[i] = (double)i * 0.25 + global_double;
    }
    
    long long checksum = 0;
    
    /* Multiple iterations with different scheduling patterns */
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Wide basic block with many instructions */
        wide_basic_block(int_array, float_array, double_array, 16);
        
        /* Pattern 2: Complex control flow with switch */
        for (j = 0; j < 8; j++) {
            int result = switch_control_flow(int_array[j], 
                                           int_array[j+8], 
                                           int_array[j+16]);
            int_array[j+24] = result;
            checksum += result;
        }
        
        /* Pattern 3: Loop with software pipelining potential */
        for (j = 0; j < 4; j++) {
            double result = loop_pipelining(8, double_array[j]);
            double_array[j+4] = result;
            checksum += (long long)result;
        }
        
        /* Pattern 4: Mixed operations with memory aliasing */
        for (j = 0; j < 4; j++) {
            float f = mixed_operations(int_array[j], 
                                      float_array[j], 
                                      double_array[j], 
                                      int_array[j+4]);
            float_array[j+4] = f;
            checksum += (long long)(f * 1000);
        }
        
        /* Update global variables to prevent dead code elimination */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
        global_float = sinf(global_float * 0.1f);
        global_double = cos(global_double * 0.05);
    }
    
    /* Final computation and output to prevent optimization */
    printf("Checksum: %lld\n", checksum);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = 0;
    for (i = 0; i < 32; i++) {
        final_result ^= int_array[i];
    }
    printf("Final XOR: %d\n", final_result);
}

/* Additional test functions with different characteristics */
static void test_vector_heavy(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si result;
    int i;
    
    for (i = 0; i < 1000; i++) {
        result = vector_operations(a, b, c);
        a = result + b;
        b = result + c;
        c = result + a;
        
        /* Scalar operations mixed in */
        int s = a[0] + b[1] + c[2];
        a[0] = s;
        b[1] = s ^ 0x55;
        c[2] = s * 2;
    }
    
    printf("Vector test: %d %d %d %d\n", 
           result[0], result[1], result[2], result[3]);
}

static void test_memory_aliasing(void) {
    int buffer[64];
    float fbuffer[32];
    int i, j;
    
    /* Initialize with overlapping indices */
    for (i = 0; i < 64; i++) {
        buffer[i] = i;
    }
    for (i = 0; i < 32; i++) {
        fbuffer[i] = i * 0.1f;
    }
    
    /* Operations with potential aliasing */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 16; j++) {
            /* These may alias */
            buffer[j*2] = complex_int_chain(buffer[j], 
                                          buffer[j+1], 
                                          buffer[j+2], 
                                          buffer[j+3], 
                                          buffer[j+4]);
            
            fbuffer[j] = mixed_operations(buffer[j], 
                                         fbuffer[j], 
                                         (double)buffer[j+1], 
                                         buffer[j+2]);
            
            /* Update with potential dependency */
            buffer[j+16] = buffer[j] + (int)fbuffer[j];
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    int sum = 0;
    for (i = 0; i < 64; i++) {
        sum += buffer[i];
    }
    printf("Memory test sum: %d\n", sum);
}

int main(void) {
    printf("Starting scheduler coverage tests...\n");
    
    /* Seed for reproducibility */
    srand(42);
    
    /* Test 1: Comprehensive scheduling patterns */
    printf("\nTest 1: Comprehensive scheduling patterns\n");
    test_scheduler_comprehensive(100);
    
    /* Test 2: Vector-heavy operations */
    printf("\nTest 2: Vector-heavy operations\n");
    test_vector_heavy();
    
    /* Test 3: Memory aliasing patterns */
    printf("\nTest 3: Memory aliasing patterns\n");
    test_memory_aliasing();
    
    /* Additional quick tests with different optimization barriers */
    printf("\nAdditional quick tests...\n");
    
    /* Force different execution paths */
    for (int i = 0; i < 10; i++) {
        int x = rand() % 100;
        int y = rand() % 100;
        int z = rand() % 100;
        
        int r = switch_control_flow(x, y, z);
        float f = mixed_operations(r, x * 0.5f, y * 0.25, z);
        
        printf("Quick test %d: %d, %.2f\n", i, r, f);
    }
    
    printf("\nAll tests completed.\n");
    return 0;
}
