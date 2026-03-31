/* test_sched_coverage.c
 * 
 * This program is designed to trigger the uncovered cleanup code in
 * haifa-sched.cc's free_sched_block function by creating complex
 * basic blocks that force the scheduler to allocate and use:
 * 1. Target-specific scheduling hooks (targetm.sched.free_sched_context)
 * 2. Frontend state saving (current_sched_info->restore_state)
 * 3. Large instruction queues and ready lists
 * 4. Complex dependency chains requiring state tracking
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
volatile float global_float_result = 0.0f;
volatile double global_double_result = 0.0;

/* Function with side effects to create scheduling barriers */
static ALWAYS_INLINE int side_effect_function(int x) {
    /* Inline assembly creates a scheduling barrier */
    asm volatile("" : "+r" (x) : : "memory");
    return x * 1103515245 + 12345;
}

/* Complex integer computation with dependencies */
static ALWAYS_INLINE void test_integer_dep_chain(int *arr, int n) {
    int a = arr[0];
    int b = arr[1];
    int c = arr[2];
    int d = arr[3];
    int e = arr[4];
    
    /* Long dependency chain */
    a = b + c;          /* 1 */
    d = a * e;          /* 2 - depends on 1 */
    c = d - b;          /* 3 - depends on 2 */
    e = c / (a + 1);    /* 4 - depends on 1, 3 */
    b = e << 2;         /* 5 - depends on 4 */
    a = b ^ c;          /* 6 - depends on 3, 5 */
    d = a | e;          /* 7 - depends on 4, 6 */
    c = d & b;          /* 8 - depends on 5, 7 */
    
    /* Store results with side effects */
    arr[0] = side_effect_function(a);
    arr[1] = side_effect_function(b);
    arr[2] = side_effect_function(c);
    arr[3] = side_effect_function(d);
    arr[4] = side_effect_function(e);
}

/* Mixed integer and floating-point operations */
static ALWAYS_INLINE void test_mixed_operations(float *farr, int *iarr, int n) {
    float f1 = farr[0];
    float f2 = farr[1];
    float f3 = farr[2];
    int i1 = iarr[0];
    int i2 = iarr[1];
    int i3 = iarr[2];
    
    /* Interleaved FP and integer ops */
    f1 = f2 * f3 + 1.5f;            /* FP op */
    i1 = i2 + i3 * 2;               /* Integer op */
    f2 = sinf(f1) * cosf(f3);       /* FP math lib call */
    i2 = (i1 << 3) | (i3 & 0xFF);   /* Integer bit ops */
    f3 = f1 / f2 - 0.5f;            /* FP op */
    i3 = i1 * i2 + global_seed;     /* Integer with global */
    
    /* More mixed operations */
    for (int j = 0; j < 4; j++) {
        f1 = f1 + f2 * (float)i1;
        i1 = i1 + (int)f3;
        f2 = f2 - f3 / (float)(i2 + 1);
        i2 = i2 ^ (int)(f1 * 100.0f);
    }
    
    farr[0] = f1;
    farr[1] = f2;
    farr[2] = f3;
    iarr[0] = i1;
    iarr[1] = i2;
    iarr[2] = i3;
}

/* Function with speculative scheduling opportunities */
static ALWAYS_INLINE int test_speculative_scheduling(int *arr, int n) {
    int result = 0;
    
    /* Complex conditional with multiple dependent operations */
    for (int i = 0; i < n; i++) {
        int x = arr[i];
        int y = arr[(i + 1) % n];
        
        /* Long dependency chain before branch */
        x = x * 3 + 7;
        y = y / 2 - 1;
        int z = x ^ y;
        z = z << (x & 0x3);
        z = z + global_seed;
        
        /* Branch that might be speculatively scheduled */
        if (z > 1000) {
            result += z * 2;
            x = side_effect_function(x);
        } else if (z > 500) {
            result += z / 2;
            y = side_effect_function(y);
        } else {
            result += z;
            z = side_effect_function(z);
        }
        
        /* More operations after branch */
        arr[i] = x + y + z;
    }
    
    return result;
}

/* Wide basic block with many independent operations */
static ALWAYS_INLINE void test_wide_basic_block(double *darr, int size) {
    /* Many independent computation paths */
    double a = darr[0];
    double b = darr[1];
    double c = darr[2];
    double d = darr[3];
    double e = darr[4];
    double f = darr[5];
    double g = darr[6];
    double h = darr[7];
    
    /* Independent chains that can be reordered */
    a = b * c + sin(d);                     /* Chain 1 */
    b = cos(e) * tan(f);                    /* Chain 2 */
    c = sqrt(g) * log(h + 1.0);             /* Chain 3 */
    d = exp(a) * atan(b);                   /* Chain 4 */
    e = pow(c, 2.0) + asin(d);              /* Chain 5 */
    f = sinh(e) * cosh(f);                  /* Chain 6 */
    g = tanh(g) * acos(h);                  /* Chain 7 */
    h = fmod(a, b) + remainder(c, d);       /* Chain 8 */
    
    /* More independent operations */
    double t1 = a + b;
    double t2 = c - d;
    double t3 = e * f;
    double t4 = g / h;
    double t5 = t1 * t2;
    double t6 = t3 + t4;
    double t7 = t5 - t6;
    double t8 = t7 * global_double_result;
    
    /* Vector operations (SIMD-like) */
    v2df v1 = {a, b};
    v2df v2 = {c, d};
    v2df v3 = {e, f};
    v2df v4 = {g, h};
    
    v2df vr1 = v1 + v2 * v3;
    v2df vr2 = v4 - v1 / v2;
    v2df vr3 = vr1 * vr2 + v3;
    
    /* Store results */
    darr[0] = a + ((double*)&vr1)[0];
    darr[1] = b + ((double*)&vr1)[1];
    darr[2] = c + ((double*)&vr2)[0];
    darr[3] = d + ((double*)&vr2)[1];
    darr[4] = e + ((double*)&vr3)[0];
    darr[5] = f + ((double*)&vr3)[1];
    darr[6] = g + t8;
    darr[7] = h + global_double_result;
}

/* Function with software pipelining potential */
static ALWAYS_INLINE double test_software_pipelining(double *arr, int n) {
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    
    /* Small loop that might be software pipelined */
    for (int i = 0; i < 8; i++) {  /* Small iteration count */
        double x = arr[i];
        double y = arr[(i + 1) % n];
        double z = arr[(i + 2) % n];
        
        /* Dependent operations within loop */
        x = x * 1.5 + sin(y);
        y = y / 2.0 - cos(z);
        z = z * 3.0 + tan(x);
        
        /* Accumulate with different dependencies */
        sum1 += x * y;
        sum2 += y * z;
        sum3 += z * x;
        
        /* Conditional update */
        if (sum1 > sum2) {
            x = side_effect_function((int)x);
        }
        
        arr[i] = x + y + z;
    }
    
    return sum1 + sum2 + sum3;
}

/* Main test function with multiple complex basic blocks */
static void test_complex_scheduling(int iterations) {
    /* Allocate arrays with different alignments */
    int *int_arr = (int*)aligned_alloc(64, 256 * sizeof(int));
    float *float_arr = (float*)aligned_alloc(32, 128 * sizeof(float));
    double *double_arr = (double*)aligned_alloc(64, 128 * sizeof(double));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        if (i < 128) {
            float_arr[i] = (float)(i * 0.1f);
            double_arr[i] = (double)(i * 0.01);
        }
    }
    
    double total_result = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Basic Block 1: Integer dependency chain */
        test_integer_dep_chain(int_arr, 5);
        
        /* Basic Block 2: Mixed operations */
        test_mixed_operations(float_arr, int_arr, 3);
        
        /* Basic Block 3: Speculative scheduling */
        int spec_result = test_speculative_scheduling(int_arr + 10, 16);
        total_result += spec_result;
        
        /* Basic Block 4: Wide basic block */
        test_wide_basic_block(double_arr, 8);
        
        /* Basic Block 5: Software pipelining */
        double pipe_result = test_software_pipelining(double_arr + 16, 24);
        total_result += pipe_result;
        
        /* Update global variables to prevent dead code elimination */
        global_float_result += float_arr[iter % 128];
        global_double_result += double_arr[iter % 128];
        
        /* Side effect to prevent reordering across iterations */
        asm volatile("" : : : "memory");
    }
    
    /* Compute checksum */
    unsigned long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum ^= (unsigned long)int_arr[i];
        checksum = (checksum << 1) | (checksum >> 63);
    }
    
    printf("Checksum: %lu, Total result: %f\n", checksum, total_result);
    
    free(int_arr);
    free(float_arr);
    free(double_arr);
}

/* Additional test with vector operations for SIMD scheduling */
static void test_vector_scheduling(void) {
    v4si va = {1, 2, 3, 4};
    v4si vb = {5, 6, 7, 8};
    v4si vc = {9, 10, 11, 12};
    v4sf vf1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vf2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Multiple vector operations creating parallel dependency chains */
    for (int i = 0; i < 100; i++) {
        va = va + vb * vc;
        vb = vb - vc / (va + 1);
        vc = vc & va | vb;
        
        vf1 = vf1 * vf2 + (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
        vf2 = vf2 / vf1 - (v4sf){0.5f, 0.6f, 0.7f, 0.8f};
        
        /* Scalar operations mixed with vector */
        int scalar = ((int*)&va)[0] + ((int*)&vb)[1];
        scalar = side_effect_function(scalar);
        
        /* Store to memory with potential aliasing */
        ((int*)&va)[i % 4] = scalar;
    }
    
    /* Use results */
    global_seed += ((int*)&va)[0];
    global_float_result += ((float*)&vf1)[0];
}

/* Test with computed goto for state tracking */
static void test_computed_goto(int *arr, int n) {
    void *labels[] = {&&label0, &&label1, &&label2, &&label3};
    
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int idx = arr[i] % 4;
        goto *labels[idx];
        
    label0:
        sum += arr[i] * 2;
        arr[i] = side_effect_function(arr[i]);
        continue;
        
    label1:
        sum += arr[i] / 2;
        arr[i] = side_effect_function(arr[i] + 1);
        continue;
        
    label2:
        sum += arr[i] << 1;
        arr[i] = side_effect_function(arr[i] * 2);
        continue;
        
    label3:
        sum += arr[i] >> 1;
        arr[i] = side_effect_function(arr[i] - 1);
        continue;
    }
    
    global_seed = sum;
}

int main(void) {
    printf("Starting scheduler coverage test...\n");
    
    /* Test 1: Complex scheduling with multiple basic blocks */
    test_complex_scheduling(100);
    
    /* Test 2: Vector operations for SIMD scheduling */
    test_vector_scheduling();
    
    /* Test 3: Computed goto for state tracking */
    int goto_arr[64];
    for (int i = 0; i < 64; i++) {
        goto_arr[i] = i * 3 + 7;
    }
    test_computed_goto(goto_arr, 64);
    
    /* Test 4: Large unrolled loop */
    {
        double large_arr[256];
        for (int i = 0; i < 256; i++) {
            large_arr[i] = i * 0.25;
        }
        
        /* Manually unrolled computation */
        for (int i = 0; i < 256; i += 8) {
            /* 8 independent chains */
            large_arr[i] = large_arr[i] * 1.1 + sin(large_arr[i+1]);
            large_arr[i+1] = large_arr[i+1] / 1.2 - cos(large_arr[i+2]);
            large_arr[i+2] = large_arr[i+2] * 1.3 + tan(large_arr[i+3]);
            large_arr[i+3] = large_arr[i+3] / 1.4 - atan(large_arr[i+4]);
            large_arr[i+4] = large_arr[i+4] * 1.5 + exp(large_arr[i+5]);
            large_arr[i+5] = large_arr[i+5] / 1.6 - log(large_arr[i+6]+1.0);
            large_arr[i+6] = large_arr[i+6] * 1.7 + sqrt(large_arr[i+7]);
            large_arr[i+7] = large_arr[i+7] / 1.8 - pow(large_arr[i], 0.5);
        }
        
        double sum = 0.0;
        for (int i = 0; i < 256; i++) {
            sum += large_arr[i];
        }
        printf("Large array sum: %f\n", sum);
    }
    
    printf("Test completed. Global results: %d, %f, %f\n", 
           global_seed, global_float_result, global_double_result);
    
    return 0;
}
