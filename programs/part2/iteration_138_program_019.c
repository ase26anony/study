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
                              volatile v2di *out3, int init_val) {
    /* Initialize vectors with patterns */
    v4si a = {init_val, init_val + 1, init_val + 2, init_val + 3};
    v4si b = {init_val + 4, init_val + 5, init_val + 6, init_val + 7};
    v4sf c = {init_val * 1.0f, init_val * 2.0f, init_val * 3.0f, init_val * 4.0f};
    v4sf d = {init_val * 5.0f, init_val * 6.0f, init_val * 7.0f, init_val * 8.0f};
    
    /* Vector addition - may trigger built-in vectorized function creation */
    v4si add_result = a + b;
    
    /* Vector multiplication */
    v4si mul_result = a * b;
    
    /* Mixed operations with conversion */
    v4sf float_result = c + d;
    
    /* Use __builtin_convertvector for type conversion */
    v4si converted = __builtin_convertvector(float_result, v4si);
    
    /* Use __builtin_shuffle with a mask */
    v4si shuffled = __builtin_shuffle(converted, (v4si){3, 2, 1, 0});
    
    /* Combine results */
    v4si final_int = add_result + mul_result + shuffled;
    
    /* Convert int vector to float vector */
    v4sf final_float = __builtin_convertvector(final_int, v4sf);
    
    /* Convert to double vector */
    v2df double_vec = __builtin_convertvector((v2di){final_int[0], final_int[1]}, v2df);
    
    /* Store to volatile to prevent optimization */
    *out1 = final_int;
    *out2 = final_float;
    *out3 = (v2di){final_int[0], final_int[1]};
    
    /* Use architecture-specific built-in if available */
#ifdef __SSE2__
    /* This may create internal artificial declarations */
    v4si sse_result = __builtin_ia32_paddd128(add_result, mul_result);
    (void)sse_result; /* Prevent unused variable warning */
#endif
}

/* Another function with different vector types */
__attribute__((noinline))
static void process_short_vectors(volatile v8hi *out) {
    v8hi v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi v2 = {8, 7, 6, 5, 4, 3, 2, 1};
    
    /* Multiple operations to encourage internal function creation */
    v8hi result = v1 + v2;
    result = result * v1;
    result = result - v2;
    
    /* Shuffle with complex pattern */
    v8hi shuffled = __builtin_shuffle(result, (v8hi){7, 6, 5, 4, 3, 2, 1, 0});
    
    *out = shuffled;
}

int main() {
    volatile v4si int_result;
    volatile v4sf float_result;
    volatile v2di double_int_result;
    volatile v8hi short_result;
    
    int sum = 0;
    
    /* Call vector operations multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        vector_operations(&int_result, &float_result, &double_int_result, i * 10);
        
        /* Use OpenMP SIMD pragma on a vectorizable loop */
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 4; j++) {
            sum += int_result[j];
        }
    }
    
    /* Process short vectors */
    process_short_vectors(&short_result);
    
    /* Add short vector elements to sum */
    for (int i = 0; i < 8; i++) {
        sum += short_result[i];
    }
    
    /* Complex expression mixing different vector types */
    v4si va = {1, 2, 3, 4};
    v4si vb = {5, 6, 7, 8};
    v2di vc = {va[0] + vb[0], va[1] + vb[1]};
    
    /* Convert between vector types */
    v4sf vf = __builtin_convertvector(va, v4sf);
    v4si vi = __builtin_convertvector(vf, v4si);
    
    /* Use the results */
    sum += vi[0] + vi[1] + vi[2] + vi[3];
    sum += vc[0] + vc[1];
    
    printf("Result: %d\n", sum);
    
    return 0;
}
