/* test_sched_cleanup.c
 * 
 * This program is designed to exercise GCC's Haifa scheduler cleanup code
 * in haifa-sched.cc lines 4681-4691. It creates complex basic blocks that
 * force the scheduler to allocate and use the full scheduling context,
 * including target-specific hooks, frontend state saving, and large
 * instruction queues.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Prevent excessive inlining to keep basic blocks large */
#define NOINLINE __attribute__((noinline))

/* Vector types for creating wide operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile float global_float = 3.14159f;
volatile double global_double = 2.71828;

/* ============================================
 * Function 1: Complex dependency chains with mixed operations
 * This creates long dependency chains that require careful scheduling
 * ============================================ */
NOINLINE static double complex_dependency_chain(int iterations) {
    double a = global_double;
    float b = global_float;
    int c = global_seed;
    double result = 0.0;
    
    /* Complex dependency chain with mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Integer operations */
        c = c * 1103515245 + 12345;
        int d = c >> 16;
        int e = d * d;
        int f = e + c;
        
        /* Floating point operations with dependencies */
        b = b * 1.1f + sinf(b);
        a = a * 1.01 + cos(a);
        
        /* Mixed type operations */
        double g = a * (double)b;
        double h = g / (double)(f & 0xFFF);
        
        /* More dependencies */
        result += h * (i + 1);
        a = a - h * 0.1;
        b = b + (float)h * 0.01f;
    }
    
    return result;
}

/* ============================================
 * Function 2: Wide basic block with independent parallel chains
 * This fills the instruction queue and ready list
 * ============================================ */
NOINLINE static float wide_parallel_operations(int size) {
    /* Large arrays to work with */
    float arr1[256], arr2[256], arr3[256];
    int int_arr1[256], int_arr2[256];
    
    /* Initialize with some values */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 0.1f;
        arr2[i] = i * 0.2f;
        int_arr1[i] = i * 3;
        int_arr2[i] = i * 5;
    }
    
    float sum = 0.0f;
    
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < size; i += 8) {
        /* Independent floating point chains */
        float f1 = arr1[i] * arr2[i] + arr1[i+1];
        float f2 = arr1[i+1] * arr2[i+1] - arr1[i];
        float f3 = arr1[i+2] / (arr2[i+2] + 1.0f);
        float f4 = arr1[i+3] * arr1[i+3] - arr2[i+3];
        
        /* Independent integer chains */
        int i1 = int_arr1[i] + int_arr2[i];
        int i2 = int_arr1[i+1] * int_arr2[i+1];
        int i3 = int_arr1[i+2] - int_arr2[i+2];
        int i4 = int_arr1[i+3] & int_arr2[i+3];
        
        /* More mixed operations */
        float f5 = f1 * (float)i1 + f2;
        float f6 = f3 * (float)i2 - f4;
        float f7 = (float)i3 * f1 / f2;
        float f8 = (float)i4 + f3 * f4;
        
        /* Memory operations with potential aliasing */
        arr3[i] = f5;
        arr3[i+1] = f6;
        arr3[i+2] = f7;
        arr3[i+3] = f8;
        
        /* Conditional update to create control flow complexity */
        if (f5 > f6) {
            sum += f5 - f6;
        } else {
            sum += f6 - f5;
        }
        
        if (f7 < f8) {
            sum += f8 * 0.5f;
        }
        
        /* More operations to widen the block */
        arr1[i] = f6 * 0.9f;
        arr2[i+1] = f7 * 1.1f;
        int_arr1[i+2] = (int)f8;
        int_arr2[i+3] = (int)(f5 * 2.0f);
    }
    
    return sum;
}

/* ============================================
 * Function 3: Vector operations using GCC vector extensions
 * These expand to multiple parallel operations during scheduling
 * ============================================ */
NOINLINE static v4sf vector_operations(v4sf a, v4sf b, v4si mask) {
    v4sf result;
    
    /* Multiple vector operations creating scheduling pressure */
    v4sf c = a + b;
    v4sf d = a * b;
    v4sf e = c - d;
    v4sf f = __builtin_shuffle(c, d, (v4si){0, 1, 2, 3});
    
    /* Conditional vector operations */
    v4sf g = (v4sf)(mask != (v4si){0, 0, 0, 0}) ? e : f;
    
    /* More vector math */
    v4sf h = __builtin_ia32_sqrtps(g);
    v4sf i = __builtin_ia32_rsqrtps(h);
    
    /* Mixed precision */
    v4si j = __builtin_convertvector(i, v4si);
    v4sf k = __builtin_convertvector(j, v4sf);
    
    result = k * g + h - i;
    
    return result;
}

/* ============================================
 * Function 4: Function with speculative scheduling opportunities
 * Contains conditional branches that may be speculatively scheduled
 * ============================================ */
NOINLINE static double speculative_scheduling(int *data, int size) {
    double total = 0.0;
    int count = 0;
    
    /* Loop with conditional that encourages speculative scheduling */
    for (int i = 0; i < size; i++) {
        int val = data[i];
        
        /* Complex conditional chain */
        if (val > 100) {
            double dval = sqrt((double)val);
            total += dval * dval;
            count++;
            
            /* Nested conditionals */
            if (val > 200) {
                total += sin(dval) * 2.0;
                if (val > 300) {
                    total += cos(dval) * 3.0;
                }
            }
        } else if (val > 50) {
            total += log((double)val + 1.0);
            count += 2;
        } else {
            total += (double)val * 0.5;
        }
        
        /* Another conditional with different computation */
        if ((val & 1) == 0) {
            total += (double)(val * val) * 0.01;
        } else {
            total -= (double)(val / 2) * 0.005;
        }
    }
    
    /* Switch statement for additional control flow complexity */
    switch (count % 4) {
        case 0:
            total *= 1.1;
            break;
        case 1:
            total *= 0.9;
            break;
        case 2:
            total += 100.0;
            break;
        case 3:
            total -= 50.0;
            break;
    }
    
    return total;
}

/* ============================================
 * Function 5: Memory intensive with aliasing concerns
 * Uses pointer arithmetic that may alias
 * ============================================ */
NOINLINE static void memory_operations_with_aliasing(float *restrict a, 
                                                     float *restrict b, 
                                                     float *restrict c,
                                                     int n) {
    /* Use restrict to help compiler, but create complex access patterns */
    for (int i = 0; i < n - 4; i++) {
        /* Strided accesses that might alias */
        float t1 = a[i] * b[i];
        float t2 = a[i+1] + b[i+1];
        float t3 = a[i+2] - b[i+2];
        float t4 = a[i+3] / (b[i+3] + 1.0f);
        
        /* Cross-dependent operations */
        c[i] = t1 + t2 * t3;
        c[i+1] = t2 - t3 / t4;
        c[i+2] = t3 * t4 + t1;
        c[i+3] = t4 / (t1 + 1.0f) - t2;
        
        /* Update source arrays creating loop-carried dependencies */
        a[i] = c[i] * 0.9f;
        b[i+1] = c[i+1] * 1.1f;
        a[i+2] += c[i+2] * 0.5f;
        b[i+3] -= c[i+3] * 0.3f;
    }
}

/* ============================================
 * Function 6: Inline assembly to create scheduling barriers
 * ============================================ */
NOINLINE static int assembly_barriers(int x) {
    int result = x;
    
    /* Inline assembly that acts as scheduling barrier */
    asm volatile ("# Assembly barrier 1" : : : "memory");
    
    /* Complex computation between barriers */
    for (int i = 0; i < 16; i++) {
        result = result * 1103515245 + 12345;
        result = (result >> 16) & 0x7FFF;
    }
    
    asm volatile ("# Assembly barrier 2" : : : "memory");
    
    /* More computation */
    result = result * result;
    result = result - (result / 3);
    
    asm volatile ("# Assembly barrier 3" : : : "memory");
    
    return result;
}

/* ============================================
 * Main driver function
 * Calls all test functions to exercise scheduler
 * ============================================ */
int main() {
    clock_t start = clock();
    double total_result = 0.0;
    
    printf("Starting scheduler stress test...\n");
    
    /* Test 1: Complex dependency chains */
    printf("Running complex dependency chains...\n");
    total_result += complex_dependency_chain(1000);
    
    /* Test 2: Wide parallel operations */
    printf("Running wide parallel operations...\n");
    total_result += (double)wide_parallel_operations(256);
    
    /* Test 3: Vector operations */
    printf("Running vector operations...\n");
    v4sf vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4si vec_mask = {0, 1, 0, 1};
    v4sf vec_result = vector_operations(vec_a, vec_b, vec_mask);
    total_result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* Test 4: Speculative scheduling */
    printf("Running speculative scheduling test...\n");
    int data[500];
    for (int i = 0; i < 500; i++) {
        data[i] = (i * 17) % 400;
    }
    total_result += speculative_scheduling(data, 500);
    
    /* Test 5: Memory operations with aliasing */
    printf("Running memory operations with aliasing...\n");
    float arr_a[300], arr_b[300], arr_c[300];
    for (int i = 0; i < 300; i++) {
        arr_a[i] = i * 0.01f;
        arr_b[i] = i * 0.02f;
    }
    memory_operations_with_aliasing(arr_a, arr_b, arr_c, 300);
    for (int i = 0; i < 10; i++) {
        total_result += arr_c[i];
    }
    
    /* Test 6: Assembly barriers */
    printf("Running assembly barrier test...\n");
    total_result += (double)assembly_barriers(123456);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total result: %f\n", total_result);
    printf("Elapsed time: %f seconds\n", elapsed);
    printf("Scheduler stress test completed.\n");
    
    /* Use the result to prevent dead code elimination */
    if (total_result > 0.0) {
        return 0;
    } else {
        return 1;
    }
}
