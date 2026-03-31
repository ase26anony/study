/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -mavx2 -fopenmp-simd -fdump-tree-optimized vector_test.c -o vector_test */

#include <stdio.h>
#include <stdint.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function with AVX2 target attribute to ensure vector built-ins are considered */
__attribute__((target("avx2"), noinline))
static void vector_operations(volatile v4si *out1, volatile v4sf *out2, 
                              volatile v2di *out3, int seed) {
    /* Initialize vectors with patterns */
    v4si a = {seed + 1, seed + 2, seed + 3, seed + 4};
    v4si b = {seed + 5, seed + 6, seed + 7, seed + 8};
    v4sf c = {seed * 1.0f, seed * 2.0f, seed * 3.0f, seed * 4.0f};
    v4sf d = {seed * 5.0f, seed * 6.0f, seed * 7.0f, seed * 8.0f};
    
    /* Vector addition - may trigger builtin vectorized function creation */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Use __builtin_shuffle to create complex patterns */
    v4si shuffle_mask = {3, 1, 2, 0};
    v4si shuffled = __builtin_shuffle(a, b, shuffle_mask);
    
    /* Type conversion using __builtin_convertvector */
    v4sf float_vec = __builtin_convertvector(a, v4sf);
    
    /* Mixed operations with different vector types */
    v2di di_vec = {seed * 10LL, seed * 20LL};
    v2di di_vec2 = {seed * 30LL, seed * 40LL};
    v2di di_result = di_vec + di_vec2;
    
    /* Architecture-specific built-in (x86 SSE/AVX) */
    v4si builtin_add = __builtin_ia32_paddd128(a, b);
    
    /* Complex expression mixing operations */
    v4si complex_expr = (a + b) * shuffled - builtin_add;
    
    /* Store to volatile to prevent optimization */
    *out1 = complex_expr;
    *out2 = float_vec + d;
    *out3 = di_result;
}

/* Function with OpenMP SIMD pragma */
__attribute__((noinline))
static int omp_simd_loop(int *array, int n) {
    int sum = 0;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Vectorizable operation with mixed patterns */
        array[i] = (array[i] * 3 + 7) & 0xFF;
        sum += array[i];
    }
    
    return sum;
}

/* Another function using vector extensions directly */
__attribute__((target("avx2"), noinline))
static v4sf vector_conversion_test(v4si int_vec, v4sf float_vec) {
    /* Multiple conversions and operations */
    v4sf converted = __builtin_convertvector(int_vec, v4sf);
    v4sf result = converted * float_vec;
    
    /* Shuffle between different vector types */
    v4si shuffle_temp = __builtin_shuffle(int_vec, int_vec, (v4si){2, 3, 0, 1});
    v4sf converted2 = __builtin_convertvector(shuffle_temp, v4sf);
    
    return result + converted2;
}

int main() {
    volatile v4si result1;
    volatile v4sf result2;
    volatile v2di result3;
    
    /* Call vector operations multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        vector_operations(&result1, &result2, &result3, i * 100);
    }
    
    /* Test OpenMP SIMD vectorization */
    int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i;
    }
    
    int sum = omp_simd_loop(array, 256);
    
    /* Test vector conversions */
    v4si int_vec = {1, 2, 3, 4};
    v4sf float_vec = {1.5f, 2.5f, 3.5f, 4.5f};
    v4sf conv_result = vector_conversion_test(int_vec, float_vec);
    
    /* Use volatile to ensure all results are used */
    volatile v4sf final_check = conv_result;
    
    /* Aggregate and print results to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += result1[i];
    }
    
    printf("Result: %d, OMP sum: %d\n", total, sum);
    
    return 0;
}
