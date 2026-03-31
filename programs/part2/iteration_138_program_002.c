/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -mavx2 -fopenmp-simd -fdump-tree-optimized -o vector_test vector_test.c */

#include <stdio.h>
#include <stdint.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function with AVX2 target attribute to ensure vector builtins are considered */
__attribute__((target("avx2"), noinline))
static void vector_operations(volatile v4si *out1, volatile v4sf *out2, 
                              volatile v2di *out3, v4si a, v4si b, v4sf c, v4sf d) {
    /* Vector addition - may trigger builtin vectorized function creation */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Mixed operations */
    v4si mixed = add_result + mul_result;
    
    /* Type conversion using builtin */
    v4sf float_vec = __builtin_convertvector(mixed, v4sf);
    
    /* Shuffle operation using builtin */
    v4si shuffled = __builtin_shuffle(mixed, (v4si){3, 2, 1, 0});
    
    /* Another conversion */
    v2di long_vec = __builtin_convertvector(shuffled, v2di);
    
    /* Store to volatile to prevent optimization */
    *out1 = shuffled;
    *out2 = float_vec + c * d;  /* Mixed float operations */
    *out3 = long_vec;
}

/* Another function with different vector operations */
__attribute__((target("avx2"), noinline))
static v4si complex_shuffle_operations(v4si a, v4si b, v8hi c) {
    /* Complex expression with multiple shuffles */
    v4si temp = a + b;
    
    /* Multiple shuffle operations that may create internal functions */
    v4si shuffle1 = __builtin_shuffle(temp, (v4si){1, 0, 3, 2});
    v4si shuffle2 = __builtin_shuffle(temp, (v4si){3, 2, 1, 0});
    
    /* Convert to short vector and back */
    v8hi short_vec = __builtin_convertvector(temp, v8hi);
    v8hi short_result = short_vec + c;
    
    /* Convert back */
    v4si final = __builtin_convertvector(short_result, v4si);
    
    return final + shuffle1 * shuffle2;
}

int main() {
    volatile v4si result1;
    volatile v4sf result2;
    volatile v2di result3;
    
    /* Initialize vectors with patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.5f, 2.5f, 3.5f, 4.5f};
    v4sf vec_d = {0.5f, 1.0f, 1.5f, 2.0f};
    v8hi vec_e = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Call vector operations multiple times */
    for (int i = 0; i < 10; i++) {
        vector_operations(&result1, &result2, &result3, 
                         vec_a + i, vec_b - i, vec_c, vec_d);
        
        /* Use OpenMP SIMD pragma for vectorization */
        #pragma omp simd
        for (int j = 0; j < 4; j++) {
            vec_a[j] += result1[j];
        }
    }
    
    /* Complex shuffle operations */
    v4si complex_result = complex_shuffle_operations(vec_a, vec_b, vec_e);
    
    /* OpenMP SIMD loop with reduction */
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < 4; i++) {
        sum += complex_result[i];
    }
    
    /* Additional vector operations using builtins directly */
    v2df double_vec1 = {1.0, 2.0};
    v2df double_vec2 = {3.0, 4.0};
    
    /* This may trigger internal builtin function creation */
    v2df double_result = __builtin_ia32_addpd(double_vec1, double_vec2);
    
    /* Mix different vector types */
    v4si int_result = __builtin_convertvector(double_result, v4si);
    
    /* Final computation and output */
    sum += int_result[0] + int_result[1];
    printf("Result: %d\n", sum);
    
    return 0;
}
