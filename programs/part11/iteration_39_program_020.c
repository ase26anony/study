/* test_sched_context.c - Complex scheduling test for GCC Haifa scheduler */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for parallel operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile function to prevent optimization */
static volatile int global_counter = 0;

/* Function with side effects to create scheduling barriers */
static ALWAYS_INLINE int side_effect_func(int x) {
    global_counter += x;
    return x * 2;
}

/* Complex integer computation with dependency chain */
static ALWAYS_INLINE int complex_int_chain(int a, int b, int c, int d, int e) {
    int t1 = a + b;          /* 1st operation */
    int t2 = t1 * c;         /* Depends on t1 */
    int t3 = t2 - d;         /* Depends on t2 */
    int t4 = t3 ^ e;         /* Depends on t3 */
    int t5 = t4 << 2;        /* Depends on t4 */
    int t6 = t5 >> 1;        /* Depends on t5 */
    int t7 = t6 + side_effect_func(t5); /* Function call barrier */
    int t8 = t7 * 3;         /* Depends on t7 */
    return t8;
}

/* Mixed integer/float computation */
static ALWAYS_INLINE float mixed_operations(int a, float b, double c, int d) {
    float f1 = (float)a * b;          /* int to float conversion */
    double d1 = (double)f1 + c;       /* float to double */
    int i1 = (int)d1 * d;             /* double to int */
    float f2 = (float)i1 / b;         /* int to float */
    double d2 = d1 * f2;              /* mixed precision */
    float f3 = (float)d2 + b * 2.0f;  /* more mixing */
    return f3 + side_effect_func(a);  /* barrier */
}

/* Memory-intensive computation with potential aliasing */
static ALWAYS_INLINE void memory_ops(int* arr1, int* arr2, int* arr3, int size) {
    for (int i = 0; i < size; i++) {
        /* Multiple dependent memory operations */
        int val1 = arr1[i];
        int val2 = arr2[i];
        int val3 = val1 + val2;
        arr3[i] = val3 * 2;
        
        /* Potential aliasing - compiler can't reorder easily */
        arr1[(i + 1) % size] = val3;
        arr2[(i + 2) % size] = val3 + 1;
    }
}

/* Vector operations to create many parallel instructions */
static ALWAYS_INLINE v4sf vector_ops(v4sf a, v4sf b, v4sf c) {
    v4sf r1 = a + b;          /* SIMD add */
    v4sf r2 = r1 * c;         /* SIMD mul (depends on r1) */
    v4sf r3 = r2 - a;         /* SIMD sub (depends on r2) */
    v4sf r4 = r3 / b;         /* SIMD div (depends on r3) */
    v4sf r5 = __builtin_ia32_sqrtps(r4); /* SIMD sqrt */
    return r5;
}

/* Complex basic block with many independent chains */
static ALWAYS_INLINE int wide_basic_block(int a, int b, int c, int d, 
                                          int e, int f, int g, int h) {
    /* Multiple independent computation chains */
    int chain1 = complex_int_chain(a, b, c, d, e);
    int chain2 = complex_int_chain(b, c, d, e, f);
    int chain3 = complex_int_chain(c, d, e, f, g);
    int chain4 = complex_int_chain(d, e, f, g, h);
    int chain5 = complex_int_chain(e, f, g, h, a);
    int chain6 = complex_int_chain(f, g, h, a, b);
    int chain7 = complex_int_chain(g, h, a, b, c);
    int chain8 = complex_int_chain(h, a, b, c, d);
    
    /* Mix them together */
    int result = chain1 + chain2 - chain3 * chain4;
    result += chain5 ^ chain6 | chain7 & chain8;
    
    /* More operations to increase block size */
    result = (result << 3) | (result >> 29);
    result += side_effect_func(result);
    result *= 1103515245;
    result += 12345;
    
    return result & 0x7FFFFFFF;
}

/* Loop with small iteration count for potential software pipelining */
static ALWAYS_INLINE int speculative_loop(int iterations, int seed) {
    int result = seed;
    
    /* Small loop that might be software pipelined */
    for (int i = 0; i < iterations; i++) {
        /* Complex operations with dependencies */
        int t1 = result * 1664525;
        int t2 = t1 + 1013904223;
        int t3 = t2 ^ (t2 >> 16);
        int t4 = t3 * 1103515245;
        int t5 = t4 + 12345;
        
        /* Conditional that might cause speculative scheduling */
        if (t5 & 1) {
            result = t5 + side_effect_func(i);
        } else {
            result = t5 - side_effect_func(i);
        }
        
        /* Memory operation to force state */
        volatile int* mem = &global_counter;
        *mem = result;
    }
    
    return result;
}

/* Function with switch statement for complex control flow */
static ALWAYS_INLINE int switch_block(int x) {
    int result = 0;
    
    switch (x & 7) {
        case 0:
            result = complex_int_chain(x, 1, 2, 3, 4);
            break;
        case 1:
            result = complex_int_chain(x, 5, 6, 7, 8);
            break;
        case 2:
            result = wide_basic_block(x, x+1, x+2, x+3, x+4, x+5, x+6, x+7);
            break;
        case 3:
            result = speculative_loop(4, x);
            break;
        case 4:
            result = x * x + side_effect_func(x);
            break;
        case 5:
            result = (x << 4) | (x >> 28);
            break;
        case 6:
            result = x ^ 0xAAAAAAAA;
            break;
        case 7:
            result = ~x;
            break;
    }
    
    return result;
}

/* Main test function with multiple complex basic blocks */
static int test_function_1(int iterations) {
    int checksum = 0;
    
    /* Block 1: Wide basic block */
    checksum += wide_basic_block(iterations, iterations+1, iterations+2,
                                 iterations+3, iterations+4, iterations+5,
                                 iterations+6, iterations+7);
    
    /* Block 2: Mixed operations with function calls */
    float fval = mixed_operations(iterations, 1.5f, 2.71828, 42);
    checksum += (int)fval;
    
    /* Block 3: Memory operations */
    int arr1[16], arr2[16], arr3[16];
    for (int i = 0; i < 16; i++) {
        arr1[i] = i + iterations;
        arr2[i] = i * iterations;
    }
    memory_ops(arr1, arr2, arr3, 16);
    for (int i = 0; i < 16; i++) {
        checksum += arr3[i];
    }
    
    /* Block 4: Vector operations */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec_result = vector_ops(vec1, vec2, vec3);
    
    /* Extract results from vector */
    float vec_array[4];
    memcpy(vec_array, &vec_result, sizeof(vec_result));
    checksum += (int)(vec_array[0] + vec_array[1] + vec_array[2] + vec_array[3]);
    
    /* Block 5: Switch with multiple cases */
    checksum += switch_block(checksum);
    
    /* Block 6: Speculative loop */
    checksum += speculative_loop(8, checksum);
    
    return checksum;
}

/* Second test function with different patterns */
static int test_function_2(int base) {
    int result = base;
    
    /* Unrolled loop creating wide basic block */
    #pragma GCC unroll 8
    for (int i = 0; i < 32; i++) {
        result = complex_int_chain(result, i, i+1, i+2, i+3);
        result ^= (result << 13);
        result ^= (result >> 17);
        result ^= (result << 5);
    }
    
    /* More complex control flow */
    if (result & 1) {
        result = wide_basic_block(result, result>>1, result>>2, result>>3,
                                  result>>4, result>>5, result>>6, result>>7);
    } else {
        result = speculative_loop(6, result);
    }
    
    /* Final mixing */
    for (int i = 0; i < 4; i++) {
        result = result * 6364136223846793005ULL + 1442695040888963407ULL;
        result += side_effect_func(result);
    }
    
    return result;
}

/* Third test function focusing on floating point */
static float test_function_3(float base) {
    float result = base;
    
    /* Many floating point operations */
    for (int i = 0; i < 20; i++) {
        result = sinf(result) * 2.0f;
        result = cosf(result) + 1.0f;
        result = expf(result * 0.1f);
        result = logf(fabsf(result) + 1.0f);
        result += (float)side_effect_func((int)result);
    }
    
    /* Mixed precision */
    double dresult = (double)result;
    for (int i = 0; i < 10; i++) {
        dresult = sqrt(dresult * dresult + 1.0);
        dresult = pow(dresult, 1.01);
        dresult = dresult * 0.99;
    }
    
    return (float)dresult;
}

/* Main driver */
int main() {
    int total_checksum = 0;
    
    printf("Starting scheduling test...\n");
    
    /* Seed RNG for variability */
    srand(time(NULL));
    
    /* Run multiple test functions to exercise different scheduling contexts */
    for (int i = 0; i < 100; i++) {
        int iter = rand() % 100 + 1;
        
        /* Call test functions with different parameters */
        total_checksum ^= test_function_1(iter);
        total_checksum += test_function_2(total_checksum);
        
        float fval = test_function_3((float)iter * 0.1f);
        total_checksum += (int)(fval * 1000.0f);
        
        /* Prevent optimization */
        asm volatile("" : "+r" (total_checksum));
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("Global counter: %d\n", global_counter);
    
    return total_checksum != 0 ? 0 : 1;
}
