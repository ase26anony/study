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
static v4si process_vectors(v4si a, v4si b, v4sf c, v2di d) {
    volatile v4si result1, result2;
    volatile v4sf float_result;
    volatile v2di double_int_result;
    
    /* Vector addition - may trigger built-in vectorized function creation */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Mixed operations with conversion */
    v4sf float_vec = __builtin_convertvector(a, v4sf);
    v4sf float_op = float_vec + c;
    
    /* Shuffle operation - strong candidate for internal built-in */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){2, 3, 0, 1});
    
    /* Type conversion between different vector types */
    v2di converted = __builtin_convertvector(shuffled, v2di);
    v2di mixed = converted + d;
    
    /* Store to volatile to prevent optimization */
    result1 = add_result + mul_result;
    result2 = shuffled;
    float_result = float_op;
    double_int_result = mixed;
    
    /* Complex expression mixing multiple operations */
    return result1 + __builtin_convertvector(double_int_result, v4si);
}

/* Another function with different vector operations */
__attribute__((noinline))
static v8hi process_short_vectors(v8hi a, v8hi b) {
    /* Multiple vector operations that may require internal helpers */
    v8hi add = a + b;
    v8hi sub = a - b;
    v8hi mul = a * b;
    
    /* Complex shuffle pattern */
    v8hi shuffled = __builtin_shuffle(add, sub, (v8hi){0, 2, 4, 6, 1, 3, 5, 7});
    
    /* Mixed operation result */
    return shuffled + mul;
}

int main() {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.5f, 2.5f, 3.5f, 4.5f};
    v2di vec_d = {10LL, 20LL};
    v8hi vec_e = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi vec_f = {9, 10, 11, 12, 13, 14, 15, 16};
    
    int total_sum = 0;
    
    /* Call vector processing function multiple times */
    for (int i = 0; i < 100; i++) {
        v4si result = process_vectors(vec_a, vec_b, vec_c, vec_d);
        
        /* Extract and sum elements to ensure code isn't dead */
        for (int j = 0; j < 4; j++) {
            total_sum += result[j];
        }
        
        /* Modify inputs slightly each iteration */
        vec_a[0] += 1;
        vec_b[3] += 1;
    }
    
    /* Process short vectors */
    v8hi short_result = process_short_vectors(vec_e, vec_f);
    for (int i = 0; i < 8; i++) {
        total_sum += short_result[i];
    }
    
    /* OpenMP SIMD loop - may trigger creation of vectorized loop versions */
    int array[1024];
    int sum_array = 0;
    
    #pragma omp simd
    for (int i = 0; i < 1024; i++) {
        array[i] = i * 2;
    }
    
    #pragma omp simd reduction(+:sum_array)
    for (int i = 0; i < 1024; i++) {
        sum_array += array[i];
    }
    
    total_sum += sum_array;
    
    /* Use architecture-specific built-ins if available */
    #ifdef __SSE2__
    {
        v4si sse_result;
        /* This may trigger internal built-in function creation */
        __builtin_ia32_paddd128(vec_a, vec_b);
    }
    #endif
    
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}
