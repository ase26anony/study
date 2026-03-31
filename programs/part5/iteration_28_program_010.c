/* Compile with: g++ -O3 -fopenmp -fsanitize=address -march=native -ftree-vectorize targhooks_test.cc -o targhooks_test */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Vector type declaration - may create artificial type nodes */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with hidden visibility using built-ins */
static inline int __attribute__((visibility("hidden"), always_inline))
hidden_builtin_func(int x, int y) {
    /* Complex expression with multiple built-ins */
    int a = __builtin_abs(x);
    int b = __builtin_expect(y > 0, 1);
    int c = __builtin_clz(a | 1);
    return __builtin_add_overflow_p(a, b, c) ? c : a + b;
}

/* Weak function with internal visibility using built-ins */
int __attribute__((weak, visibility("internal")))
weak_internal_func(int* ptr) {
    if (!ptr) __builtin_unreachable();
    
    /* Use alignment built-in */
    int* aligned_ptr = (int*)__builtin_assume_aligned(ptr, 16);
    
    /* Unpredictable branch */
    if (__builtin_unpredictable(*aligned_ptr > 1000)) {
        return __builtin_popcount(*aligned_ptr);
    }
    return *aligned_ptr;
}

/* Hot function with default visibility using vector operations */
int __attribute__((hot, visibility("default")))
hot_vector_func(v4si* vec1, v4si* vec2, int n) {
    v4si sum = {0, 0, 0, 0};
    
    for (int i = 0; i < n; i++) {
        /* Vector operations that may create artificial nodes */
        v4si v1 = vec1[i];
        v4si v2 = vec2[i];
        v4si prod = v1 * v2;
        
        /* Vector shuffle */
        v4si shuffled = __builtin_shuffle(prod, prod, (v4si){3, 2, 1, 0});
        sum += shuffled;
        
        /* Built-in for optimization hint */
        if (__builtin_expect(i > n/2, 0)) {
            __builtin_prefetch(&vec1[i+4], 0, 3);
        }
    }
    
    /* Horizontal add */
    int result = sum[0] + sum[1] + sum[2] + sum[3];
    return __builtin_sadd_overflow(result, 1, &result) ? 0 : result;
}

/* Constructor function - may generate initialization code */
void __attribute__((constructor(101)))
init_func() {
    printf("Initializing...\n");
}

/* Destructor function */
void __attribute__((destructor(101)))
cleanup_func() {
    printf("Cleaning up...\n");
}

/* Function with sanitizer-friendly code */
void __attribute__((noinline))
sanitizer_test(int* arr, int size) {
    /* Array access that sanitizer will instrument */
    for (int i = 0; i < size; i++) {
        arr[i] = i * 2;
    }
    
    /* Potential out-of-bounds access for ASan */
    if (size > 10) {
        int val = arr[size - 1];  /* Safe */
        arr[0] = val;
    }
}

/* Main function with OpenMP and complex control flow */
int main() {
    const int N = 1024;
    int* data = (int*)aligned_alloc(16, N * sizeof(int));
    
    /* Initialize with sanitizer test */
    sanitizer_test(data, N);
    
    /* Use hidden visibility function */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        data[i] = hidden_builtin_func(data[i], i);
        sum += data[i];
    }
    
    /* Use weak internal function */
    int weak_result = weak_internal_func(data);
    
    /* Prepare vector data */
    v4si vec1[N/4];
    v4si vec2[N/4];
    
    for (int i = 0; i < N/4; i++) {
        for (int j = 0; j < 4; j++) {
            vec1[i][j] = i * 4 + j;
            vec2[i][j] = (i * 4 + j) % 8;
        }
    }
    
    /* Use hot vector function */
    int vec_result = hot_vector_func(vec1, vec2, N/4);
    
    /* OpenMP parallel region - may generate artificial helper functions */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        data[i] = __builtin_parity(data[i]) ? data[i] * 2 : data[i] / 2;
        sum += data[i];
        
        /* Trap in unreachable path */
        if (__builtin_expect(data[i] < 0, 0)) {
            __builtin_trap();
        }
    }
    
    /* Complex expression with multiple built-ins */
    int final_result = __builtin_add_overflow_p(sum, vec_result, weak_result) 
                      ? __builtin_abs(sum) 
                      : __builtin_clz(sum | 1);
    
    /* Convert vector for potential artificial nodes */
    v4si int_vec = {1, 2, 3, 4};
    v4sf float_vec = __builtin_convertvector(int_vec, v4sf);
    
    printf("Result: %d\n", final_result);
    
    free(data);
    return 0;
}
