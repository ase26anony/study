Here's a C++ program designed to trigger the uncovered lines in `targhooks.cc` by creating artificial declarations with specific visibility and linkage properties:

```cpp
// Compile with: g++ -O3 -fopenmp -fsanitize=address -march=native -ftree-vectorize -fno-omit-frame-pointer

#include <cstdlib>
#include <cmath>

// Vector type for target-specific built-ins
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

// Function with hidden visibility using built-ins
__attribute__((visibility("hidden"), noinline))
static int hidden_builtin_func(int x, int y) {
    // Complex expression with multiple built-ins
    int a = __builtin_abs(x);
    int b = __builtin_expect(y > 0, 1);
    int c = __builtin_clz(a | 1);
    
    // Unpredictable control flow
    if (__builtin_unpredictable(a > b)) {
        return __builtin_popcount(a ^ b) + c;
    } else {
        return __builtin_ctz(a + b) * c;
    }
}

// Weak function with internal visibility
__attribute__((weak, visibility("internal")))
void weak_internal_func(int* ptr) {
    if (ptr) {
        // Assume aligned pointer
        int* aligned_ptr = (int*)__builtin_assume_aligned(ptr, 16);
        *aligned_ptr = __builtin_abs(*aligned_ptr);
    }
}

// Hot function with default visibility using vector operations
__attribute__((hot, visibility("default")))
v4si vector_operations(v4si a, v4si b) {
    // Vector arithmetic
    v4si result = a + b * 2;
    
    // Vector shuffle
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    
    // Conditional with built-in unreachable
    if (__builtin_expect(__builtin_add_overflow_p(a[0], b[0], (int)0), 0)) {
        __builtin_unreachable();
    }
    
    return result;
}

// Constructor function
__attribute__((constructor, visibility("hidden")))
void init_func() {
    // Use built-in in constructor
    volatile int x = __builtin_bswap32(0x12345678);
    (void)x;
}

// Destructor function  
__attribute__((destructor, visibility("hidden")))
void cleanup_func() {
    // Use trap in destructor
    if (__builtin_expect(0, 0)) {
        __builtin_trap();
    }
}

// Function with sanitizer-friendly code
__attribute__((noinline))
void sanitizer_func(int* arr, int size) {
    // Array access that sanitizer will check
    for (int i = 0; i < size; i++) {
        // Use built-in for optimization hint
        if (__builtin_expect(i < size - 1, 1)) {
            arr[i] = hidden_builtin_func(arr[i], i);
        } else {
            // Last element gets special treatment
            arr[i] = __builtin_abs(arr[i]) * __builtin_clz(arr[i] | 1);
        }
    }
    
    // Call weak function
    weak_internal_func(arr);
}

// Main function with OpenMP and complex control flow
int main() {
    const int N = 1024;
    int* data = new int[N];
    
    // Initialize data
    for (int i = 0; i < N; i++) {
        data[i] = i - N/2;
    }
    
    // Call sanitizer function
    sanitizer_func(data, N);
    
    // OpenMP parallel region - triggers compiler-generated helper functions
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        // Complex expression with built-ins
        int x = data[i];
        int y = __builtin_abs(x);
        int z = __builtin_popcount(y);
        
        // Unpredictable branch
        if (__builtin_unpredictable(z > 16)) {
            data[i] = __builtin_ctz(z) + __builtin_clz(y);
        } else {
            data[i] = __builtin_ffs(z) * y;
        }
        
        // Assume aligned access
        int* ptr = &data[i];
        ptr = (int*)__builtin_assume_aligned(ptr, sizeof(int));
        
        // Overflow check
        if (__builtin_add_overflow_p(*ptr, i, (int)0)) {
            *ptr = __builtin_sadd_overflow(*ptr, i, ptr) ? 0 : *ptr;
        }
    }
    
    // Vector operations
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = vector_operations(vec_a, vec_b);
    
    // Use vector result
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec_result[i];
    }
    
    // More built-in usage
    int final_result = __builtin_bswap32(sum);
    final_result = __builtin_abs(final_result);
    
    // Convert vector
    v4sf float_vec = __builtin_convertvector(vec_result, v4sf);
    
    // Clean up
    delete[] data;
    
    return final_result > 0 ? 0 : 1;
}
```

This program combines multiple techniques to trigger the target code:

1. **Artificial/Built-in Declarations**: Uses numerous GCC built-ins (`__builtin_abs`, `__builtin_expect`, `__builtin_unpredictable`, etc.) in complex expressions that force the compiler to create internal artificial tree nodes.

2. **Visibility Attributes**: Functions with `visibility("hidden")`, `visibility("internal")`, and `visibility("default")` attributes, combined with `constructor`/`destructor` attributes that often require compiler-generated initialization code.

3. **OpenMP Pragmas**: The `#pragma omp parallel for` triggers the compiler to generate helper functions and data structures, which are typically artificial declarations.

4. **Vector Extensions**: Uses GCC vector types and vector built-ins (`__builtin_shuffle`, `__builtin_convertvector`) that may require artificial declarations for vector operations.

5. **Sanitizer Interaction**: The array access pattern in `sanitizer_func` combined with `-fsanitize=address` will cause the compiler to insert runtime checks via artificial functions.

6. **Complex Control Flow**: Multiple branches with `__builtin_unpredictable`, `__builtin_expect`, and `__builtin_unreachable()` that force the compiler to synthesize control flow nodes.

7. **Weak Linkage**: The `weak_internal_func` uses `__attribute__((weak))` which may trigger special handling in the compiler's symbol resolution.

The combination of these techniques across different functions and in `main()` ensures that various compiler passes (frontend, middle-end optimizations, backend code generation) are engaged, increasing the likelihood of hitting the target lines in `targhooks.cc` that set flags on artificial tree nodes.
