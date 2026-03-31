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
__attribute__((target("avx2")))
__attribute__((noinline))
static v4si vector_operations(v4si a, v4si b, v4sf c, v4sf d) {
    /* Use volatile to prevent optimization of intermediate results */
    volatile v4si result1, result2;
    volatile v4sf fresult;
    volatile v2di lresult;
    
    /* Vector addition - may trigger built-in vectorized function creation */
    result1 = a + b;
    
    /* Vector multiplication */
    result2 = a * b;
    
    /* Mixed operations with conversion */
    v4sf temp = c + d;
    
    /* Use __builtin_convertvector for type conversion */
    v4si converted = __builtin_convertvector(temp, v4si);
    
    /* Shuffle operation - strong candidate for artificial function creation */
    v4si shuffled = __builtin_shuffle(result1, result2, 
                                     (v4si){0, 4, 1, 5});
    
    /* Complex expression mixing multiple operations */
    v4si final = (result1 + result2) * converted + shuffled;
    
    return final;
}

/* Another function using different vector types */
__attribute__((target("avx2")))
__attribute__((noinline))
static v2df double_vector_ops(v2df a, v2df b) {
    volatile v2df result;
    
    /* Operations that might use built-in vector functions */
    result = a * b + a / b;
    
    /* Shuffle for double vectors */
    v2df shuffled = __builtin_shuffle(a, b, (v2di){1, 0});
    
    return result + shuffled;
}

/* Function with OpenMP SIMD pragma */
__attribute__((noinline))
static void omp_simd_loop(int* restrict a, int* restrict b, int* restrict c, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Vectorizable operation that may create internal functions */
        c[i] = a[i] * b[i] + a[i] / (b[i] + 1);
    }
}

int main() {
    /* Initialize vector variables with patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.5f, 2.5f, 3.5f, 4.5f};
    v4sf vec_d = {0.5f, 1.5f, 2.5f, 3.5f};
    
    v2df dvec_a = {1.0, 2.0};
    v2df dvec_b = {3.0, 4.0};
    
    /* Call vector operations multiple times */
    v4si results[4];
    for (int i = 0; i < 4; i++) {
        /* Modify inputs slightly each iteration */
        vec_a[0] += i;
        vec_b[1] += i;
        
        results[i] = vector_operations(vec_a, vec_b, vec_c, vec_d);
        
        /* Also call double vector operations */
        dvec_a[0] += 0.1 * i;
        v2df dres = double_vector_ops(dvec_a, dvec_b);
        
        /* Use volatile to ensure operations aren't optimized away */
        volatile double dummy = dres[0] + dres[1];
        (void)dummy;  /* Suppress unused variable warning */
    }
    
    /* Test OpenMP SIMD loop */
    int array_size = 1024;
    int array_a[array_size];
    int array_b[array_size];
    int array_c[array_size];
    
    /* Initialize arrays */
    for (int i = 0; i < array_size; i++) {
        array_a[i] = i;
        array_b[i] = array_size - i;
    }
    
    /* Call SIMD loop function */
    omp_simd_loop(array_a, array_b, array_c, array_size);
    
    /* Aggregate results to ensure all code is live */
    long long total_sum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            total_sum += results[i][j];
        }
    }
    
    for (int i = 0; i < array_size; i++) {
        total_sum += array_c[i];
    }
    
    printf("Total sum: %lld\n", total_sum);
    
    return 0;
}
