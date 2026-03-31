/* test_scheduler_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that require extensive instruction scheduling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* Prevent excessive optimization */
#define NOINLINE __attribute__((noinline))

/* Vector types for creating wide operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Global variables to prevent constant propagation */
volatile int global_seed = 42;
volatile float global_float_seed = 3.14159f;
volatile double global_double_seed = 2.71828;

/* Complex arithmetic dependency chain with mixed operations */
NOINLINE static int complex_dependency_chain(int a, int b, int c, int d, int e) {
    /* Creates a long dependency chain forcing careful scheduling */
    int t1 = a * b + c;
    int t2 = t1 / (d + 1);
    int t3 = t2 << (e & 3);
    int t4 = t3 ^ (t1 * 2);
    int t5 = t4 - (t2 >> 1);
    int t6 = t5 * t3 + t4;
    int t7 = t6 & 0x7FFFFFFF;
    int t8 = t7 | (t5 << 2);
    int t9 = t8 ^ t6;
    int t10 = t9 + t7 - t8;
    
    /* Mix with floating point to use different execution units */
    float ft1 = (float)t10 * 1.5f;
    float ft2 = ft1 * ft1 - 2.0f * ft1;
    int t11 = t10 + (int)ft2;
    
    /* Another integer chain */
    t11 = t11 * 3 + 7;
    t11 = t11 ^ (t11 >> 16);
    t11 = t11 * 1103515245 + 12345;
    
    return t11 & 0xFF;
}

/* Function with memory aliasing and pointer arithmetic */
NOINLINE static void memory_aliasing_operations(int* arr1, int* arr2, int size) {
    /* Potential aliasing prevents reordering */
    for (int i = 0; i < size - 1; i++) {
        arr1[i] = arr2[i + 1] * 2;
        arr2[i] = arr1[i] + arr1[i + 1];
        arr1[i + 1] = arr2[i] - 3;
    }
    
    /* Independent operations to fill ready list */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    for (int i = 0; i < size; i += 4) {
        sum1 += arr1[i];
        sum2 += arr2[i];
        sum3 += arr1[i + 1];
        sum4 += arr2[i + 1];
    }
    
    /* Force dependency across sums */
    arr1[0] = sum1 + sum2;
    arr2[0] = sum3 + sum4;
}

/* Function using vector operations to create wide basic blocks */
NOINLINE static v4si vector_operations(v4si a, v4si b, v4si c) {
    /* Multiple independent vector operations */
    v4si r1 = a + b;
    v4si r2 = a * c;
    v4si r3 = b - c;
    v4si r4 = r1 & r2;
    v4si r5 = r3 | r4;
    v4si r6 = r1 * r2 + r3;
    v4si r7 = r4 - r5 * r6;
    v4si r8 = r7 << 2;
    v4si r9 = r8 >> 1;
    v4si r10 = r9 ^ r7;
    
    /* Mix with scalar operations */
    int s1 = r10[0] + r10[1];
    int s2 = r10[2] * r10[3];
    int s3 = s1 ^ s2;
    
    /* Create dependency back to vector */
    v4si result = r10 + (v4si){s3, s3, s3, s3};
    return result;
}

/* Function with speculative scheduling opportunities */
NOINLINE static int speculative_scheduling(int x, int y, int z) {
    int result = 0;
    
    /* Multiple conditional updates in sequence */
    if (x > 0) {
        result += x * 2;
        /* Inline asm to create scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    if (y < 100) {
        result -= y / 3;
        /* Complex floating point operation */
        float temp = sqrtf(fabsf((float)result));
        result += (int)temp;
    }
    
    if (z != 0) {
        result *= (z & 1) ? 3 : 5;
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Loop with small iteration count for software pipelining */
    for (int i = 0; i < 8; i++) {
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
        if (result & 1) {
            result ^= 0x55555555;
        }
    }
    
    return result;
}

/* Large unrolled loop creating wide basic block */
NOINLINE static int wide_basic_block(int iterations) {
    int a = 1, b = 2, c = 3, d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    
    /* Unrolled computation with multiple independent chains */
    for (int i = 0; i < iterations; i++) {
        /* Chain 1 */
        a = a * b + i;
        b = b - c * a;
        c = c ^ (d + b);
        d = d | (a << 2);
        
        /* Chain 2 (independent from chain 1) */
        e = e + f * 3;
        f = f / (g + 1);
        g = g & (h - 1);
        h = h ^ (e >> 1);
        
        /* Chain 3 (mixes with others) */
        int t1 = a + e;
        int t2 = b + f;
        int t3 = c + g;
        int t4 = d + h;
        
        /* Floating point operations mixed in */
        float ft1 = (float)t1 * 0.5f;
        float ft2 = (float)t2 * 1.5f;
        double dt1 = (double)t3 * 0.25;
        double dt2 = (double)t4 * 0.75;
        
        /* Convert back and create dependencies */
        a = t1 + (int)ft1;
        b = t2 + (int)ft2;
        c = t3 + (int)dt1;
        d = t4 + (int)dt2;
    }
    
    return a + b + c + d + e + f + g + h;
}

/* Function with function calls creating scheduling boundaries */
NOINLINE static int function_call_mix(int x) {
    /* Call complex functions to create scheduling context */
    int r1 = complex_dependency_chain(x, x+1, x+2, x+3, x+4);
    
    /* Local computation */
    int r2 = x * 3 + 7;
    r2 = r2 ^ (r2 >> 4);
    r2 = r2 * 1664525 + 1013904223;
    
    /* Another call */
    int arr1[16], arr2[16];
    for (int i = 0; i < 16; i++) {
        arr1[i] = i + x;
        arr2[i] = i * x;
    }
    memory_aliasing_operations(arr1, arr2, 16);
    
    /* More computation */
    r2 += arr1[0] + arr2[0];
    
    /* Vector operation */
    v4si v1 = {x, x+1, x+2, x+3};
    v4si v2 = {x+4, x+5, x+6, x+7};
    v4si v3 = {x+8, x+9, x+10, x+11};
    v4si vr = vector_operations(v1, v2, v3);
    
    r2 += vr[0] + vr[1] + vr[2] + vr[3];
    
    return r1 + r2;
}

/* Main test driver with multiple scheduling scenarios */
int main() {
    clock_t start = clock();
    int total_result = 0;
    
    printf("Starting scheduler coverage test...\n");
    
    /* Test 1: Complex dependency chains */
    printf("Test 1: Complex dependency chains\n");
    for (int i = 0; i < 100; i++) {
        total_result ^= complex_dependency_chain(i, i*2, i*3, i*4, i*5);
    }
    
    /* Test 2: Memory operations with aliasing */
    printf("Test 2: Memory aliasing operations\n");
    int arr1[64], arr2[64];
    for (int i = 0; i < 64; i++) {
        arr1[i] = i;
        arr2[i] = 64 - i;
    }
    for (int i = 0; i < 10; i++) {
        memory_aliasing_operations(arr1, arr2, 64);
        total_result += arr1[0] + arr2[0];
    }
    
    /* Test 3: Vector operations */
    printf("Test 3: Vector operations\n");
    v4si vresult = {0, 0, 0, 0};
    for (int i = 0; i < 50; i++) {
        v4si va = {i, i+1, i+2, i+3};
        v4si vb = {i*2, i*3, i*4, i*5};
        v4si vc = {i*6, i*7, i*8, i*9};
        v4si vr = vector_operations(va, vb, vc);
        vresult = vresult + vr;
    }
    total_result += vresult[0] + vresult[1] + vresult[2] + vresult[3];
    
    /* Test 4: Speculative scheduling */
    printf("Test 4: Speculative scheduling\n");
    for (int i = 0; i < 200; i++) {
        total_result += speculative_scheduling(i, i*2, i*3);
    }
    
    /* Test 5: Wide basic blocks */
    printf("Test 5: Wide basic blocks\n");
    for (int i = 0; i < 5; i++) {
        total_result += wide_basic_block(20);
    }
    
    /* Test 6: Function call mix */
    printf("Test 6: Function call mix\n");
    for (int i = 0; i < 100; i++) {
        total_result += function_call_mix(i);
    }
    
    /* Test 7: Mixed floating point and integer */
    printf("Test 7: Mixed floating point operations\n");
    double fp_acc = 0.0;
    for (int i = 1; i <= 1000; i++) {
        float f1 = (float)i * 0.1f;
        double d1 = (double)i * 0.01;
        
        /* Mixed precision chain */
        f1 = f1 * f1 - 2.0f * f1 + 1.0f;
        d1 = d1 * d1 - 3.0 * d1 + 2.0;
        
        /* Convert and mix with integer */
        int i1 = (int)f1;
        int i2 = (int)d1;
        
        /* More floating point */
        f1 = sqrtf(fabsf(f1));
        d1 = sqrt(fabs(d1));
        
        /* Final mix */
        fp_acc += (double)f1 + d1 + (double)i1 + (double)i2;
    }
    total_result += (int)fp_acc;
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nTest completed in %.3f seconds\n", elapsed);
    printf("Final checksum: %d (0x%08x)\n", total_result, total_result);
    
    /* Verify result isn't trivial */
    if (total_result == 0) {
        printf("WARNING: Result is zero - possible over-optimization\n");
    }
    
    return total_result != 0 ? 0 : 1;
}
