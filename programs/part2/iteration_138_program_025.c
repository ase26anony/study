/* Compile with: gcc -O2 -ftree-vectorize -fno-inline -mavx2 -fopenmp-simd -fdump-tree-optimized vector_test.c -o vector_test */

#include <stdio.h>
#include <stdint.h>

/* Define various vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function with AVX2 target attribute to ensure vector builtins are considered */
__attribute__((target("avx2")))
__attribute__((noinline))
static v4si process_vectors(v4si a, v4si b, v4sf c, v2di d) {
    volatile v4si result1, result2;
    volatile v4sf float_result;
    volatile v2di double_int_result;
    
    /* Vector addition - may trigger builtin vectorized function creation */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Use __builtin_shuffle to create internal function declaration */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    
    /* Type conversion using builtin - strong candidate for artificial decl */
    v4sf converted = __builtin_convertvector(a, v4sf);
    
    /* Mix operations between different vector types */
    v4sf mixed_ops = converted + c;
    
    /* Store to volatile to prevent optimization */
    result1 = add_result + mul_result + shuffled;
    float_result = mixed_ops;
    
    /* Convert back to int vector */
    v4si final_int = __builtin_convertvector(mixed_ops, v4si);
    
    /* Use architecture-specific builtin if available */
#ifdef __x86_64__
    /* This builtin often creates artificial declarations */
    v2di packed = __builtin_ia32_paddd128((v2di)a, (v2di)b);
    double_int_result = packed + d;
#endif
    
    return final_int + result1;
}

/* Another function with complex vector operations */
__attribute__((noinline))
static v8hi process_small_vectors(v8hi a, v8hi b) {
    /* Multiple operations to encourage internal function creation */
    v8hi add = a + b;
    v8hi sub = a - b;
    v8hi mul = a * b;
    
    /* Complex shuffle pattern */
    v8hi shuffled = __builtin_shuffle(add, sub, (v8hi){0, 2, 4, 6, 1, 3, 5, 7});
    
    return shuffled + mul;
}

int main() {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.5f, 2.5f, 3.5f, 4.5f};
    v2di vec_d = {10LL, 20LL};
    
    v8hi small_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi small_b = {8, 7, 6, 5, 4, 3, 2, 1};
    
    volatile v4si final_results[4];
    volatile v8hi small_results[2];
    
    /* Call vector processing multiple times */
    for (int i = 0; i < 4; i++) {
        /* Modify inputs slightly each iteration */
        vec_a[0] += i;
        vec_b[1] += i;
        
        final_results[i] = process_vectors(vec_a, vec_b, vec_c, vec_d);
    }
    
    /* Process small vectors */
    small_results[0] = process_small_vectors(small_a, small_b);
    small_results[1] = process_small_vectors(small_b, small_a);
    
    /* OpenMP SIMD loop - may create vectorized versions as internal functions */
    int array[1024];
    int sum = 0;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < 1024; i++) {
        /* Mix scalar and implicit vector operations */
        array[i] = i * 2 + (i % 4);
        sum += array[i];
        
        /* Use vector operations inside loop */
        if (i % 128 == 0) {
            v4si temp = {i, i+1, i+2, i+3};
            v4si temp2 = temp * (v4si){2, 2, 2, 2};
            /* Force use of result */
            array[i] += temp2[0];
        }
    }
    
    /* Additional complex expression with multiple vector types */
    v2df double_vec1 = {1.0, 2.0};
    v2df double_vec2 = {3.0, 4.0};
    v2df double_result = double_vec1 * double_vec2;
    
    /* Convert between different vector types */
    v4si int_from_double = __builtin_convertvector(double_result, v4si);
    
    /* Aggregate results to prevent dead code elimination */
    int total = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            total += final_results[i][j];
        }
    }
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            total += small_results[i][j];
        }
    }
    
    total += sum;
    total += int_from_double[0] + int_from_double[1];
    
    printf("Result: %d\n", total);
    
    return 0;
}
