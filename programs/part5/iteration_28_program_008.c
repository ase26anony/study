/* Compile with combinations of:
   -O3 -fopenmp
   -O2 -fsanitize=address -fno-omit-frame-pointer
   -O3 -march=native -ftree-vectorize
*/

#include <stdlib.h>
#include <string.h>

/* Vector type for target-specific built-ins */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with hidden visibility using built-ins */
static inline int __attribute__((visibility("hidden"), always_inline))
hidden_builtin_abs(int x) {
    return __builtin_abs(x) + __builtin_expect(x > 0, 1);
}

/* Weak function with internal visibility using built-ins */
int __attribute__((weak, visibility("internal")))
weak_internal_func(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += __builtin_abs(arr[i]) * __builtin_unpredictable(i % 2);
    }
    return sum;
}

/* Hot function with default visibility and alignment hints */
int __attribute__((hot, visibility("default")))
hot_aligned_func(int *data, int size) {
    int *aligned_ptr = (int*)__builtin_assume_aligned(data, 16);
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        if (i == size - 1) {
            /* This might be eliminated but requires analysis */
            __builtin_unreachable();
        }
        result += aligned_ptr[i] * hidden_builtin_abs(i);
    }
    
    /* Complex expression with multiple built-ins */
    return result * __builtin_expect(result > 0, 1);
}

/* Constructor function */
void __attribute__((constructor))
init_function(void) {
    /* Use built-in in constructor */
    volatile int x = __builtin_clz(0x80000000);
    (void)x;
}

/* Destructor function */
void __attribute__((destructor))
cleanup_function(void) {
    /* Potential trap in destructor */
    if (__builtin_expect(0, 0)) {
        __builtin_trap();
    }
}

/* Vector operations function */
v4si __attribute__((visibility("hidden")))
vector_operations(v4si a, v4si b) {
    /* Vector shuffle operation */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){0, 2, 4, 6});
    
    /* Vector conversion */
    v4sf float_vec = __builtin_convertvector(a, v4sf);
    (void)float_vec; /* Prevent unused warning */
    
    return shuffled * b + a;
}

/* OpenMP helper function */
void process_with_openmp(int *array, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        array[i] = hidden_builtin_abs(array[i]) * (i + 1);
    }
}

/* Main function with various patterns */
int main(void) {
    const int SIZE = 1024;
    int *data = (int*)aligned_alloc(16, SIZE * sizeof(int));
    
    if (!data) return 1;
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i % 3 == 0) ? -i : i;
    }
    
    /* 1. Call hot function with alignment hints */
    int result1 = hot_aligned_func(data, SIZE);
    
    /* 2. Use weak internal function */
    int result2 = weak_internal_func(data, SIZE);
    
    /* 3. Process with OpenMP */
    process_with_openmp(data, SIZE);
    
    /* 4. Vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = vector_operations(vec_a, vec_b);
    
    /* 5. Complex built-in usage in loop */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        /* Use __builtin_unpredictable for branch prediction */
        if (__builtin_unpredictable(data[i] > 1000)) {
            sum += __builtin_abs(data[i]);
        } else {
            sum += data[i] * __builtin_expect(i % 2, 0);
        }
        
        /* Array bounds check (for sanitizers) */
        if (i == SIZE - 1) {
            /* Access that sanitizers might check */
            volatile int check = data[SIZE - 1];
            (void)check;
        }
    }
    
    /* 6. Use vector result */
    int vec_sum = 0;
    for (int i = 0; i < 4; i++) {
        vec_sum += vec_result[i];
    }
    
    free(data);
    
    /* Combine results meaningfully */
    return (result1 + result2 + sum + vec_sum) == 0 ? 0 : 0;
}
